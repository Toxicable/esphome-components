#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace l04xmtw {

class L04XMTWComponent : public PollingComponent, public uart::UARTDevice {
public:
  void set_distance_sensor(sensor::Sensor *distance_sensor) { distance_sensor_ = distance_sensor; }

  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;

protected:
  sensor::Sensor *distance_sensor_{nullptr};
  uint8_t buffer_[4]{};
  uint8_t buffer_index_{0};
  uint32_t last_byte_time_{0};
  bool waiting_{false};
  bool warned_this_cycle_{false};
  uint8_t consecutive_timeouts_{0};
  uint32_t trigger_ms_{0};
  uint32_t rx_deadline_ms_{0};
  static constexpr uint8_t TIMEOUT_WARN_THRESHOLD = 3;
  static constexpr uint32_t RX_WINDOW_MS = 80;
  static constexpr uint32_t INTERBYTE_TIMEOUT_MS = 30;
};

} // namespace l04xmtw
} // namespace esphome
