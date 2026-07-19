#pragma once

#include "lps25hb_registers.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace lps25hb {

class LPS25HBComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }
  void set_pressure_sensor(sensor::Sensor *sensor) { this->pressure_sensor_ = sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  bool read_register_(lps25hb_core::registers::RegisterId id, uint8_t *value);
  bool write_register_(lps25hb_core::registers::RegisterId id, uint8_t value);
  bool read_who_am_i_(uint8_t &who);
  bool trigger_one_shot_();
  bool wait_data_ready_(uint32_t timeout_ms);
  bool read_measurements_(float &temperature_c, float &pressure_hpa);

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *pressure_sensor_{nullptr};
};

}  // namespace lps25hb
}  // namespace esphome
