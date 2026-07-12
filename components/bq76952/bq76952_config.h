#pragma once

#include <cstdint>

namespace esphome {
namespace bq76952 {

// Complete desired configuration for one BQ76952 instance. ESPHome builds this
// aggregate once and passes it to the component with set_config(). Fields that
// are not configured retain their defaults and their matching has_* flag stays
// false.
struct BQ76952Config {
  static constexpr uint8_t BOOT_PRESERVE = 0;
  static constexpr uint8_t BOOT_ENABLE = 1;
  static constexpr uint8_t BOOT_DISABLE = 2;

  // Core topology and component policy.
  uint8_t cell_count_{16};
  float sense_resistor_milliohm_{1.0f};
  uint32_t boot_config_apply_delay_ms_{10000};
  bool event_logging_{false};
  bool xchg_debug_burst_{false};

  // OTP-backed startup policy.
  uint8_t autonomous_fet_mode_{BOOT_PRESERVE};
  uint8_t sleep_mode_{BOOT_PRESERVE};

  // Regulators and multifunction pins.
  bool has_reg0_config_{false};
  bool reg0_enabled_{false};
  bool has_reg1_enabled_config_{false};
  bool reg1_enabled_{false};
  bool has_reg1_voltage_config_{false};
  uint8_t reg1_voltage_code_{0};
  bool has_ts1_config_{false};
  bool ts1_pullup_180k_{false};
  bool has_ts2_config_{false};
  bool ts2_pullup_180k_{false};
  bool has_ts3_config_{false};
  bool ts3_pullup_180k_{false};

  // FET and balancing mode enables.
  bool has_predischarge_setting_{false};
  bool predischarge_enabled_{false};
  bool has_sleep_charge_setting_{false};
  bool sleep_charge_enabled_{false};
  bool has_autonomous_balancing_setting_{false};
  bool autonomous_balancing_enabled_{false};

  // Pack model used by host-derived SoC.
  bool has_nominal_capacity_ah_{false};
  float nominal_capacity_ah_{0.0f};

  // Cell voltage protections.
  bool has_cell_undervoltage_limit_{false};
  uint16_t cell_undervoltage_limit_mv_{0};
  bool has_cell_undervoltage_delay_{false};
  uint16_t cell_undervoltage_delay_ms_{0};
  bool has_cell_overvoltage_limit_{false};
  uint16_t cell_overvoltage_limit_mv_{0};
  bool has_cell_overvoltage_delay_{false};
  uint16_t cell_overvoltage_delay_ms_{0};

  // Current and short-circuit protections.
  bool has_charge_current_limit_{false};
  float charge_current_limit_a_{0.0f};
  bool has_charge_current_delay_{false};
  uint16_t charge_current_delay_ms_{0};
  bool has_discharge_current_limit_{false};
  float discharge_current_limit_a_{0.0f};
  bool has_discharge_current_delay_{false};
  uint16_t discharge_current_delay_ms_{0};
  bool has_discharge_current_limit_2_{false};
  float discharge_current_limit_2_a_{0.0f};
  bool has_discharge_current_delay_2_{false};
  uint16_t discharge_current_delay_2_ms_{0};
  bool has_discharge_current_limit_3_{false};
  float discharge_current_limit_3_a_{0.0f};
  bool has_discharge_current_delay_3_{false};
  uint8_t discharge_current_delay_3_s_{0};
  bool has_scd_threshold_{false};
  uint16_t scd_threshold_mv_{0};
  bool has_scd_delay_{false};
  uint16_t scd_delay_us_{0};
  bool has_scd_recovery_time_{false};
  uint8_t scd_recovery_time_s_{0};
  bool has_current_recovery_time_{false};
  uint8_t current_recovery_time_s_{0};

  // Definitions for configuration areas not yet applied by the implementation.
  // These mirror capabilities used by the Libre Solar driver so subsequent
  // functional PRs can wire them in without growing the component API again.

  // Communication and current calibration.
  bool i2c_crc_enabled_{false};
  bool configure_current_gain_from_sense_resistor_{false};

  // Voltage protection recovery behaviour.
  bool has_cell_undervoltage_recovery_hysteresis_{false};
  uint16_t cell_undervoltage_recovery_hysteresis_mv_{0};
  bool has_cell_overvoltage_recovery_hysteresis_{false};
  uint16_t cell_overvoltage_recovery_hysteresis_mv_{0};

  // Temperature protections. Recovery points can be derived from the shared
  // hysteresis in the same way as the reference implementation.
  bool has_charge_overtemperature_limit_{false};
  int8_t charge_overtemperature_limit_c_{0};
  bool has_discharge_overtemperature_limit_{false};
  int8_t discharge_overtemperature_limit_c_{0};
  bool has_charge_undertemperature_limit_{false};
  int8_t charge_undertemperature_limit_c_{0};
  bool has_discharge_undertemperature_limit_{false};
  int8_t discharge_undertemperature_limit_c_{0};
  bool has_temperature_hysteresis_{false};
  uint8_t temperature_hysteresis_c_{0};

  // Automatic pre-discharge termination.
  bool has_predischarge_timeout_{false};
  uint16_t predischarge_timeout_ms_{0};
  bool has_predischarge_stop_delta_{false};
  uint16_t predischarge_stop_delta_mv_{0};

  // Automatic balancing thresholds and limits.
  bool has_balancing_min_cell_voltage_{false};
  uint16_t balancing_min_cell_voltage_mv_{0};
  bool has_balancing_start_delta_{false};
  uint16_t balancing_start_delta_mv_{0};
  bool has_balancing_stop_delta_{false};
  uint16_t balancing_stop_delta_mv_{0};
  bool has_balancing_idle_current_{false};
  float balancing_idle_current_a_{0.0f};
  bool has_balancing_min_temperature_{false};
  int8_t balancing_min_temperature_c_{0};
  bool has_balancing_max_temperature_{false};
  int8_t balancing_max_temperature_c_{0};
  bool has_max_balanced_cells_{false};
  uint8_t max_balanced_cells_{1};

  // REG2 support retained alongside the existing REG0/REG1 definitions.
  bool has_reg2_enabled_config_{false};
  bool reg2_enabled_{false};
  bool has_reg2_voltage_config_{false};
  uint8_t reg2_voltage_code_{0};

  // Alert pin routing and additional FET policy.
  bool has_safety_alert_mask_a_{false};
  uint8_t safety_alert_mask_a_{0};
  bool has_safety_alert_mask_b_{false};
  uint8_t safety_alert_mask_b_{0};
  bool has_alert_default_mask_{false};
  uint16_t alert_default_mask_{0};
  bool has_body_diode_threshold_{false};
  int16_t body_diode_threshold_ma_{0};
};

}  // namespace bq76952
}  // namespace esphome
