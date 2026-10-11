#pragma once

#include "esphome.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <map>
#include <string>

namespace esphome {
namespace uponor_knx_rf_thermostat {

static const char *TAG = "UPONOR_RF";
static byte buffer[400] = {0xFF};

struct ThermostatState {
    uint32_t last_packet_time = 0;
    int burst_count = 0;
    uint32_t last_trigger_time = 0;
};

struct KNXDATA {
    byte data[200] = {0};
    int length;
    uint8_t battery_status;
    uint16_t serialNoHighWord;
    uint32_t serialNoLowWord;
    char sensor_id[13] = {0};
    uint16_t target_address;
    uint16_t sensor_data;
    int RSSI;
    int lqi;
    double temperature;
};

class UponorThermostat {
public:
    std::string id_;
    std::string name_;
    sensor::Sensor *current_temperature_sensor_;
    sensor::Sensor *target_temperature_sensor_;
    binary_sensor::BinarySensor *battery_state_sensor_;
    
    UponorThermostat(std::string id, std::string name, 
                     sensor::Sensor *curr_temp, 
                     sensor::Sensor *target_temp, 
                     binary_sensor::BinarySensor *battery)
        : id_(id), name_(name),
          current_temperature_sensor_(curr_temp),
          target_temperature_sensor_(target_temp),
          battery_state_sensor_(battery) {}

    void update_current_temp(double temp) {
        if (current_temperature_sensor_ != nullptr)
            current_temperature_sensor_->publish_state(temp);
    }

    void update_target_temp(double temp) {
        if (target_temperature_sensor_ != nullptr)
            target_temperature_sensor_->publish_state(temp);
    }

    void update_battery_state(int state) {
        if (battery_state_sensor_ != nullptr)
            battery_state_sensor_->publish_state(state == 0);
    }
};

class KNXRFGateway : public esphome::Component {
private:
    int knx_offset = 32;
    std::map<std::string, UponorThermostat*> thermostats_;
    std::map<std::string, ThermostatState> states_;
    
    uint8_t pin_sck_;
    uint8_t pin_miso_;
    uint8_t pin_mosi_;
    uint8_t pin_csn_;
    uint8_t pin_gdo0_;

public:
    KNXRFGateway(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t csn, uint8_t gdo0)
        : pin_sck_(sck), pin_miso_(miso), pin_mosi_(mosi), pin_csn_(csn), pin_gdo0_(gdo0) {}

    void add_thermostat(std::string id, std::string name, 
                        sensor::Sensor *curr_temp, 
                        sensor::Sensor *target_temp, 
                        binary_sensor::BinarySensor *battery) {
        thermostats_[id] = new UponorThermostat(id, name, curr_temp, target_temp, battery);
    }

    struct KNXDATA parse(struct KNXDATA knxdata) {
        knxdata.RSSI = ELECHOUSE_cc1101.getRssi();
        knxdata.lqi = ELECHOUSE_cc1101.getLqi();
    
        knxdata.battery_status = (get_knx_data(knxdata, 3) & 0x2) >> 1;
        knxdata.serialNoHighWord = get_knx_data(knxdata, 4);
        knxdata.serialNoHighWord = (knxdata.serialNoHighWord << 8) + get_knx_data(knxdata, 5);
        knxdata.serialNoLowWord = get_knx_data(knxdata, 6);
        knxdata.serialNoLowWord = (knxdata.serialNoLowWord << 8) + get_knx_data(knxdata, 7);
        knxdata.serialNoLowWord = (knxdata.serialNoLowWord << 8) + get_knx_data(knxdata, 8);
        knxdata.serialNoLowWord = (knxdata.serialNoLowWord << 8) + get_knx_data(knxdata, 9);
        sprintf(knxdata.sensor_id, "%04X%08lX", knxdata.serialNoHighWord, (unsigned long)knxdata.serialNoLowWord);

        knxdata.target_address = (get_knx_data(knxdata, 15) << 8) + get_knx_data(knxdata, 16);
        knxdata.sensor_data = (get_knx_data(knxdata, 20) << 8) + get_knx_data(knxdata, 21);
        knxdata.temperature = transformTemperature(knxdata.sensor_data) / 100.0;
        return knxdata;
    }

    byte get_knx_data(struct KNXDATA knxdata, int index) {
        return knxdata.data[knx_offset + index];
    }

    uint16_t transformTemperature(uint16_t data) {
        if (data & 0x800) data = (data & 0x7FF) * 2;
        return data;
    }

    int mandecode(unsigned int a1) {
        int ret = 0;
        for (int i = 0; i < 8; i++) {
            int b2 = (a1 >> ((7 - i) * 2)) & 0b11;
            switch (b2) {
            case 0b01: ret = (ret << 1) | 1; break;
            case 0b10: ret = (ret << 1) | 0; break;
            }
        }
        return ret;
    }

