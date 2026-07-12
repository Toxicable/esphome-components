#pragma once

#include <cstdint>

namespace esphome {
namespace bq76952 {

// Runtime state for high-level BMS orchestration: configuration
// reconciliation, communication health, diagnostics, and output requests.
class BQ76952ServiceState {
 protected:
  // Auto-detected from Settings:Configuration:DA Configuration (0x9303).
  int32_t current_lsb_ua_{1000};
  bool user_volts_cv_{true};

  bool cell_map_initialized_{false};
  bool configuration_pending_{false};
  bool configuration_force_live_reapply_{false};
  bool live_boot_modes_pending_{false};
  bool communication_seen_{false};
  bool communication_online_{false};
  const char* configuration_reason_{"none"};
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
  bool output_request_pending_{false};
  bool output_request_expected_enabled_{false};
  uint32_t output_request_started_ms_{0};
  uint32_t last_cell16_diagnostic_ms_{0};
  uint32_t configuration_due_ms_{0};
  uint32_t configuration_retry_log_ms_{0};
  uint32_t next_configuration_audit_ms_{0};
};

}  // namespace bq76952
}  // namespace esphome
