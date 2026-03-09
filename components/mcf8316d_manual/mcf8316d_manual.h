#pragma once

#include <cstdint>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mcf8316d_manual {

class MCF8316DManualComponent;

class MCF8316DBrakeSwitch : public switch_::Switch {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void write_state(bool state) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DDirectionSelect : public select::Select {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void control(const std::string &value) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DSpeedNumber : public number::Number {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void control(float value) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DClearFaultsButton : public button::Button {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DWatchdogTickleButton : public button::Button {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DManualComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_reg32(uint16_t offset, uint32_t &value);
  bool write_reg32(uint16_t offset, uint32_t value);
  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value);

  bool set_brake_override(bool brake_on);
  bool set_direction_mode(const std::string &direction_mode);
  bool set_speed_percent(float speed_percent);
  void pulse_clear_faults();
  void pulse_watchdog_tickle();

  void set_inter_byte_delay_us(uint32_t inter_byte_delay_us) { inter_byte_delay_us_ = inter_byte_delay_us; }
  void set_auto_tickle_watchdog(bool auto_tickle_watchdog) { auto_tickle_watchdog_ = auto_tickle_watchdog; }

  void set_brake_switch(MCF8316DBrakeSwitch *sw) { brake_switch_ = sw; }
  void set_direction_select(MCF8316DDirectionSelect *sel) { direction_select_ = sel; }
  void set_speed_number(MCF8316DSpeedNumber *num) { speed_number_ = num; }
  void set_fault_active_binary_sensor(binary_sensor::BinarySensor *s) { fault_active_binary_sensor_ = s; }
  void set_sys_enable_binary_sensor(binary_sensor::BinarySensor *s) { sys_enable_binary_sensor_ = s; }
  void set_vm_voltage_sensor(sensor::Sensor *s) { vm_voltage_sensor_ = s; }
  void set_duty_cmd_percent_sensor(sensor::Sensor *s) { duty_cmd_percent_sensor_ = s; }
  void set_volt_mag_percent_sensor(sensor::Sensor *s) { volt_mag_percent_sensor_ = s; }
  void set_fault_summary_text_sensor(text_sensor::TextSensor *s) { fault_summary_text_sensor_ = s; }

 protected:
  bool read_probe_and_publish_();
  bool perform_read_(uint16_t offset, uint32_t &value);
  bool perform_write_(uint16_t offset, uint32_t value);
  uint32_t build_control_word_(bool is_read, uint16_t offset) const;
  void delay_between_bytes_() const;
  void publish_faults_(uint32_t fault_status);
  void publish_algo_status_(uint32_t algo_status);
  void handle_fault_shutdown_(bool fault_active);

  static constexpr uint16_t REG_CONTROLLER_FAULT_STATUS = 0x00E2;
  static constexpr uint16_t REG_ALGO_STATUS = 0x00E4;
  static constexpr uint16_t REG_ALGO_CTRL1 = 0x00EA;
  static constexpr uint16_t REG_ALGO_DEBUG1 = 0x00EC;
  static constexpr uint16_t REG_PIN_CONFIG = 0x00A4;
  static constexpr uint16_t REG_PERI_CONFIG1 = 0x00AA;
  static constexpr uint16_t REG_VM_VOLTAGE = 0x047C;

  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_MASK = (0x3u << 10);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_BRAKE = (0x1u << 10);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_NO_BRAKE = (0x2u << 10);

  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_MASK = (0x3u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_HARDWARE = (0x0u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CW = (0x1u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CCW = (0x2u << 0);

  static constexpr uint32_t ALGO_DEBUG1_OVERRIDE_MASK = (1u << 31);
  static constexpr uint32_t ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK = (0x7FFFu << 16);

  static constexpr uint32_t ALGO_CTRL1_CLR_FLT_MASK = (1u << 0);
  static constexpr uint32_t ALGO_CTRL1_WATCHDOG_TICKLE_MASK = (1u << 1);

  static constexpr uint32_t ALGO_STATUS_DUTY_CMD_MASK = 0x0FFFu;
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_MASK = (0x7FFFu << 16);
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_SHIFT = 16;
  static constexpr uint32_t ALGO_STATUS_SYS_ENABLE_FLAG_MASK = (1u << 15);

  static constexpr uint32_t CONTROLLER_FAULT_ACTIVE_MASK = (1u << 0);
  static constexpr uint32_t FAULT_WATCHDOG = (1u << 1);
  static constexpr uint32_t FAULT_NO_MTR = (1u << 2);
  static constexpr uint32_t FAULT_MTR_LCK = (1u << 3);
  static constexpr uint32_t FAULT_ABN_SPEED = (1u << 4);
  static constexpr uint32_t FAULT_ABN_BEMF = (1u << 5);
  static constexpr uint32_t FAULT_MTR_UNDER_VOLTAGE = (1u << 6);
  static constexpr uint32_t FAULT_MTR_OVER_VOLTAGE = (1u << 7);
  static constexpr uint32_t FAULT_I2C_CRC = (1u << 8);
  static constexpr uint32_t FAULT_EEPROM_ERR = (1u << 9);

  uint32_t inter_byte_delay_us_{100};
  bool auto_tickle_watchdog_{false};
  uint32_t last_watchdog_tickle_ms_{0};
  bool fault_latched_{false};

  MCF8316DBrakeSwitch *brake_switch_{nullptr};
  MCF8316DDirectionSelect *direction_select_{nullptr};
  MCF8316DSpeedNumber *speed_number_{nullptr};
  binary_sensor::BinarySensor *fault_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *sys_enable_binary_sensor_{nullptr};
  sensor::Sensor *vm_voltage_sensor_{nullptr};
  sensor::Sensor *duty_cmd_percent_sensor_{nullptr};
  sensor::Sensor *volt_mag_percent_sensor_{nullptr};
  text_sensor::TextSensor *fault_summary_text_sensor_{nullptr};
};

}  // namespace mcf8316d_manual
}  // namespace esphome
