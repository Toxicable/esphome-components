#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace a02yyuw {

class A02YYUWComponent : public PollingComponent, public uart::UARTDevice {
public:
  void set_distance_sensor(sensor::Sensor *distance_sensor) { distance_sensor_ = distance_sensor; }

  void update() override;
  void loop() override;
  void dump_config() override;

protected:
  sensor::Sensor *distance_sensor_{nullptr};
  uint8_t buffer_[4]{};
  uint8_t buffer_index_{0};
  uint32_t last_byte_time_{0};
};

} // namespace a02yyuw
} // namespace esphome
