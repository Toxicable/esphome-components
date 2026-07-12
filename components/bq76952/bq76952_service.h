#pragma once

#include <array>
#include <cstdint>

#include "bq76952_config.h"
#include "bq76952_protocol.h"

namespace esphome {
namespace bq76952 {

struct BQ76952Snapshot {
  bool online{false};
  uint16_t control_status{0};
  uint16_t battery_status{0};
  uint16_t alarm_status{0};
  uint8_t safety_status_a{0};
  uint8_t safety_status_b{0};
  uint8_t safety_status_c{0};
  uint8_t fet_status{0};

  std::array<int16_t, 16> cell_voltage_mv{};
  uint8_t cell_count{0};
  int16_t stack_voltage_mv{0};
  int16_t pack_voltage_mv{0};
  int16_t load_detect_voltage_mv{0};
  float current_a{0.0f};
  float passed_charge_ah{0.0f};
  float passed_charge_time_s{0.0f};
  float die_temperature_c{0.0f};
  std::array<float, 3> thermistor_temperature_c{};
};

// Product-level BMS behaviour. The service owns desired configuration,
// connection/recovery state, configuration reconciliation, protection setup,
// measurement conversion, and intentional runtime actions. It does not know
// about ESPHome entities.
class BQ76952Service {
 public:
  explicit BQ76952Service(BQ76952Protocol &protocol);

  void set_config(const BQ76952Config &config);
  const BQ76952Config &config() const;

  bool poll(BQ76952Snapshot &snapshot);
  bool reconcile_configuration(bool force_live_state = false);

  bool set_output_enabled(bool enabled);
  bool set_autonomous_fet_enabled(bool enabled);
  bool clear_alarm_latches();
  bool reset_passed_charge();
  bool program_factory_otp();

 private:
  bool establish_connection();
  void note_communication_failure();

  bool apply_regulators(bool force_live_state);
  bool apply_thermistors();
  bool apply_fet_configuration();
  bool apply_balancing();
  bool apply_protections();
  bool apply_current_calibration();

  bool read_snapshot(BQ76952Snapshot &snapshot);
  uint16_t cell_mode_mask() const;
  uint8_t raw_cell_channel(uint8_t logical_cell) const;

  BQ76952Protocol &protocol_;
  BQ76952Config config_{};
  bool config_set_{false};
  bool online_{false};
  bool configured_{false};
  uint32_t next_configuration_audit_ms_{0};
};

}  // namespace bq76952
}  // namespace esphome
