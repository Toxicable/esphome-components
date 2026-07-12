#pragma once

#include <cstdint>

namespace esphome {
namespace bq76952 {

// The SoC fallback curve is chemistry-specific. Lithium-ion is the only
// supported curve for now, so users must acknowledge it explicitly.
enum class BQ76952CellChemistry : uint8_t {
  LITHIUM_ION = 0,
};

// Keep the device's existing calibrated current gain, or calculate a new gain
// from sense_resistor_milliohm when configuration is applied.
enum class BQ76952CurrentGainPolicy : uint8_t {
  FACTORY_CALIBRATION = 0,
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

// PCHG is a reduced-current charging path for a deeply depleted pack. The
// service selects PCHG below the start threshold and hands over to CHG above
// the stop threshold.
struct BQ76952PrechargeConfig {
  bool enabled{false};
  uint16_t start_cell_voltage_mv{0};
  uint16_t stop_cell_voltage_mv{0};
};

// PDSG limits inrush into load-side capacitance before the main DSG FET turns
// on. It terminates on timeout or once the remaining load-side voltage delta is
// below stop_delta_mv.
struct BQ76952PredischargeConfig {
  bool enabled{false};
  uint16_t timeout_ms{0};
  uint16_t stop_delta_mv{0};
};

struct BQ76952FetConfig {
  // Autonomous means the BQ76952 controls CHG/DSG from its protection state.
  bool autonomous{false};
  bool sleep_charge_enabled{false};

  // Current threshold used for body-diode protection/recovery decisions.
  // The sign follows the BQ76952 register convention.
  int16_t body_diode_threshold_ma{0};

  BQ76952PrechargeConfig precharge{};
  BQ76952PredischargeConfig predischarge{};
};

struct BQ76952BalancingConfig {
  bool charging_enabled{false};

  // Allows balancing when charge current has fallen below
  // relaxed_current_threshold_a, rather than only while actively charging.
  bool relaxed_balancing_enabled{false};
  float relaxed_current_threshold_a{0.0f};

  uint16_t minimum_cell_voltage_mv{0};
  uint16_t start_delta_mv{0};
  uint16_t stop_delta_mv{0};
  int8_t minimum_temperature_c{0};
  int8_t maximum_temperature_c{0};
  uint8_t maximum_balanced_cells{0};
};

// CUV/COV are evaluated independently against every active cell, not against
// total pack voltage.
struct BQ76952CellVoltageProtectionConfig {
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

// OCD3 is the slower, long-duration discharge-overcurrent tier. Its delay is
// measured in seconds rather than milliseconds.
struct BQ76952LongDurationCurrentProtectionConfig {
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

// Charge and discharge temperature limits are separate because a pack can be
// safe to discharge at temperatures where charging would be unsafe.
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
  BQ76952CellVoltageProtectionConfig cell_undervoltage{};
  BQ76952CellVoltageProtectionConfig cell_overvoltage{};
  BQ76952CurrentProtectionConfig charge_overcurrent{};
  BQ76952CurrentProtectionConfig discharge_overcurrent_1{};
  BQ76952CurrentProtectionConfig discharge_overcurrent_2{};
  BQ76952LongDurationCurrentProtectionConfig discharge_overcurrent_3{};
  BQ76952ShortCircuitProtectionConfig discharge_short_circuit{};
  BQ76952TemperatureProtectionConfig temperature{};

  // Delay before current protections may recover after the fault clears.
  uint8_t current_recovery_time_s{0};
};

// Complete desired state for one BQ76952. There are deliberately no optional
// fields and no implicit "preserve whatever the chip already contains" mode.
// ESPHome must construct every group explicitly, including disabled features.
struct BQ76952Config {
  uint8_t cell_count{16};
  BQ76952CellChemistry cell_chemistry{BQ76952CellChemistry::LITHIUM_ION};
  float sense_resistor_milliohm{1.0f};
  bool i2c_crc_enabled{false};
  BQ76952CurrentGainPolicy current_gain_policy{BQ76952CurrentGainPolicy::FACTORY_CALIBRATION};
  BQ76952RegulatorConfig regulators{};
  BQ76952ThermistorConfig thermistors{};
  BQ76952FetConfig fet{};
  BQ76952BalancingConfig balancing{};
  BQ76952ProtectionConfig protections{};
};

}  // namespace bq76952
}  // namespace esphome
