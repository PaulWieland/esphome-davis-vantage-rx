#include "davis_vantage.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cmath>
#include <cstdio>

namespace esphome {
namespace davis_vantage {

static const char *const TAG = "davis_vantage";

void DavisVantage::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Davis Vantage Receiver...");
  channel_dwell_start_ = millis();
}

void DavisVantage::update() {
  if (!cc1101_) return;

  uint32_t now = millis();

  // Are we in SYNC mode?
  if (now - last_packet_time_ < 15000) {

    // If a NEW packet was just received, sync our expected timer
    if (last_packet_time_ != prev_last_packet_time_) {
      prev_last_packet_time_ = last_packet_time_;
      expected_packet_time_ = last_packet_time_;
    }

    // Coasting: Davis station hops every 2562ms
    if (now - expected_packet_time_ > (2562 + 50)) {
      hop_index_ = (hop_index_ + 1) % 51;
      expected_packet_time_ += 2562;
      ESP_LOGW(TAG, "COASTING: Missed packet. FFWD to channel %d", hop_index_);
    }

    // Tune
    if (current_freq_index_ != hop_index_) {
      cc1101_->set_frequency(us_hop_freqs[hop_index_]);
      current_freq_index_ = hop_index_;
      channel_dwell_start_ = now;
    }
    return;
  }

  // HUNT MODE: Converging Sweep (Backward Hunt)
  if (now - channel_dwell_start_ >= 3000) {
    hop_index_ = (hop_index_ - 1 + 51) % 51;
    channel_dwell_start_ = now;
    ESP_LOGD(TAG, "HUNTING: Converging backward to channel %d", hop_index_);
  }

  if (current_freq_index_ != hop_index_) {
    cc1101_->set_frequency(us_hop_freqs[hop_index_]);
    current_freq_index_ = hop_index_;
  }
}

void DavisVantage::process_packet(std::vector<uint8_t> &x) {
  if (x.size() < 8) return;

  // 1. Bit-reversal
  std::vector<uint8_t> d(8);
  for (int i = 0; i < 8; i++) {
    uint8_t b = x[i];
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    d[i] = b;
  }

  // 2. Robust CRC check
  bool crc_ok = false;
  for (int shift = 0; shift < 3; shift++) {
    uint16_t crc = 0;
    for (int i = 0; i < 6; i++) {
      crc ^= (uint16_t)d[i] << 8;
      for (int j = 0; j < 8; j++) {
        crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
      }
    }

    uint16_t received_crc_1 = ((uint16_t)d[6] << 8) | d[7];
    uint16_t received_crc_2 = ((uint16_t)d[7] << 8) | d[6];

    if (crc == received_crc_1 || crc == received_crc_2) {
      crc_ok = true;
      break;
    }

    uint8_t carry = 0;
    for (int i = 0; i < 8; i++) {
      uint8_t next_carry = (d[i] & 0x01) << 7;
      d[i] = (d[i] >> 1) | carry;
      carry = next_carry;
    }
  }

  if (!crc_ok) return;

  // 3. Transmitter ID lock
  uint8_t unit_id = d[0] & 0x07;
  bool battery_low = (d[0] & 0x08) != 0;

  if (known_unit_id_ < 0) {
    known_unit_id_ = unit_id;
    ESP_LOGW(TAG, "Station ID successfully locked onto: %d", unit_id);
  } else if (unit_id != known_unit_id_) {
    return; // Ignore neighbors
  }

  // Sync Timer Update
  last_packet_time_ = millis();
  hop_index_ = (hop_index_ + 1) % 51;
  ESP_LOGI(TAG, "SYNC: Next predicted channel is %d", hop_index_);
  ESP_LOGI(TAG, "Valid packet received! Type: %d, Wind: %.1f mph", (int)(d[0] >> 4), (float)d[1]);

  if (battery_sensor_) battery_sensor_->publish_state(battery_low);

  // Wind processing
  float wind_mph = d[1];
  float wind_dir = d[2] * (360.0f / 255.0f);

  if (wind_mph >= 0.0f && wind_mph < 160.0f) {
    if (wind_speed_sensor_) wind_speed_sensor_->publish_state(wind_mph);
    if (wind_gust_sensor_ && std::isnan(wind_gust_sensor_->state)) {
      wind_gust_sensor_->publish_state(wind_mph);
    }
  }
  
  if (wind_dir >= 0.0f && wind_dir <= 360.0f) {
    raw_wind_dir_ = wind_dir;
    if (wind_dir_sensor_) {
      int index = int((wind_dir + 11.25f) / 22.5f) % 16;
      const char* compassPoints[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
      };
      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%.0f° %s", wind_dir, compassPoints[index]);
      wind_dir_sensor_->publish_state(std::string(buffer));
    }
  }

  // Parse packet types
  uint8_t ptype = d[0] >> 4;
  if (ptype == 8) {
    uint16_t raw = ((uint16_t)d[3] << 8) | d[4];
    float temp_f = raw / 160.0f;
    if (temp_f > -40.0f && temp_f < 140.0f) {
      if (temp_sensor_) temp_sensor_->publish_state(temp_f);
      ESP_LOGI(TAG, "Weather Update - Temperature: %.1f F", temp_f);
    }
  } else if (ptype == 9) {
    float gust_mph = d[3];
    if (gust_mph >= 0.0f && gust_mph < 200.0f) {
      if (wind_gust_sensor_) wind_gust_sensor_->publish_state(gust_mph);
    }
  } else if (ptype == 10) {
    uint16_t raw = (((uint16_t)(d[4] >> 4) & 0x03) << 8) | d[3];
    float hum = raw / 10.0f;
    if (hum > 0.0f && hum <= 100.0f) {
      if (hum_sensor_) hum_sensor_->publish_state(hum);
      ESP_LOGI(TAG, "Weather Update - Humidity: %.0f%%", hum);
    }
  } else if (ptype == 14) {
    int tips = ((int)d[3] + (((int)d[4] >> 7) << 8)) & 0x7F;
    if (rain_count_prev_ >= 0) {
      int delta = tips - rain_count_prev_;
      if (delta < 0) delta += 128;
      if (delta > 0 && delta < 10) {
        rain_total_in_ += delta * 0.01f;
        if (rain_sensor_) rain_sensor_->publish_state(rain_total_in_);
      }
    }
    rain_count_prev_ = tips;
  }
}

}  // namespace davis_vantage
}  // namespace esphome
