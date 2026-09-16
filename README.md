# Davis Vantage Vue ESP32 Receiver

This project uses an ESP32 and a CC1101 wireless transceiver module to intercept RF signals from a Davis Vantage Vue weather station, decode them using ESPHome, and eventually broadcast them via MQTT and upload to Weather Underground.

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

1. Install ESPHome (e.g., via Home Assistant or command line `pip install esphome`).
2. Update the `wifi` section in `davis_receiver.yaml` with your local Wi-Fi credentials.
3. Flash the configuration to your ESP32 via USB for the first time:
   ```bash
   esphome run davis_receiver.yaml
   ```
4. Open the ESPHome log output. You should start seeing raw RF packet hex data dumped to the console when the weather station transmits (approximately every 2.5 seconds, though with the passive frequency listener you may catch them a bit less frequently as it scans).

## Next Steps
Once you confirm you are receiving data:
1. Map the raw packet bits to specific sensors (temperature, wind, rain) in the lambda.
2. Publish these sensors via ESPHome's native MQTT component.
3. Use an automation, Node-RED, or a custom script to forward this data to Weather Underground.
