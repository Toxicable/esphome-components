#pragma once

#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace esc_higher {

struct STM32TempReadResult {
  bool ok{false};
  uint8_t status{0xFF};
  int16_t temp_c{0};
  uint8_t fault{0};
  const char* error_message{"uninitialized"};
};

class ESCHigherComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  STM32TempReadResult read_stm32_temp_raw();
  void set_temperature_c_sensor(sensor::Sensor* s) {
    temperature_c_sensor_ = s;
  }
  void set_status_sensor(sensor::Sensor* s) {
    status_sensor_ = s;
  }
  void set_fault_sensor(sensor::Sensor* s) {
    fault_sensor_ = s;
  }

 protected:
  sensor::Sensor* temperature_c_sensor_{nullptr};
  sensor::Sensor* status_sensor_{nullptr};
  sensor::Sensor* fault_sensor_{nullptr};
};

}  // namespace esc_higher
}  // namespace esphome
