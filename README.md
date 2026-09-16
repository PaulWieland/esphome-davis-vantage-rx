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

*Note: Ensure you are using an antenna suitable for 915MHz (US) for the best reception. The standard small coiled antennas included with generic CC1101 modules may have very limited range.*

## Setup Instructions

1. Install ESPHome (e.g., via the command line `pip install esphome`).
2. Update the `wifi` section in `davis_receiver.yaml` (or via your `secrets.yaml` file) with your local Wi-Fi credentials.
3. Flash the configuration to your ESP32 via USB for the first time:
   ```bash
   esphome run davis_receiver.yaml
   ```
4. The ESPHome log output will show raw RF packet hex data and decoded weather metrics.

## Frequency Hopping & Synchronization

The Davis Vantage Vue (US 915MHz model) transmits data using a Frequency Hopping Spread Spectrum (FHSS) protocol across 51 different channels. A packet is broadcast roughly every 2.56 seconds. 

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
If your station is set to ID 2, change the `initial_value` to `'1'`, and so on. This ensures your receiver permanently ignores any neighboring Davis stations broadcasting on different IDs!
