#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq76922 {

class BQ76922Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_cell_count(uint8_t cell_count) {
    cell_count_ = cell_count;
  }
  void set_autonomous_fet_mode(uint8_t mode) {
    autonomous_fet_mode_ = mode;
  }
  void set_sleep_mode(uint8_t mode) {
    sleep_mode_ = mode;
  }

  void set_stack_voltage_sensor(sensor::Sensor* sensor) {
    stack_voltage_sensor_ = sensor;
  }
  void set_pack_voltage_sensor(sensor::Sensor* sensor) {
    pack_voltage_sensor_ = sensor;
  }
  void set_ld_voltage_sensor(sensor::Sensor* sensor) {
    ld_voltage_sensor_ = sensor;
  }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor);
  void set_current_sensor(sensor::Sensor* sensor) {
    current_sensor_ = sensor;
  }
  void set_die_temperature_sensor(sensor::Sensor* sensor) {
    die_temperature_sensor_ = sensor;
  }

  void set_security_state_sensor(text_sensor::TextSensor* sensor) {
    security_state_sensor_ = sensor;
  }
  void set_operating_mode_sensor(text_sensor::TextSensor* sensor) {
    operating_mode_sensor_ = sensor;
  }
  void set_power_path_state_sensor(text_sensor::TextSensor* sensor) {
    power_path_state_sensor_ = sensor;
  }
  void set_alarm_flags_sensor(text_sensor::TextSensor* sensor) {
    alarm_flags_sensor_ = sensor;
  }

  void set_sleep_mode_binary_sensor(binary_sensor::BinarySensor* sensor) {
    sleep_mode_binary_sensor_ = sensor;
  }
  void set_cfgupdate_binary_sensor(binary_sensor::BinarySensor* sensor) {
    cfgupdate_binary_sensor_ = sensor;
  }
  void set_protection_fault_binary_sensor(binary_sensor::BinarySensor* sensor) {
    protection_fault_binary_sensor_ = sensor;
  }
  void set_permanent_fail_binary_sensor(binary_sensor::BinarySensor* sensor) {
    permanent_fail_binary_sensor_ = sensor;
  }
  void set_sleep_allowed_state_binary_sensor(binary_sensor::BinarySensor* sensor) {
    sleep_allowed_state_binary_sensor_ = sensor;
  }
  void set_alert_pin_binary_sensor(binary_sensor::BinarySensor* sensor) {
    alert_pin_binary_sensor_ = sensor;
  }
  void set_chg_fet_on_binary_sensor(binary_sensor::BinarySensor* sensor) {
    chg_fet_on_binary_sensor_ = sensor;
  }
  void set_dsg_fet_on_binary_sensor(binary_sensor::BinarySensor* sensor) {
    dsg_fet_on_binary_sensor_ = sensor;
  }
  void set_autonomous_fet_enabled_binary_sensor(binary_sensor::BinarySensor* sensor) {
    autonomous_fet_enabled_binary_sensor_ = sensor;
  }

  void set_power_path_select(select::Select* sel) {
    power_path_select_ = sel;
  }
  void set_autonomous_fet_switch(switch_::Switch* sw) {
    autonomous_fet_switch_ = sw;
  }
  void set_sleep_allowed_switch(switch_::Switch* sw) {
    sleep_allowed_switch_ = sw;
  }

  void setup() override;
  void update() override;
  void dump_config() override;

  bool set_power_path_mode(const char* mode);
  bool set_autonomous_fet_control(bool enabled);
  bool set_sleep_allowed(bool allowed);
  bool clear_alarm_latches();

 protected:
  static constexpr uint8_t BOOT_PRESERVE = 0;
  static constexpr uint8_t BOOT_ENABLE = 1;
  static constexpr uint8_t BOOT_DISABLE = 2;

  bool read_byte_(uint8_t reg, uint8_t& value);
  bool read_bytes_(uint8_t reg, uint8_t* data, size_t len);
  bool write_byte_(uint8_t reg, uint8_t value);
  bool write_bytes_(uint8_t reg, const uint8_t* data, size_t len);

  bool read_u16_(uint8_t reg, uint16_t& value);
  bool read_i16_(uint8_t reg, int16_t& value);
  bool write_u16_(uint8_t reg, uint16_t value);

  bool write_subcommand_(uint16_t subcommand);
  bool wait_subcommand_ready_(uint16_t subcommand, uint32_t timeout_ms = 20);
  bool read_subcommand_(uint16_t subcommand, uint8_t* data, size_t len);
  bool read_subcommand_u16_(uint16_t subcommand, uint16_t& value);
  bool write_subcommand_data_(uint16_t subcommand, const uint8_t* data, size_t len);

  bool apply_boot_modes_();
  bool load_unit_scaling_();

  const char* security_state_to_string_(uint16_t battery_status) const;
  const char* operating_mode_to_string_(uint16_t battery_status, uint16_t control_status) const;
  const char* power_path_to_string_(uint8_t fet_status) const;
  std::string alarm_flags_to_string_(uint16_t alarm_status) const;
  void append_flag_(std::string& flags, const char* flag) const;

  uint8_t cell_count_{5};
  // Auto-detected from Settings:Configuration:DA Configuration (0x9303).
  int32_t current_lsb_ua_{1000};
  bool user_volts_cv_{true};
  uint8_t autonomous_fet_mode_{BOOT_PRESERVE};
  uint8_t sleep_mode_{BOOT_PRESERVE};
  std::array<uint8_t, 5> cell_read_map_{0, 1, 2, 3, 4};
  bool cell_map_initialized_{false};

  sensor::Sensor* stack_voltage_sensor_{nullptr};
  sensor::Sensor* pack_voltage_sensor_{nullptr};
  sensor::Sensor* ld_voltage_sensor_{nullptr};
  std::array<sensor::Sensor*, 5> cell_voltage_sensors_{};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* die_temperature_sensor_{nullptr};

  text_sensor::TextSensor* security_state_sensor_{nullptr};
  text_sensor::TextSensor* operating_mode_sensor_{nullptr};
  text_sensor::TextSensor* power_path_state_sensor_{nullptr};
  text_sensor::TextSensor* alarm_flags_sensor_{nullptr};

  binary_sensor::BinarySensor* sleep_mode_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* cfgupdate_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* protection_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* permanent_fail_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* sleep_allowed_state_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* alert_pin_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* chg_fet_on_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* dsg_fet_on_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* autonomous_fet_enabled_binary_sensor_{nullptr};

  select::Select* power_path_select_{nullptr};
  switch_::Switch* autonomous_fet_switch_{nullptr};
  switch_::Switch* sleep_allowed_switch_{nullptr};
};

class BQ76922PowerPathSelect : public select::Select, public Parented<BQ76922Component> {
 protected:
  void control(size_t index) override;
};

class BQ76922AutonomousFetSwitch : public switch_::Switch, public Parented<BQ76922Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76922SleepAllowedSwitch : public switch_::Switch, public Parented<BQ76922Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76922ClearAlarmsButton : public button::Button, public Parented<BQ76922Component> {
 protected:
  void press_action() override;
};

}  // namespace bq76922
}  // namespace esphome
