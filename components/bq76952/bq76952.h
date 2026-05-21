#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
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
namespace bq76952 {

class BQ76952Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_cell_count(uint8_t cell_count) {
    cell_count_ = cell_count;
  }
  void set_sense_resistor_milliohm(float value) {
    sense_resistor_milliohm_ = value;
  }
  void set_charge_current_limit_a(float value) {
    charge_current_limit_a_ = value;
    has_charge_current_limit_ = true;
  }
  void set_discharge_current_limit_a(float value) {
    discharge_current_limit_a_ = value;
    has_discharge_current_limit_ = true;
  }
  void set_charge_current_delay_ms(uint16_t value) {
    charge_current_delay_ms_ = value;
    has_charge_current_delay_ = true;
  }
  void set_discharge_current_delay_ms(uint16_t value) {
    discharge_current_delay_ms_ = value;
    has_discharge_current_delay_ = true;
  }
  void set_current_recovery_time_s(uint8_t value) {
    current_recovery_time_s_ = value;
    has_current_recovery_time_ = true;
  }
  void set_reg0_enabled(bool value) {
    reg0_enabled_ = value;
    has_reg0_config_ = true;
  }
  void set_reg1_enabled(bool value) {
    reg1_enabled_ = value;
    has_reg1_enabled_config_ = true;
  }
  void set_reg1_voltage(uint8_t value) {
    reg1_voltage_code_ = value;
    has_reg1_voltage_config_ = true;
  }
  void set_autonomous_fet_mode(uint8_t mode) {
    autonomous_fet_mode_ = mode;
  }
  void set_sleep_mode(uint8_t mode) {
    sleep_mode_ = mode;
  }
  void set_predischarge_enabled(bool value) {
    predischarge_enabled_ = value;
    has_predischarge_setting_ = true;
  }
  void set_apply_configuration_on_boot(bool value) {
    apply_configuration_on_boot_ = value;
  }
  void set_ts1_pullup_180k(bool value) {
    has_ts1_config_ = true;
    ts1_pullup_180k_ = value;
  }
  void set_ts2_pullup_180k(bool value) {
    has_ts2_config_ = true;
    ts2_pullup_180k_ = value;
  }
  void set_ts3_pullup_180k(bool value) {
    has_ts3_config_ = true;
    ts3_pullup_180k_ = value;
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
  void set_passed_charge_sensor(sensor::Sensor* sensor) {
    passed_charge_sensor_ = sensor;
  }
  void set_passed_charge_time_sensor(sensor::Sensor* sensor) {
    passed_charge_time_sensor_ = sensor;
  }
  void set_die_temperature_sensor(sensor::Sensor* sensor) {
    die_temperature_sensor_ = sensor;
  }
  void set_ts1_temperature_sensor(sensor::Sensor* sensor) {
    ts1_temperature_sensor_ = sensor;
  }
  void set_ts2_temperature_sensor(sensor::Sensor* sensor) {
    ts2_temperature_sensor_ = sensor;
  }
  void set_ts3_temperature_sensor(sensor::Sensor* sensor) {
    ts3_temperature_sensor_ = sensor;
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
  void set_safety_status_flags_sensor(text_sensor::TextSensor* sensor) {
    safety_status_flags_sensor_ = sensor;
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
  void set_pdsg_fet_on_binary_sensor(binary_sensor::BinarySensor* sensor) {
    pdsg_fet_on_binary_sensor_ = sensor;
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
  bool reset_passed_charge_counter();
  bool apply_requested_configuration();
  bool program_factory_otp_defaults();

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
  bool read_data_memory_u8_(uint16_t address, uint8_t& value);
  bool read_data_memory_u16_(uint16_t address, uint16_t& value);
  bool write_data_memory_u8_(uint16_t address, uint8_t value);
  bool write_data_memory_u16_(uint16_t address, uint16_t value);
  bool set_cfgupdate_mode_(bool enabled);
  bool has_current_limit_config_() const;
  bool has_regulator_config_() const;
  bool has_ts_pin_config_() const;
  bool has_predischarge_config_() const;
  bool has_boot_mode_config_() const;
  bool apply_boot_modes_();
  bool apply_requested_configuration_();
  bool apply_boot_mode_startup_defaults_();
  bool apply_regulator_config_();
  bool load_unit_scaling_();
  bool apply_ts_pin_config_();
  bool apply_predischarge_config_();
  bool apply_current_limit_config_();
  uint8_t encode_current_threshold_code_(float current_a, uint8_t min_code, uint8_t max_code, const char* label);
  uint8_t encode_current_delay_code_(uint16_t delay_ms, const char* label);
  bool precheck_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& needs_write);
  bool precheck_data_memory_value_(uint16_t address, uint8_t desired_value, const char* label, bool& needs_write);
  bool ensure_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& ok);
  bool write_data_memory_value_if_needed_(uint16_t address, uint8_t desired_value, const char* label, bool& ok);

  const char* security_state_to_string_(uint16_t battery_status) const;
  const char* operating_mode_to_string_(uint16_t battery_status, uint16_t control_status) const;
  const char* power_path_to_string_(uint8_t fet_status) const;
  std::string alarm_flags_to_string_(uint16_t alarm_status) const;
  std::string safety_status_flags_to_string_(uint8_t status_a, uint8_t status_b, uint8_t status_c) const;
  void append_flag_(std::string& flags, const char* flag) const;

  uint8_t cell_count_{16};
  // Auto-detected from Settings:Configuration:DA Configuration (0x9303).
  int32_t current_lsb_ua_{1000};
  bool user_volts_cv_{true};
  float sense_resistor_milliohm_{1.0f};
  float charge_current_limit_a_{0.0f};
  float discharge_current_limit_a_{0.0f};
  uint16_t charge_current_delay_ms_{0};
  uint16_t discharge_current_delay_ms_{0};
  uint8_t current_recovery_time_s_{0};
  uint8_t reg1_voltage_code_{0};
  bool has_charge_current_limit_{false};
  bool has_discharge_current_limit_{false};
  bool has_charge_current_delay_{false};
  bool has_discharge_current_delay_{false};
  bool has_current_recovery_time_{false};
  bool has_predischarge_setting_{false};
  bool predischarge_enabled_{false};
  bool has_reg0_config_{false};
  bool has_reg1_enabled_config_{false};
  bool has_reg1_voltage_config_{false};
  bool reg0_enabled_{false};
  bool reg1_enabled_{false};
  bool has_ts1_config_{false};
  bool has_ts2_config_{false};
  bool has_ts3_config_{false};
  bool ts1_pullup_180k_{false};
  bool ts2_pullup_180k_{false};
  bool ts3_pullup_180k_{false};
  uint8_t autonomous_fet_mode_{BOOT_PRESERVE};
  uint8_t sleep_mode_{BOOT_PRESERVE};
  bool apply_configuration_on_boot_{true};
  std::array<uint8_t, 16> cell_read_map_{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  bool cell_map_initialized_{false};
  bool regulator_config_deferred_{false};
  bool current_limit_config_deferred_{false};
  bool ts_pin_config_deferred_{false};
  uint32_t deferred_boot_config_log_ms_{0};
  uint32_t deferred_boot_config_apply_ms_{0};

  sensor::Sensor* stack_voltage_sensor_{nullptr};
  sensor::Sensor* pack_voltage_sensor_{nullptr};
  sensor::Sensor* ld_voltage_sensor_{nullptr};
  std::array<sensor::Sensor*, 16> cell_voltage_sensors_{};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* passed_charge_sensor_{nullptr};
  sensor::Sensor* passed_charge_time_sensor_{nullptr};
  sensor::Sensor* die_temperature_sensor_{nullptr};
  sensor::Sensor* ts1_temperature_sensor_{nullptr};
  sensor::Sensor* ts2_temperature_sensor_{nullptr};
  sensor::Sensor* ts3_temperature_sensor_{nullptr};

  text_sensor::TextSensor* security_state_sensor_{nullptr};
  text_sensor::TextSensor* operating_mode_sensor_{nullptr};
  text_sensor::TextSensor* power_path_state_sensor_{nullptr};
  text_sensor::TextSensor* alarm_flags_sensor_{nullptr};
  text_sensor::TextSensor* safety_status_flags_sensor_{nullptr};

  binary_sensor::BinarySensor* sleep_mode_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* cfgupdate_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* protection_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* permanent_fail_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* sleep_allowed_state_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* alert_pin_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* chg_fet_on_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* dsg_fet_on_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* pdsg_fet_on_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* autonomous_fet_enabled_binary_sensor_{nullptr};

  select::Select* power_path_select_{nullptr};
  switch_::Switch* autonomous_fet_switch_{nullptr};
  switch_::Switch* sleep_allowed_switch_{nullptr};
};

class BQ76952PowerPathSelect : public select::Select, public Parented<BQ76952Component> {
 protected:
  void control(size_t index) override;
};

class BQ76952AutonomousFetSwitch : public switch_::Switch, public Parented<BQ76952Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76952SleepAllowedSwitch : public switch_::Switch, public Parented<BQ76952Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76952ClearAlarmsButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

class BQ76952ResetPassedChargeButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

class BQ76952ApplyConfigurationButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

class BQ76952ProgramFactoryOtpButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

}  // namespace bq76952
}  // namespace esphome
