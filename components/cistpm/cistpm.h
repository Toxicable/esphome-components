#pragma once

#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace cistpm {

class CISTPMComponent : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_ph_sensor(sensor::Sensor *sensor) { this->ph_sensor_ = sensor; }
  void set_ph_temperature_sensor(sensor::Sensor *sensor) { this->ph_temperature_sensor_ = sensor; }
  void set_orp_sensor(sensor::Sensor *sensor) { this->orp_sensor_ = sensor; }
  void set_orp_temperature_sensor(sensor::Sensor *sensor) { this->orp_temperature_sensor_ = sensor; }
  void set_ph_address(uint64_t address) { this->ph_address_ = address; }
  void set_orp_address(uint64_t address) { this->orp_address_ = address; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void dump_config() override;

 protected:
  sensor::Sensor *ph_sensor_{nullptr};
  sensor::Sensor *ph_temperature_sensor_{nullptr};
  sensor::Sensor *orp_sensor_{nullptr};
  sensor::Sensor *orp_temperature_sensor_{nullptr};
  uint64_t ph_address_{0};
  uint64_t orp_address_{0};
};

}  // namespace cistpm
}  // namespace esphome
