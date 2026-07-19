#pragma once

#include <cstddef>
#include <cstdint>

#include "mlx90614_registers.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mlx90614 {

class MLX90614Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_slave_address(uint8_t address) { this->slave_address_ = address; }
  void set_ambient_sensor(sensor::Sensor *sensor) { this->ambient_sensor_ = sensor; }
  void set_object_sensor(sensor::Sensor *sensor) { this->object_sensor_ = sensor; }
  void set_object2_sensor(sensor::Sensor *sensor) { this->object2_sensor_ = sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  bool read_temp_c_(mlx90614_core::registers::RegisterId id, float *out_c);
  bool read_word_with_pec_(mlx90614_core::registers::RegisterId id, uint16_t *out_word);
  uint8_t crc8_smbus_(const uint8_t *data, size_t len) const;

  sensor::Sensor *ambient_sensor_{nullptr};
  sensor::Sensor *object_sensor_{nullptr};
  sensor::Sensor *object2_sensor_{nullptr};
  uint8_t slave_address_{0x5A};
};

}  // namespace mlx90614
}  // namespace esphome
