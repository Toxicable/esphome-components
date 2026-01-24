#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mlx90614 {

class MLX90614Component : public PollingComponent, public i2c::I2CDevice {
public:
  void set_slave_address(uint8_t addr) { slave_address_ = addr; }

  void set_ambient_sensor(sensor::Sensor *s) { ambient_sensor_ = s; }
  void set_object_sensor(sensor::Sensor *s) { object_sensor_ = s; }
  void set_object2_sensor(sensor::Sensor *s) { object2_sensor_ = s; }

  void setup() override;
  void update() override;
  void dump_config() override;

protected:
  // MLX90614 RAM addresses
  static constexpr uint8_t RAM_TA_ = 0x06;
  static constexpr uint8_t RAM_TOBJ1_ = 0x07;
  static constexpr uint8_t RAM_TOBJ2_ = 0x08;

  bool read_temp_c_(uint8_t ram_addr, float *out_c);
  bool read_word_with_pec_(uint8_t command, uint16_t *out_word);
  uint8_t crc8_smbus_(const uint8_t *data, size_t len) const;

  sensor::Sensor *ambient_sensor_{nullptr};
  sensor::Sensor *object_sensor_{nullptr};
  sensor::Sensor *object2_sensor_{nullptr};

  uint8_t slave_address_{0x5A};
};

} // namespace mlx90614
} // namespace esphome
