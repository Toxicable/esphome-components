#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace lps25hb {

class LPS25HBComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_pressure_sensor(sensor::Sensor *s) { pressure_sensor_ = s; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *pressure_sensor_{nullptr};

  bool read_who_am_i_(uint8_t &who);
  bool trigger_one_shot_();
  bool wait_data_ready_(uint32_t timeout_ms);
  bool read_measurements_(float &temp_c, float &pressure_hpa);
};

}  // namespace lps25hb
}  // namespace esphome