    void setup() override {
        ESP_LOGI(TAG, "Initializing CC1101 -> SCK: %d, MISO: %d, MOSI: %d, CSN: %d, GDO0: %d", 
                 pin_sck_, pin_miso_, pin_mosi_, pin_csn_, pin_gdo0_);
        ELECHOUSE_cc1101.setSpiPin(pin_sck_, pin_miso_, pin_mosi_, pin_csn_);
        ELECHOUSE_cc1101.setGDO0(pin_gdo0_);

        ELECHOUSE_cc1101.Init();
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.setModulation(0);
        ELECHOUSE_cc1101.setMHZ(868.3);
        ELECHOUSE_cc1101.setDeviation(47.607422);
        ELECHOUSE_cc1101.setChannel(0);
        ELECHOUSE_cc1101.setChsp(199.951172);
        ELECHOUSE_cc1101.setRxBW(270.833333);
        ELECHOUSE_cc1101.setDRate(32.7301);
        ELECHOUSE_cc1101.setPA(5);
        ELECHOUSE_cc1101.setSyncMode(5);
        ELECHOUSE_cc1101.setSyncWord(0x76, 0x96);
        ELECHOUSE_cc1101.setAdrChk(0);
        ELECHOUSE_cc1101.setAddr(0);
        ELECHOUSE_cc1101.setWhiteData(0);
        ELECHOUSE_cc1101.setPktFormat(0);
        ELECHOUSE_cc1101.setLengthConfig(0);
        ELECHOUSE_cc1101.setPacketLength(61);
        ELECHOUSE_cc1101.setCrc(0);
        ELECHOUSE_cc1101.setCRC_AF(0);
        ELECHOUSE_cc1101.setDcFilterOff(0);
        ELECHOUSE_cc1101.setManchester(0);
        ELECHOUSE_cc1101.setFEC(0);
        ELECHOUSE_cc1101.setPQT(0);
        ELECHOUSE_cc1101.setAppendStatus(1);
        ESP_LOGI(TAG, "CC1101 setup complete.");
    }

    void loop() override {
        if (ELECHOUSE_cc1101.CheckRxFifo(100)) {
            struct KNXDATA knxdata;
            int len = ELECHOUSE_cc1101.ReceiveData(buffer);
            buffer[len] = '\0';
            knxdata.length = len;
            byte pckidx = 0;
            for (int i = 0; i < len; i++) {
                if (i % 2 == 1) {
                    knxdata.data[pckidx] = mandecode(buffer[i - 1] * 256 + buffer[i]);
                    pckidx++;
                }
            }

            if (pckidx >= (knx_offset + 10)) {
                uint16_t hi = (knxdata.data[knx_offset + 4] << 8) + knxdata.data[knx_offset + 5];
                uint32_t lo = (((((knxdata.data[knx_offset + 6] << 8) + knxdata.data[knx_offset + 7]) << 8) + knxdata.data[knx_offset + 8]) << 8) + knxdata.data[knx_offset + 9];
                char temp_id[13];
                sprintf(temp_id, "%04X%08lX", hi, (unsigned long)lo);

                uint8_t first_byte = knxdata.data[0];

                // Detektera knapptryckning på typ 34 eller 14 via burst-intervall (~1.53s)
                if (first_byte == 0x34 || first_byte == 0x14) {
                    uint32_t now = millis();
                    ThermostatState &st = states_[temp_id];
                    uint32_t diff = now - st.last_packet_time;

                    if (st.last_packet_time > 0 && diff >= 1000 && diff <= 3000) {
                        st.burst_count++;
                    } else if (diff > 4000) {
                        st.burst_count = 1;
                    }
                    st.last_packet_time = now;

                    if (st.burst_count >= 2 && (now - st.last_trigger_time > 10000)) {
                        st.last_trigger_time = now;
                        st.burst_count = 0;

                        auto it = thermostats_.find(temp_id);
                        if (it != thermostats_.end()) {
                            ESP_LOGW(TAG, "🔘 KNAPPTRYCKNING DETEKTERAD på [%s] (ID: %s)", it->second->name_.c_str(), temp_id);
                        } else {
                            ESP_LOGW(TAG, "🔍 NY ENHET / KNAPPTRYCKNING Upptäckt! ID: %s (Anteckna klockslag och rum)", temp_id);
                        }
                    }
                }

                // Ordinarie parsning
                if (pckidx > 34 && knxdata.data[knx_offset + 1] == 68 && knxdata.data[knx_offset + 2] == 255) {
                    knxdata = parse(knxdata);
                    auto it = thermostats_.find(knxdata.sensor_id);
                    if (it != thermostats_.end()) {
                        UponorThermostat *th = it->second;
                        if (knxdata.target_address == 1) {
                            th->update_current_temp(knxdata.temperature);
                            th->update_battery_state(knxdata.battery_status);
                        } else if (knxdata.target_address == 2) {
                            th->update_target_temp(knxdata.temperature);
                        }
                    }
                }
            }
        }
    }
};

} // namespace uponor_knx_rf_thermostat
} // namespace esphome
