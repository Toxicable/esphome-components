#pragma once

#include <cstdint>
#include <string>

#include "husb238_bus.h"
#include "husb238_service.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace husb238 {

class HUSB238VoltageSelect;

class HUSB238Component : public PollingComponent, public i2c::I2CDevice, public ::husb238_core::RegisterBus {
 public:
  HUSB238Component();

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  bool read_register(uint8_t reg, uint8_t *value) override;
  bool write_register(uint8_t reg, uint8_t value) override;
  void delay_ms(uint32_t ms) override;

  void set_initial_request_voltage(uint8_t voltage) { this->initial_request_voltage_ = voltage; }
  void set_request_on_boot(bool request_on_boot) { this->request_on_boot_ = request_on_boot; }

  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_power_sensor(sensor::Sensor *sensor) { this->power_sensor_ = sensor; }
  void set_attached_binary_sensor(binary_sensor::BinarySensor *sensor) { this->attached_binary_sensor_ = sensor; }
  void set_cc2_connected_binary_sensor(binary_sensor::BinarySensor *sensor) { this->cc2_connected_binary_sensor_ = sensor; }
  void set_pd_response_text_sensor(text_sensor::TextSensor *sensor) { this->pd_response_text_sensor_ = sensor; }
  void set_available_pdos_text_sensor(text_sensor::TextSensor *sensor) { this->available_pdos_text_sensor_ = sensor; }
  void set_voltage_select(HUSB238VoltageSelect *select) { this->voltage_select_ = select; }

  bool request_voltage(uint8_t voltage);
  bool request_source_capabilities();
  bool hard_reset();

 protected:
  void publish_voltage_select_(uint8_t voltage);
  std::string build_available_pdos_string_(const ::husb238_core::SourcePdo *pdos) const;

  void maybe_run_boot_request_(bool attached);

  ::husb238_core::HusbService service_;
  uint8_t initial_request_voltage_{0};
  bool request_on_boot_{true};
  bool boot_request_pending_{false};
  uint32_t boot_request_due_ms_{0};

  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  binary_sensor::BinarySensor *attached_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *cc2_connected_binary_sensor_{nullptr};
  text_sensor::TextSensor *pd_response_text_sensor_{nullptr};
  text_sensor::TextSensor *available_pdos_text_sensor_{nullptr};
  HUSB238VoltageSelect *voltage_select_{nullptr};
};

class HUSB238VoltageSelect : public select::Select {
 public:
  void set_parent(HUSB238Component *parent) { this->parent_ = parent; }

 protected:
  void control(const std::string &value) override;
  HUSB238Component *parent_{nullptr};
};

class HUSB238HardResetButton : public button::Button {
 public:
  void set_parent(HUSB238Component *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  HUSB238Component *parent_{nullptr};
};

class HUSB238RefreshCapabilitiesButton : public button::Button {
 public:
  void set_parent(HUSB238Component *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  HUSB238Component *parent_{nullptr};
};

}  // namespace husb238
}  // namespace esphome
