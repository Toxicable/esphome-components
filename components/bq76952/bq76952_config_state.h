#pragma once

#include <cstdint>

#include "bq76952_config.h"

namespace esphome {
namespace bq76952 {

// Flattened state consumed by the current implementation in bq76952.cpp.
//
// This is deliberately internal. The public configuration surface uses
// std::optional values and complete protection groups; this adapter exists only
// until the implementation is moved onto those types directly.
class BQ76952ConfigState {
 protected:
  static constexpr uint8_t BOOT_PRESERVE = 0;
  static constexpr uint8_t BOOT_ENABLE = 1;
  static constexpr uint8_t BOOT_DISABLE = 2;

  void load_config_(const BQ76952Config& config) {
    cell_count_ = config.cell_count >= 3 && config.cell_count <= 16 ? config.cell_count : 16;
    sense_resistor_milliohm_ = config.sense_resistor_milliohm;

    autonomous_fet_mode_ = BOOT_PRESERVE;
    if (config.autonomous_fet_enabled.has_value()) {
      autonomous_fet_mode_ = *config.autonomous_fet_enabled ? BOOT_ENABLE : BOOT_DISABLE;
    }

    has_reg0_config_ = config.reg0_enabled.has_value();
    reg0_enabled_ = config.reg0_enabled.value_or(false);

    has_reg1_enabled_config_ =
        config.reg1_enabled.has_value() || config.reg1_voltage_code.has_value();
    reg1_enabled_ =
        config.reg1_enabled.value_or(config.reg1_voltage_code.has_value());
    has_reg1_voltage_config_ = config.reg1_voltage_code.has_value();
    reg1_voltage_code_ = config.reg1_voltage_code.value_or(0);

    has_ts1_config_ = config.ts1_pullup_180k.has_value();
    ts1_pullup_180k_ = config.ts1_pullup_180k.value_or(false);
    has_ts2_config_ = config.ts2_pullup_180k.has_value();
    ts2_pullup_180k_ = config.ts2_pullup_180k.value_or(false);
    has_ts3_config_ = config.ts3_pullup_180k.has_value();
    ts3_pullup_180k_ = config.ts3_pullup_180k.value_or(false);

    has_predischarge_setting_ = config.predischarge_enabled.has_value();
    predischarge_enabled_ = config.predischarge_enabled.value_or(false);
    has_sleep_charge_setting_ = config.sleep_charge_enabled.has_value();
    sleep_charge_enabled_ = config.sleep_charge_enabled.value_or(false);
    has_autonomous_balancing_setting_ = config.autonomous_balancing_enabled.has_value();
    autonomous_balancing_enabled_ = config.autonomous_balancing_enabled.value_or(false);

    has_cell_undervoltage_limit_ = config.cell_undervoltage.has_value();
    has_cell_undervoltage_delay_ = config.cell_undervoltage.has_value();
    if (config.cell_undervoltage.has_value()) {
      cell_undervoltage_limit_mv_ = config.cell_undervoltage->limit_mv;
      cell_undervoltage_delay_ms_ = config.cell_undervoltage->delay_ms;
    }

    has_cell_overvoltage_limit_ = config.cell_overvoltage.has_value();
    has_cell_overvoltage_delay_ = config.cell_overvoltage.has_value();
    if (config.cell_overvoltage.has_value()) {
      cell_overvoltage_limit_mv_ = config.cell_overvoltage->limit_mv;
      cell_overvoltage_delay_ms_ = config.cell_overvoltage->delay_ms;
    }

    has_charge_current_limit_ = config.charge_overcurrent.has_value();
    has_charge_current_delay_ = config.charge_overcurrent.has_value();
    if (config.charge_overcurrent.has_value()) {
      charge_current_limit_a_ = config.charge_overcurrent->limit_a;
      charge_current_delay_ms_ = config.charge_overcurrent->delay_ms;
    }

    has_discharge_current_limit_ = config.discharge_overcurrent_1.has_value();
    has_discharge_current_delay_ = config.discharge_overcurrent_1.has_value();
    if (config.discharge_overcurrent_1.has_value()) {
      discharge_current_limit_a_ = config.discharge_overcurrent_1->limit_a;
      discharge_current_delay_ms_ = config.discharge_overcurrent_1->delay_ms;
    }

    has_discharge_current_limit_2_ = config.discharge_overcurrent_2.has_value();
    has_discharge_current_delay_2_ = config.discharge_overcurrent_2.has_value();
    if (config.discharge_overcurrent_2.has_value()) {
      discharge_current_limit_2_a_ = config.discharge_overcurrent_2->limit_a;
      discharge_current_delay_2_ms_ = config.discharge_overcurrent_2->delay_ms;
    }

    has_discharge_current_limit_3_ = config.discharge_overcurrent_3.has_value();
    has_discharge_current_delay_3_ = config.discharge_overcurrent_3.has_value();
    if (config.discharge_overcurrent_3.has_value()) {
      discharge_current_limit_3_a_ = config.discharge_overcurrent_3->limit_a;
      discharge_current_delay_3_s_ = config.discharge_overcurrent_3->delay_s;
    }

    has_scd_threshold_ = config.discharge_short_circuit.has_value();
    has_scd_delay_ = config.discharge_short_circuit.has_value();
    has_scd_recovery_time_ = config.discharge_short_circuit.has_value();
    if (config.discharge_short_circuit.has_value()) {
      scd_threshold_mv_ = config.discharge_short_circuit->threshold_mv;
      scd_delay_us_ = config.discharge_short_circuit->delay_us;
      scd_recovery_time_s_ = config.discharge_short_circuit->recovery_time_s;
    }

    has_current_recovery_time_ = config.current_recovery_time_s.has_value();
    current_recovery_time_s_ = config.current_recovery_time_s.value_or(0);
  }

  // Configuration is applied on the first successful communication probe.
  static constexpr uint32_t boot_config_apply_delay_ms_{0};

  // State changes are always logged by the implementation. There is no
  // user-facing event-log mode or debug-burst mode.
  static constexpr bool event_logging_{true};
  static constexpr bool xchg_debug_burst_{false};

  uint8_t cell_count_{16};
  float sense_resistor_milliohm_{1.0f};

  uint8_t autonomous_fet_mode_{BOOT_PRESERVE};
  uint8_t sleep_mode_{BOOT_ENABLE};

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

  bool has_predischarge_setting_{false};
  bool predischarge_enabled_{false};
  bool has_sleep_charge_setting_{false};
  bool sleep_charge_enabled_{false};
  bool has_autonomous_balancing_setting_{false};
  bool autonomous_balancing_enabled_{false};

  // Retained only because the current SoC implementation still references
  // these legacy fields. Nominal capacity is no longer a public setting.
  static constexpr bool has_nominal_capacity_ah_{false};
  static constexpr float nominal_capacity_ah_{0.0f};

  bool has_cell_undervoltage_limit_{false};
  uint16_t cell_undervoltage_limit_mv_{0};
  bool has_cell_undervoltage_delay_{false};
  uint16_t cell_undervoltage_delay_ms_{0};
  bool has_cell_overvoltage_limit_{false};
  uint16_t cell_overvoltage_limit_mv_{0};
  bool has_cell_overvoltage_delay_{false};
  uint16_t cell_overvoltage_delay_ms_{0};

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
};

}  // namespace bq76952
}  // namespace esphome
