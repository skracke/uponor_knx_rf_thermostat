# ESPHome Uponor KNX RF Thermostat Component

An ESPHome external component to integrate **Uponor KNX RF wireless thermostats** (868.3 MHz) into Home Assistant using an affordable **CC1101 transceiver module** and an ESP8266/ESP32 microcontroller. 

---

## Features

* **Real-time Monitoring:** Tracks current room temperature, target temperature, and battery status.
* **YAML Configuration:** Easily configure your hardware pins and thermostat mappings directly in your ESPHome YAML.
* **Interactive Discovery Mode:** Built-in burst-detection for thermostat button presses. Simply walk around your house, press the link/transmission button on each thermostat, and watch the logs flag unknown IDs with clear timestamps for easy room mapping.
* **External Component Ready:** Pull the code directly from GitHub into your ESPHome project without local file clutter.

---

## Compatibility

* **Tested Model:** Uponor **T-55**
* **Likely Compatible:** Uponor **T-54** and **T-75** (use similar KNX RF 868.3 MHz protocol packets, though not tested yet).

---

## Hardware Requirements

1. **Microcontroller:** ESP8266 "D1 mini"
   * [Amazon Link (Example)](https://www.amazon.se/-/en/EPLZON-Development-ESP8266-12F-Connector-Compatible/dp/B0DPJWJX1X?th=1)
2. **or Microcontroller:** ESP32 DevKit (ESP32-WROOM-32)
   * [Amazon Link (Example)](https://www.amazon.se/ESP32-WROOM-32-utvecklingskort-ESP-32S-Bluetooth-Arduino/dp/B084KWNMM4)
3. **RF Transceiver:** CC1101 868MHz Module
   * [Amazon Link (Example)](https://www.amazon.se/dp/B0FB3K3CQP?ref=ppx_yo2ov_dt_b_fed_asin_title)

---

## Wiring & GPIO Mapping

Connect the CC1101 module to your D1 mini (ESP8266) using the following recommended wiring scheme (matching the default software configuration):

| CC1101 Pin | D1 mini Pin | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply (Ensure a stable 3.3V supply) |
| **GND** | GND | Ground |
| **CSN** | GPIO 5 (D1) | SPI Chip Select |
| **SCK** | GPIO 14 (D5) | SPI Clock |
| **MOSI** | GPIO 13 (D7) | SPI Master Out Slave In |
| **MISO** | GPIO 12 (D6) | SPI Master In Slave Out |
| **GDO0** | GPIO 4  (D2) | Digital Output 0 (RX/TX Packet Indicator) |



Connect the CC1101 module to your ESP32 using the following recommended wiring scheme:

| CC1101 Pin | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply (Ensure a stable 3.3V supply) |
| **GND** | GND | Ground |
| **CSN** | GPIO 14 | SPI Chip Select |
| **SCK** | GPIO 22 | SPI Clock |
| **MOSI** | GPIO 13 | SPI Master Out Slave In |
| **MISO** | GPIO 23 | SPI Master In Slave Out |
| **GDO0** | GPIO 21 | Digital Output 0 (RX/TX Packet Indicator) |

> *Note: Double-check your specific CC1101 module's pinout, as label orders can sometimes vary between manufacturers.*

*Connection diagram D1 mini*
![Wiring detail](https://github.com/skracke/uponor_knx_rf_thermostat/blob/main/D1_mini%20%26%20CC1101%20connection%20diagram.png)
---

## Installation & Usage

Add this component as an `external_components` source in your ESPHome YAML configuration file:

```yaml
esphome:
  name: uponor-knxrf
  friendly_name: Uponor-KNXRF
  libraries:
    - SPI
    - SmartRC-CC1101-Driver-Lib

esp8266:
  board: d1_mini

external_components:
  - source: github://skracke/uponor_knx_rf_thermostat@main
    components: [ uponor_knx_rf_thermostat ]

sensor:
  - platform: template
    id: livingroom_temp
    name: "Livingroom temperature"
    device_class: temperature
    accuracy_decimals: 1
    filters:
      - timeout: 4h
  - platform: template
    id: livingroom_target
    name: "Livingroom target temperature"
    device_class: temperature
    accuracy_decimals: 1
    filters:
      - timeout: 4h

binary_sensor:
  - platform: template
    id: livingroom_battery
    name: "Livingroom battery state"
    device_class: battery

spi:
  clk_pin: GPIO14
  miso_pin: GPIO12
  mosi_pin: GPIO13

uponor_knx_rf_thermostat:
  sck_pin: 14
  miso_pin: 12
  mosi_pin: 13
  csn_pin: 5
  gdo0_pin: 4
  thermostats:
    - address: "00744360E2B2"
      name: "Uterum"
      current_temperature: livingroom_temp
      target_temperature: livingroom_target
      battery_state: livingroom_battery
```

---

```yaml
esphome:
  name: my-uponor-gateway
  libraries:
    - SmartRC-CC1101-Driver-Lib
    - SPI

esp32:
  board: esp32dev
  framework:
    type: arduino

external_components:
  - source: github://skracke/uponor_knx_rf_thermostat@main
    components: [ uponor_knx_rf_thermostat ]

sensor:
  - platform: template
    id: livingroom_temp
    name: "Livingroom temperature"
    device_class: temperature
    accuracy_decimals: 1
    filters:
      - timeout: 4h
  - platform: template
    id: livingroom_target
    name: "Livingroom target temperature"
    device_class: temperature
    accuracy_decimals: 1
    filters:
      - timeout: 4h

binary_sensor:
  - platform: template
    id: livingroom_battery
    name: "Livingroom battery state"
    device_class: battery

spi:
  clk_pin: GPIO14
  miso_pin: GPIO12
  mosi_pin: GPIO13

uponor_knx_rf_thermostat:
  sck_pin: 14
  miso_pin: 12
  mosi_pin: 13
  csn_pin: 5
  gdo0_pin: 4
  thermostats:
    - address: "00744360E2B2"
      name: "Uterum"
      current_temperature: livingroom_temp
      target_temperature: livingroom_target
      battery_state: livingroom_battery
```

---

## How to Discover New Thermostats

1. Flash your ESP32 with the component installed and open the **ESPHome logs**.
2. Go to a room with an unconfigured thermostat and press its link/transmission button.
3. The component detects the burst transmission and outputs a warning message in the logs, e.g.:
   ```text
   [18:14:38.929][W][UPONOR_RF:207]: 🔍 NEW DEVICE / BUTTON PRESS Detected! ID: 00744360E2C5 (Note timestamp and room)
   ```
4. Note the device ID (`00744360E2C5`) and map it to your desired room name in your YAML configuration file.

---

## License

MIT License

Copyright (c) 2026 skracke

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
