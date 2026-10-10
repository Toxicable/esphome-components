#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace adp910 {

class ADP910Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_pressure_sensor(sensor::Sensor *sensor) { this->pressure_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  bool read_measurement_(int16_t &pressure_raw, int16_t &temperature_raw);
  bool verify_product_type_();
  static uint8_t crc8_(const uint8_t *data, size_t length);

  sensor::Sensor *pressure_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
};

}  // namespace adp910
}  // namespace esphome
