#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

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
  void set_cell_undervoltage_limit_mv(uint16_t value) {
    cell_undervoltage_limit_mv_ = value;
    has_cell_undervoltage_limit_ = true;
  }
  void set_cell_undervoltage_delay_ms(uint16_t value) {
    cell_undervoltage_delay_ms_ = value;
    has_cell_undervoltage_delay_ = true;
  }
  void set_cell_overvoltage_limit_mv(uint16_t value) {
    cell_overvoltage_limit_mv_ = value;
    has_cell_overvoltage_limit_ = true;
  }
  void set_cell_overvoltage_delay_ms(uint16_t value) {
    cell_overvoltage_delay_ms_ = value;
    has_cell_overvoltage_delay_ = true;
  }
  void set_nominal_capacity_ah(float value) {
    nominal_capacity_ah_ = value;
    has_nominal_capacity_ah_ = true;
  }
  void set_charge_current_limit_a(float value) {
    charge_current_limit_a_ = value;
    has_charge_current_limit_ = true;
  }
  void set_discharge_current_limit_a(float value) {
    discharge_current_limit_a_ = value;
    has_discharge_current_limit_ = true;
  }
  void set_discharge_current_limit_2_a(float value) {
    discharge_current_limit_2_a_ = value;
    has_discharge_current_limit_2_ = true;
  }
  void set_discharge_current_limit_3_a(float value) {
    discharge_current_limit_3_a_ = value;
    has_discharge_current_limit_3_ = true;
  }
  void set_charge_current_delay_ms(uint16_t value) {
    charge_current_delay_ms_ = value;
    has_charge_current_delay_ = true;
  }
  void set_discharge_current_delay_ms(uint16_t value) {
    discharge_current_delay_ms_ = value;
    has_discharge_current_delay_ = true;
  }
  void set_discharge_current_delay_2_ms(uint16_t value) {
    discharge_current_delay_2_ms_ = value;
    has_discharge_current_delay_2_ = true;
  }
  void set_discharge_current_delay_3_s(uint8_t value) {
    discharge_current_delay_3_s_ = value;
    has_discharge_current_delay_3_ = true;
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
  void set_sleep_charge_enabled(bool value) {
    sleep_charge_enabled_ = value;
    has_sleep_charge_setting_ = true;
  }
  void set_autonomous_balancing_enabled(bool value) {
    autonomous_balancing_enabled_ = value;
    has_autonomous_balancing_setting_ = true;
  }
  void set_event_logging(bool value) {
    event_logging_ = value;
  }
  void set_boot_config_apply_delay_ms(uint32_t value) {
    boot_config_apply_delay_ms_ = value;
  }
  void set_xchg_debug_burst(bool value) {
    xchg_debug_burst_ = value;
  }
  void set_scd_threshold_mv(uint16_t value) {
    scd_threshold_mv_ = value;
    has_scd_threshold_ = true;
  }
  void set_scd_delay_us(uint16_t value) {
    scd_delay_us_ = value;
    has_scd_delay_ = true;
  }
  void set_scd_recovery_time_s(uint8_t value) {
    scd_recovery_time_s_ = value;
    has_scd_recovery_time_ = true;
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


  void set_bat_voltage_sensor(sensor::Sensor* sensor) {
    bat_voltage_sensor_ = sensor;
  }
  void set_pack_voltage_sensor(sensor::Sensor* sensor) {
    pack_voltage_sensor_ = sensor;
  }
  void set_ld_voltage_sensor(sensor::Sensor* sensor) {
    ld_voltage_sensor_ = sensor;
  }
  void set_largest_intercell_voltage_sensor(sensor::Sensor* sensor) {
    largest_intercell_voltage_sensor_ = sensor;
  }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor);
  void set_current_sensor(sensor::Sensor* sensor) {
    current_sensor_ = sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor* sensor) {
    state_of_charge_sensor_ = sensor;
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

  void set_bms_state_sensor(text_sensor::TextSensor* sensor) {
    bms_state_sensor_ = sensor;
  }
  void set_fault_sensor(text_sensor::TextSensor* sensor) {
    fault_sensor_ = sensor;
  }
  void set_fet_status_flags_sensor(text_sensor::TextSensor* sensor) {
    fet_status_flags_sensor_ = sensor;
  }

  void set_output_enabled_switch(switch_::Switch* sw) {
    output_enabled_switch_ = sw;
  }
  void set_autonomous_fet_switch(switch_::Switch* sw) {
    autonomous_fet_switch_ = sw;
  }

  void setup() override;
  void update() override;
  void dump_config() override;

  bool set_output_enabled(bool enabled);
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
  bool has_autonomous_balancing_config_() const;
  bool has_boot_mode_config_() const;
  bool apply_boot_modes_();
  bool apply_requested_configuration_();
  bool apply_boot_mode_startup_defaults_();
  bool apply_regulator_config_();
  bool load_unit_scaling_();
  bool apply_ts_pin_config_();
  bool apply_predischarge_config_();
  bool apply_autonomous_balancing_config_();
  bool apply_current_limit_config_();
  void maybe_log_event_(uint16_t control_status, uint16_t battery_status, uint8_t fet_status, uint16_t alarm_status,
                        bool have_alarm_status, uint8_t safety_status_a, uint8_t safety_status_b,
                        uint8_t safety_status_c, bool have_safety_status);
  uint8_t encode_current_threshold_code_(float current_a, uint8_t min_code, uint8_t max_code, const char* label);
  uint8_t encode_current_delay_code_(uint16_t delay_ms, const char* label);
  uint8_t encode_cell_voltage_threshold_code_(uint16_t threshold_mv, uint8_t min_code, uint8_t max_code,
                                              const char* label);
  uint16_t encode_voltage_delay_code_(uint16_t delay_ms, uint16_t min_code, uint16_t max_code, const char* label);
  bool precheck_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& needs_write);
  bool precheck_data_memory_value_(uint16_t address, uint8_t desired_value, const char* label, bool& needs_write);
  bool precheck_data_memory_value_u16_(uint16_t address, uint16_t desired_value, const char* label, bool& needs_write);
  bool ensure_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& ok);
  bool write_data_memory_value_if_needed_(uint16_t address, uint8_t desired_value, const char* label, bool& ok);
  bool write_data_memory_value_u16_if_needed_(uint16_t address, uint16_t desired_value, const char* label, bool& ok);

  const char* bms_state_to_string_(uint16_t battery_status, uint16_t control_status) const;
  const char* power_path_to_string_(uint8_t fet_status) const;
  std::string fault_to_string_(uint16_t battery_status, uint8_t status_a, uint8_t status_b, uint8_t status_c) const;
  std::string alarm_flags_to_string_(uint16_t alarm_status) const;
  std::string safety_status_flags_to_string_(uint8_t status_a, uint8_t status_b, uint8_t status_c) const;
  std::string fet_status_flags_to_string_(uint8_t fet_status) const;
  void append_flag_(std::string& flags, const char* flag) const;

  struct SocCurvePoint {
    uint16_t mv;
    float soc;
  };

  float estimate_soc_from_voltage_(int16_t cell_mv) const;
  float update_soc_(float current_a, float raw_passq_ah, int16_t min_cell_mv, int16_t max_cell_mv,
                    int16_t avg_cell_mv, uint8_t safety_status_a);
  void load_soc_state_();
  void save_soc_state_(bool force);
  void mark_soc_full_();
  void mark_soc_empty_();

  uint8_t cell_count_{16};
  // Auto-detected from Settings:Configuration:DA Configuration (0x9303).
  int32_t current_lsb_ua_{1000};
  bool user_volts_cv_{true};
  float sense_resistor_milliohm_{1.0f};
  uint16_t cell_undervoltage_limit_mv_{0};
  uint16_t cell_undervoltage_delay_ms_{0};
  uint16_t cell_overvoltage_limit_mv_{0};
  uint16_t cell_overvoltage_delay_ms_{0};
  float nominal_capacity_ah_{0.0f};
  uint16_t scd_threshold_mv_{0};
  uint16_t scd_delay_us_{0};
  uint8_t scd_recovery_time_s_{0};
  float charge_current_limit_a_{0.0f};
  float discharge_current_limit_a_{0.0f};
  float discharge_current_limit_2_a_{0.0f};
  float discharge_current_limit_3_a_{0.0f};
  uint16_t charge_current_delay_ms_{0};
  uint16_t discharge_current_delay_ms_{0};
  uint16_t discharge_current_delay_2_ms_{0};
  uint8_t discharge_current_delay_3_s_{0};
  uint8_t current_recovery_time_s_{0};
  uint8_t reg1_voltage_code_{0};
  bool has_cell_undervoltage_limit_{false};
  bool has_cell_undervoltage_delay_{false};
  bool has_cell_overvoltage_limit_{false};
  bool has_cell_overvoltage_delay_{false};
  bool has_charge_current_limit_{false};
  bool has_discharge_current_limit_{false};
  bool has_discharge_current_limit_2_{false};
  bool has_discharge_current_limit_3_{false};
  bool has_nominal_capacity_ah_{false};
  bool has_scd_threshold_{false};
  bool has_scd_delay_{false};
  bool has_scd_recovery_time_{false};
  bool has_charge_current_delay_{false};
  bool has_discharge_current_delay_{false};
  bool has_discharge_current_delay_2_{false};
  bool has_discharge_current_delay_3_{false};
  bool has_current_recovery_time_{false};
  bool has_predischarge_setting_{false};
  bool predischarge_enabled_{false};
  bool has_sleep_charge_setting_{false};
  bool sleep_charge_enabled_{false};
  bool has_autonomous_balancing_setting_{false};
  bool autonomous_balancing_enabled_{false};
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
  bool event_logging_{false};
  bool xchg_debug_burst_{false};
  std::array<uint8_t, 16> cell_read_map_{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  bool cell_map_initialized_{false};
  bool regulator_config_deferred_{false};
  bool current_limit_config_deferred_{false};
  bool ts_pin_config_deferred_{false};
  bool predischarge_config_deferred_{false};
  bool autonomous_balancing_config_deferred_{false};
  bool event_log_initialized_{false};
  uint8_t last_fet_status_{0};
  uint16_t last_alarm_status_{0};
  uint8_t last_safety_status_a_{0};
  uint8_t last_safety_status_b_{0};
  uint8_t last_safety_status_c_{0};
  uint16_t last_battery_status_fault_bits_{0};
  bool last_xchg_raw_valid_{false};
  bool last_xchg_raw_{false};
  uint16_t last_fet_control_subcommand_{0};
  uint32_t last_fet_control_subcommand_ms_{0};
  uint32_t deferred_boot_config_log_ms_{0};
  uint32_t deferred_boot_config_apply_ms_{0};
  uint32_t boot_config_apply_delay_ms_{10000};


  // --- SoC tuning parameters ---
  static constexpr int16_t SOC_FULL_MARGIN_MV = 20;
  static constexpr int16_t SOC_EMPTY_MARGIN_MV = 20;
  static constexpr uint32_t SOC_ENDPOINT_HOLD_S = 30;

  // --- Coulomb counter / logical accumulator ---
  float soc_logical_ah_{0.0f};
  float soc_last_raw_passq_ah_{0.0f};
  bool soc_have_last_raw_{false};

  // --- Endpoint anchors ---
  float soc_full_ah_{0.0f};
  float soc_empty_ah_{0.0f};
  bool soc_have_full_{false};
  bool soc_have_empty_{false};
  float soc_learned_span_ah_{0.0f};
  bool soc_have_span_{false};
  bool soc_span_provisional_{false};

  // --- Boot voltage estimate ---
  float soc_boot_estimate_fraction_{0.5f};
  float soc_boot_logical_ah_{0.0f};

  // --- Endpoint hold timers ---
  uint32_t soc_full_hold_start_ms_{0};
  uint32_t soc_empty_hold_start_ms_{0};
  bool soc_charge_seen_{false};
  bool soc_discharge_seen_{false};

  // --- Persistence ---
  uint32_t soc_last_save_ms_{0};

  static constexpr SocCurvePoint DEFAULT_LIION_CURVE_[] = {
    {2800, 0.0f}, {3000, 3.0f}, {3200, 8.0f}, {3300, 12.0f},
    {3500, 25.0f}, {3600, 40.0f}, {3700, 58.0f}, {3800, 75.0f},
    {3900, 86.0f}, {4000, 94.0f}, {4100, 98.0f}, {4200, 100.0f},
  };

  static constexpr float SOC_MAX_REASONABLE_DELTA_AH = 100.0f;

  static constexpr uint32_t SOC_PREF_NAMESPACE = 0xB7695200u;

  struct SocPersistedState {
    float logical_ah{std::numeric_limits<float>::quiet_NaN()};
    float last_raw_passq_ah{std::numeric_limits<float>::quiet_NaN()};
    float full_ah{std::numeric_limits<float>::quiet_NaN()};
    float empty_ah{std::numeric_limits<float>::quiet_NaN()};
    float learned_span_ah{std::numeric_limits<float>::quiet_NaN()};
    float last_soc_percent{std::numeric_limits<float>::quiet_NaN()};
    uint8_t flags{0};
  };

  static constexpr uint8_t SOC_HAVE_FULL = 0x01;
  static constexpr uint8_t SOC_HAVE_EMPTY = 0x02;
  static constexpr uint8_t SOC_HAVE_SPAN = 0x04;
  static constexpr uint8_t SOC_SPAN_PROVISIONAL = 0x08;

  decltype(global_preferences->make_preference<SocPersistedState>(0)) soc_pref_{};
  bool soc_pref_valid_{false};

  sensor::Sensor* bat_voltage_sensor_{nullptr};
  sensor::Sensor* pack_voltage_sensor_{nullptr};
  sensor::Sensor* ld_voltage_sensor_{nullptr};
  sensor::Sensor* largest_intercell_voltage_sensor_{nullptr};
  std::array<sensor::Sensor*, 16> cell_voltage_sensors_{};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* state_of_charge_sensor_{nullptr};
  sensor::Sensor* die_temperature_sensor_{nullptr};
  sensor::Sensor* ts1_temperature_sensor_{nullptr};
  sensor::Sensor* ts2_temperature_sensor_{nullptr};
  sensor::Sensor* ts3_temperature_sensor_{nullptr};

  text_sensor::TextSensor* bms_state_sensor_{nullptr};
  text_sensor::TextSensor* fault_sensor_{nullptr};
  text_sensor::TextSensor* fet_status_flags_sensor_{nullptr};

  switch_::Switch* output_enabled_switch_{nullptr};
  switch_::Switch* autonomous_fet_switch_{nullptr};
};

class BQ76952OutputEnabledSwitch : public switch_::Switch, public Parented<BQ76952Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76952AutonomousFetSwitch : public switch_::Switch, public Parented<BQ76952Component> {
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
