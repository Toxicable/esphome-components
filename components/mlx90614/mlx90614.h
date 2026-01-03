#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace mlx90614_esf {

class MLX90614ESFComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_ambient_sensor(sensor::Sensor *sensor) { ambient_sensor_ = sensor; }
  void set_object_sensor(sensor::Sensor *sensor) { object_sensor_ = sensor; }
  void set_object2_sensor(sensor::Sensor *sensor) { object2_sensor_ = sensor; }

  void set_verify_pec(bool verify) { verify_pec_ = verify; }
  void set_slave_address(uint8_t addr) { slave_address_ = addr; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  // RAM addresses (Ta/To) per datasheet table.
  static constexpr uint8_t RAM_TA_ = 0x06;
  static constexpr uint8_t RAM_TOBJ1_ = 0x07;
  static constexpr uint8_t RAM_TOBJ2_ = 0x08;

  bool read_temp_c_(uint8_t ram_addr, float *out_c);
  bool read_word_with_pec_(uint8_t command, uint16_t *out_word, uint8_t *out_pec);
  uint8_t crc8_(const uint8_t *data, size_t len) const;

  sensor::Sensor *ambient_sensor_{nullptr};
  sensor::Sensor *object_sensor_{nullptr};
  sensor::Sensor *object2_sensor_{nullptr};

  bool verify_pec_{true};
  uint8_t slave_address_{0x5A};
};

}  // namespace mlx90614_esf
}  // namespace esphome
