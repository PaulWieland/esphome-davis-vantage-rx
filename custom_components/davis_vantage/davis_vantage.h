#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <vector>

// Forward declaration of CC1101 is tricky because of set_frequency.
// We will use a template or define a small interface if we can't include it.
#include "esphome/components/cc1101/cc1101.h"

namespace esphome {
namespace davis_vantage {

class DavisVantage : public PollingComponent {
 public:
  void set_cc1101(cc1101::CC1101Component *cc) { cc1101_ = cc; }
  void set_unit_id(int unit_id) { known_unit_id_ = unit_id; }
  void set_region(std::string region) { region_ = region; }
  void set_metric(bool metric) { metric_ = metric; }
  
  void set_temperature_sensor(sensor::Sensor *s) { temp_sensor_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { hum_sensor_ = s; }
  void set_wind_speed_sensor(sensor::Sensor *s) { wind_speed_sensor_ = s; }
  void set_wind_gust_sensor(sensor::Sensor *s) { wind_gust_sensor_ = s; }
  void set_daily_rain_sensor(sensor::Sensor *s) { rain_sensor_ = s; }
  void set_rain_rate_sensor(sensor::Sensor *s) { rain_rate_sensor_ = s; }
  float get_rain_rate_in() {
    if (millis() - last_tip_time_ > 900000) return 0.0f;
    return current_rain_rate_in_;
  }
  void set_battery_sensor(binary_sensor::BinarySensor *s) { battery_sensor_ = s; }
  void set_wind_dir_sensor(text_sensor::TextSensor *s) { wind_dir_sensor_ = s; }

  // Set polling interval to 30ms to replace the interval block
  DavisVantage() : PollingComponent(30) {}

  void setup() override;
  void update() override;
  void process_packet(std::vector<uint8_t> &x);

  float get_wind_dir_degrees() const { return raw_wind_dir_; }

 protected:
  cc1101::CC1101Component *cc1101_{nullptr};
  sensor::Sensor *temp_sensor_{nullptr};
  sensor::Sensor *hum_sensor_{nullptr};
  sensor::Sensor *wind_speed_sensor_{nullptr};
  sensor::Sensor *wind_gust_sensor_{nullptr};
  sensor::Sensor *rain_sensor_{nullptr};
  sensor::Sensor *rain_rate_sensor_{nullptr};
  uint32_t last_tip_time_{0};
  float current_rain_rate_in_{0.0f};
  binary_sensor::BinarySensor *battery_sensor_{nullptr};
  text_sensor::TextSensor *wind_dir_sensor_{nullptr};

  std::string region_{"NA"};
  bool metric_{false};
  int known_unit_id_{0};
  int current_freq_index_{-1};
  int hop_index_{0};
  uint32_t channel_dwell_start_{0};
  uint32_t last_packet_time_{4294900000};
  uint32_t expected_packet_time_{0};
  uint32_t prev_last_packet_time_{0};
  
  int rain_count_prev_{-1};
  float rain_total_in_{0.0f};
  float raw_wind_dir_{0.0f};

  // NA frequencies
  float na_hop_freqs[51] = {
    911414000.0, 902382000.0, 911915000.0, 922953000.0, 914927000.0, 906396000.0, 925965000.0, 918438000.0, 
    908905000.0, 920445000.0, 913420000.0, 903888000.0, 916934000.0, 924459000.0, 910410000.0, 904891000.0, 
    915929000.0, 921448000.0, 907399000.0, 926968000.0, 912919000.0, 903385000.0, 917434000.0, 923456000.0, 
    909407000.0, 926466000.0, 905895000.0, 914424000.0, 919441000.0, 924960000.0, 902885000.0, 910912000.0, 
    921950000.0, 915428000.0, 906898000.0, 917936000.0, 927470000.0, 920947000.0, 908403000.0, 912417000.0, 
    918940000.0, 904389000.0, 923957000.0, 916432000.0, 909909000.0, 919944000.0, 905392000.0, 922452000.0, 
    907901000.0, 913923000.0, 925462000.0
  };
};

}  // namespace davis_vantage
}  // namespace esphome
