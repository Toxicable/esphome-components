#pragma once

#include <array>
#include <cstdint>

namespace esphome {
namespace bq76952 {

// Desired BQ76952 configuration populated by ESPHome code generation.
// Runtime protocol and telemetry state live separately in BQ76952ServiceState.
class BQ76952Configuration {
 public:
  void set_cell_count(uint8_t cell_count) { cell_count_ = cell_count; }
  void set_sense_resistor_milliohm(float value) { sense_resistor_milliohm_ = value; }
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
  void set_autonomous_fet_mode(uint8_t mode) { autonomous_fet_mode_ = mode; }
  void set_sleep_mode(uint8_t mode) { sleep_mode_ = mode; }
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
  void set_event_logging(bool value) { event_logging_ = value; }
  void set_boot_config_apply_delay_ms(uint32_t value) { boot_config_apply_delay_ms_ = value; }
  void set_xchg_debug_burst(bool value) { xchg_debug_burst_ = value; }
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
  void set_cell_channel(uint8_t index, uint8_t channel) {
    if (index >= 1 && index <= cell_read_map_.size() && channel >= 1 && channel <= cell_read_map_.size()) {
      cell_read_map_[index - 1] = channel - 1;
      explicit_cell_map_ = true;
    }
  }

 protected:
  static constexpr uint8_t BOOT_PRESERVE = 0;
  static constexpr uint8_t BOOT_ENABLE = 1;
  static constexpr uint8_t BOOT_DISABLE = 2;

  uint8_t cell_count_{16};
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
  bool explicit_cell_map_{false};
  uint32_t boot_config_apply_delay_ms_{10000};
};

}  // namespace bq76952
}  // namespace esphome
