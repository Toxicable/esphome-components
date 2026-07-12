#pragma once

#include <cstdint>

namespace esphome {
namespace bq76952 {

enum class BQ76952CurrentGainPolicy : uint8_t {
  PRESERVE_EXISTING = 0,
  DERIVE_FROM_SHUNT = 1,
};

enum class BQ76952ThermistorMode : uint8_t {
  DISABLED = 0,
  PULLUP_18K = 1,
  PULLUP_180K = 2,
};

struct BQ76952RegulatorConfig {
  bool reg0_enabled{false};
  bool reg1_enabled{false};
  uint8_t reg1_voltage_code{0};
  bool reg2_enabled{false};
  uint8_t reg2_voltage_code{0};
};

struct BQ76952ThermistorConfig {
  BQ76952ThermistorMode ts1{BQ76952ThermistorMode::DISABLED};
  BQ76952ThermistorMode ts2{BQ76952ThermistorMode::DISABLED};
  BQ76952ThermistorMode ts3{BQ76952ThermistorMode::DISABLED};
};

// PCHG is used when charging a deeply depleted pack. The BQ76952 starts on
// the precharge path below start_cell_voltage_mv and hands over to CHG above
// stop_cell_voltage_mv.
struct BQ76952PrechargeConfig {
  bool enabled{false};
  uint16_t start_cell_voltage_mv{0};
  uint16_t stop_cell_voltage_mv{0};
};

// PDSG limits inrush into a discharged load/DC link before the main DSG FET is
// enabled. It terminates on timeout or once PACK/LD is within stop_delta_mv of
// top-of-stack.
struct BQ76952PredischargeConfig {
  bool enabled{false};
  uint16_t timeout_ms{0};
  uint16_t stop_delta_mv{0};
};

struct BQ76952FetConfig {
  bool autonomous_enabled{false};
  bool sleep_charge_enabled{false};
  int16_t body_diode_threshold_ma{0};
  BQ76952PrechargeConfig precharge{};
  BQ76952PredischargeConfig predischarge{};
};

struct BQ76952BalancingConfig {
  bool charging_enabled{false};
  bool relaxed_enabled{false};
  uint16_t minimum_cell_voltage_mv{0};
  uint16_t start_delta_mv{0};
  uint16_t stop_delta_mv{0};
  float idle_current_threshold_a{0.0f};
  int8_t minimum_temperature_c{0};
  int8_t maximum_temperature_c{0};
  uint8_t maximum_balanced_cells{0};
};

struct BQ76952VoltageProtectionConfig {
  bool enabled{false};
  uint16_t threshold_mv{0};
  uint16_t delay_ms{0};
  uint16_t recovery_hysteresis_mv{0};
};

struct BQ76952CurrentProtectionConfig {
  bool enabled{false};
  float threshold_a{0.0f};
  uint16_t delay_ms{0};
};

struct BQ76952SlowCurrentProtectionConfig {
  bool enabled{false};
  float threshold_a{0.0f};
  uint8_t delay_s{0};
};

struct BQ76952ShortCircuitProtectionConfig {
  bool enabled{false};
  uint16_t threshold_mv{0};
  uint16_t delay_us{0};
  uint8_t recovery_time_s{0};
};

struct BQ76952TemperatureProtectionConfig {
  bool charge_enabled{false};
  bool discharge_enabled{false};
  int8_t charge_minimum_c{0};
  int8_t charge_maximum_c{0};
  int8_t discharge_minimum_c{0};
  int8_t discharge_maximum_c{0};
  uint8_t recovery_hysteresis_c{0};
};

struct BQ76952ProtectionConfig {
  BQ76952VoltageProtectionConfig cell_undervoltage{};
  BQ76952VoltageProtectionConfig cell_overvoltage{};
  BQ76952CurrentProtectionConfig charge_overcurrent{};
  BQ76952CurrentProtectionConfig discharge_overcurrent_1{};
  BQ76952CurrentProtectionConfig discharge_overcurrent_2{};
  BQ76952SlowCurrentProtectionConfig discharge_overcurrent_3{};
  BQ76952ShortCircuitProtectionConfig discharge_short_circuit{};
  BQ76952TemperatureProtectionConfig temperature{};
  uint8_t current_recovery_time_s{0};
};

// Complete desired state for one BQ76952. There are deliberately no optional
// fields and no implicit "preserve whatever the chip already contains" mode.
// ESPHome must construct every group explicitly, including disabled features.
struct BQ76952Config {
  uint8_t cell_count{16};
  float sense_resistor_milliohm{1.0f};
  bool i2c_crc_enabled{false};
  BQ76952CurrentGainPolicy current_gain_policy{BQ76952CurrentGainPolicy::PRESERVE_EXISTING};
  BQ76952RegulatorConfig regulators{};
  BQ76952ThermistorConfig thermistors{};
  BQ76952FetConfig fet{};
  BQ76952BalancingConfig balancing{};
  BQ76952ProtectionConfig protections{};
};

}  // namespace bq76952
}  // namespace esphome
