#pragma once

#include <cstdint>
#include <optional>

namespace esphome {
namespace bq76952 {

struct BQ76952VoltageProtectionConfig {
  uint16_t limit_mv{0};
  uint16_t delay_ms{0};
};

struct BQ76952CurrentProtectionConfig {
  float limit_a{0.0f};
  uint16_t delay_ms{0};
};

struct BQ76952SlowCurrentProtectionConfig {
  float limit_a{0.0f};
  uint8_t delay_s{0};
};

struct BQ76952ShortCircuitProtectionConfig {
  uint16_t threshold_mv{0};
  uint16_t delay_us{0};
  uint8_t recovery_time_s{0};
};

// Desired configuration for one BQ76952.
//
// Optional values mean "leave the existing device setting unchanged". Related
// protection values are grouped so the driver never receives half of a
// threshold/delay configuration.
struct BQ76952Config {
  uint8_t cell_count{16};
  float sense_resistor_milliohm{1.0f};

  // nullopt preserves the current autonomous-FET startup policy.
  std::optional<bool> autonomous_fet_enabled{};

  std::optional<bool> reg0_enabled{};
  std::optional<bool> reg1_enabled{};
  std::optional<uint8_t> reg1_voltage_code{};

  // A configured temperature entity supplies the matching thermistor setting.
  std::optional<bool> ts1_pullup_180k{};
  std::optional<bool> ts2_pullup_180k{};
  std::optional<bool> ts3_pullup_180k{};

  std::optional<bool> predischarge_enabled{};
  std::optional<bool> sleep_charge_enabled{};
  std::optional<bool> autonomous_balancing_enabled{};

  std::optional<BQ76952VoltageProtectionConfig> cell_undervoltage{};
  std::optional<BQ76952VoltageProtectionConfig> cell_overvoltage{};
  std::optional<BQ76952CurrentProtectionConfig> charge_overcurrent{};
  std::optional<BQ76952CurrentProtectionConfig> discharge_overcurrent_1{};
  std::optional<BQ76952CurrentProtectionConfig> discharge_overcurrent_2{};
  std::optional<BQ76952SlowCurrentProtectionConfig> discharge_overcurrent_3{};
  std::optional<BQ76952ShortCircuitProtectionConfig> discharge_short_circuit{};
  std::optional<uint8_t> current_recovery_time_s{};
};

}  // namespace bq76952
}  // namespace esphome
