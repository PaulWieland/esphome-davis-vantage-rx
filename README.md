# Davis Vantage Vue ESP32 Receiver

This project uses an ESP32 and a CC1101 wireless transceiver module to intercept RF signals from a Davis Vantage Vue weather station, decode them using ESPHome, and optionally broadcast them via MQTT and/or upload to Weather Underground.

<img width="580" height="320" alt="image" src="https://github.com/user-attachments/assets/7b323e9f-13d1-4aee-a689-d1126a162246" />

## Hardware Wiring

You need to connect the ESP32 (e.g. ESP32-WROOM Dev Kit) to the CC1101 module using the SPI pins.

**WARNING: The CC1101 operates at 3.3V. Do NOT connect it to the 5V pin on the ESP32, it will permanently damage the module.**

| CC1101 Pin | ESP32-WROOM Pin | Function |
| :--- | :--- | :--- |
| **VCC** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Ground |
| **MOSI** | **GPIO 23** | SPI MOSI |
| **SCLK** | **GPIO 18** | SPI Clock |
| **MISO** | **GPIO 19** | SPI MISO |
| **CSN** | **GPIO 5** | SPI Chip Select |
| **GDO0** | **GPIO 4** | Data / Interrupt Pin |
| **GDO2** | *Not Connected* | Leave disconnected |

*Note: Ensure you are using an antenna suitable for 915MHz (NA) for the best reception. The standard small coiled antennas included with generic CC1101 modules may have very limited range.*

## Setup Instructions

1. Install ESPHome (e.g., via the command line `pip install esphome`).
2. Update the `wifi` section in `davis_receiver.yaml` (or via your `secrets.yaml` file) with your local Wi-Fi credentials.
3. Flash the configuration to your ESP32 via USB for the first time:
   ```bash
   esphome run davis_receiver.yaml
   ```
4. The ESPHome log output will show raw RF packet hex data and decoded weather metrics.

## Frequency Hopping & Synchronization

The Davis Vantage Vue (NA 915MHz model) transmits data using a Frequency Hopping Spread Spectrum (FHSS) protocol across 51 different channels. A packet is broadcast roughly every 2.56 seconds. 

Since the CC1101 narrowband receiver can only listen to one frequency at a time, this ESP32 receiver uses a custom "chase and sync" algorithm:
1. **Hunting Mode**:  Listen on a channel for 3 seconds, then steps *backward* to the previous channel. Moving backward while the transmitter moves forward  guarantees a collision (a captured packet) in a maximum of ~70 seconds. <br /> <img width="537" height="454" alt="image" src="https://github.com/user-attachments/assets/3f3bca42-7e96-44c5-9a83-e3a560918ff3" />
2. **Sync Mode**: Once a packet is caught, the ESP32 calculates the next channel in the sequence, and waits for the next packet to arrive.
3. **Coasting**: If RF interference causes a missed packet, the receiver will coast forward to the next channel on a schedule. If after a long period no data is captured it will return to hunting mode in order to resync with the transmitter. <br /> <img width="628" height="286" alt="image" src="https://github.com/user-attachments/assets/d22f3eac-89d8-4f35-a7fa-3698b346bfd2" />


## Finding & Configuring Your Station ID

Davis weather stations can be configured to broadcast on one of 8 different "Station IDs" (often referred to by Davis as "Channels 1-8") to prevent interference between neighboring stations.
* **Factory Default**: Out of the box, all Davis stations are set to **Channel 1**. 
* **Finding Your ID**:
  * **Vantage Vue ISS**: Press the pushbutton on the outdoor transmitter and count the number of LED flashes.
  * **Vantage Pro2 ISS**: Check the physical DIP switch positions (1-3) on the outdoor transmitter board.
  * **From the Console**: Enter Setup Mode (Vue: press `2ND` then `SETUP` / VP2: press `DONE` and `-` together) and navigate to the Transmitter IDs screen. Look for the channel that is turned `ON` (e.g., `STA 1 VUE ISS`, where `1` is the Station ID).
* **Configuration**: In the ESPHome code, the `known_unit_id` variable is 0-indexed (Channel 1 = `0`, Channel 2 = `1`, etc.). By default, `davis_receiver.yaml` is hardcoded to listen exclusively to **ID 0** (Channel 1):
  ```yaml
  globals:
    - id: known_unit_id
      type: int
      initial_value: '0'
  ```
If your station is set to ID 2, change the `initial_value` to `'1'`, and so on.

## Optional: MQTT & Weather Underground

This project includes support for broadcasting your weather data to an MQTT broker and/or uploading it to Weather Underground. The MQTT payload structure  mimics the WeeWX `StdRESTful` JSON format (publishing to the `weather/loop` topic). This makes migration from a dead Davis data logger painless.

### Configuration
To use these features, add the following credentials to your local `.esphome/secrets.yaml` file:

```yaml
mqtt_broker: "192.168.1.100" # Your MQTT broker IP
mqtt_username: "my_mqtt_user"
mqtt_password: "my_mqtt_password"
wu_station_id: "KNYNORTH34" # Your Wunderground Station ID
wu_password: "your_wu_password"
```

### Disabling Features
Both MQTT and Weather Underground broadcasts are optional. If you prefer to run the ESP as a standalone receiver, or only want to use one of the two services, you can disable them.

To do so, open `davis_receiver.yaml` and comment out or delete the corresponding setup blocks and interval actions. Look for the `# --- OPTIONAL ---` comments in the file:
1. Comment out the `mqtt:` or `http_request:` setup block at the top of the file.
2. Comment out the corresponding `- interval:` action block inside the `interval:` component. 
*(Be careful not to delete the core `- interval: 30ms` radio tuning loop at the very bottom!)*

### Regional Frequencies (NA vs ROW) and Metric Support

Both North American (NA) and ROW (European/International (EU/UK/AU)) frequencies are supported, as well as metric unit conversions.

* **NA (North America) Frequencies:** Transmits over 51 channels across the 902-928 MHz spectrum. The receiver employs a fast-hopping "coast and hunt" algorithm to track the station.
* **ROW (EU/UK/AU) Frequencies:** Transmits over 5 channels around 868.3 MHz. For these regions, disable hopping and use a wide 812kHz filter bandwidth to capture all 5 channels.

To configure your region and output units, set the variables in the `davis_vantage:` block:

```yaml
# In davis_receiver.yaml
cc1101:
  # Remember to update the CC1101 base frequency for your region:
  # NA: 915.0MHz
  # ROW: 868.3MHz
  frequency: 915.0MHz
  # ...

davis_vantage:
  id: my_davis
  cc1101_id: my_cc1101
  unit_id: 0
  region: "NA"      # Set to "ROW" for European/International (868MHz) frequencies
  metric: false     # Set to true to output °C, km/h, and mm
```

If you set `metric: true`, be sure to also update your `unit_of_measurement` fields in the `sensor:` block so they are displayed correctly (e.g. `unit_of_measurement: "°C"`).
