# ESPHome Uponor KNX RF Thermostat Component

An ESPHome external component to integrate **Uponor KNX RF wireless thermostats** (868.3 MHz) into Home Assistant using an affordable **CC1101 transceiver module** and an ESP32 microcontroller. 

---

## Features

* **Real-time Monitoring:** Tracks current room temperature, target temperature, and battery status.
* **YAML Configuration:** Easily configure your hardware pins and thermostat mappings directly in your ESPHome YAML.
* **Interactive Discovery Mode:** Built-in burst-detection for thermostat button presses. Simply walk around your house, press the link/transmission button on each thermostat, and watch the logs flag unknown IDs with clear timestamps for easy room mapping.
* **External Component Ready:** Pull the code directly from GitHub into your ESPHome project without local file clutter.

---

## Compatibility

* **Tested Model:** Uponor **T-55**
* **Likely Compatible:** Uponor **T-54** and **T-75** (use similar KNX RF 868.3 MHz protocol packets, though not extensively tested yet).

---

## Hardware Requirements

1. **Microcontroller:** ESP32 DevKit (ESP32-WROOM-32)
   * [Amazon Link (Example)](https://www.amazon.se/ESP32-WROOM-32-utvecklingskort-ESP-32S-Bluetooth-Arduino/dp/B084KWNMM4)
2. **RF Transceiver:** CC1101 868MHz Module
   * [Amazon Link (Example)](https://www.amazon.se/dp/B0FB3K3CQP?ref=ppx_yo2ov_dt_b_fed_asin_title)

---

## Wiring & GPIO Mapping

Connect the CC1101 module to your ESP32 using the following recommended wiring scheme (matching the default software configuration):

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

---

## Installation & Usage

Add this component as an `external_components` source in your ESPHome YAML configuration file:

```yaml
esphome:
  name: my-uponor-gateway

esp32:
  board: esp32dev

external_components:
  - source: github://skracke/esphome-uponor-thermostat@main
    components: [ uponor_knx_rf_thermostat ]

# Define template sensors for your thermostats
sensor:
  - platform: template
    id: vardagsrum_temp
    name: "Vardagsrum Current Temperature"
  - platform: template
    id: vardagsrum_target
    name: "Vardagsrum Target Temperature"

binary_sensor:
  - platform: template
    id: vardagsrum_battery
    name: "Vardagsrum Battery State"

# Configure the gateway and map your thermostats
uponor_knx_rf_thermostat:
  sck_pin: 22
  miso_pin: 23
  mosi_pin: 13
  csn_pin: 14
  gdo0_pin: 21
  thermostats:
    - address: "00744360E2B0"
      name: "Vardagsrum"
      current_temperature: vardagsrum_temp
      target_temperature: vardagsrum_target
      battery_state: vardagsrum_battery
