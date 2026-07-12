#pragma once

#include <array>
#include <cstdint>

#include "bq76952_config.h"
#include "bq76952_protocol.h"
#include "bq76952_soc.h"

namespace esphome {
namespace bq76952 {

enum class BQ76952OperatingState : uint8_t {
  OFFLINE = 0,
  NORMAL,
  SLEEP,
  DEEP_SLEEP,
  CONFIG_UPDATE,
  SHUTDOWN_PENDING,
};

// Normalized faults decoded from the device's raw Safety Status A/B/C,
// Battery Status, and permanent-failure registers. The three TI register banks
// remain an internal protocol detail rather than leaking through the snapshot.
enum BQ76952FaultFlags : uint32_t {
  BQ76952_FAULT_NONE = 0,
  BQ76952_FAULT_CELL_UNDERVOLTAGE = 1U << 0,
  BQ76952_FAULT_CELL_OVERVOLTAGE = 1U << 1,
  BQ76952_FAULT_CHARGE_OVERCURRENT = 1U << 2,
  BQ76952_FAULT_DISCHARGE_OVERCURRENT_1 = 1U << 3,
  BQ76952_FAULT_DISCHARGE_OVERCURRENT_2 = 1U << 4,
  BQ76952_FAULT_DISCHARGE_OVERCURRENT_3 = 1U << 5,
  BQ76952_FAULT_DISCHARGE_SHORT_CIRCUIT = 1U << 6,
  BQ76952_FAULT_TEMPERATURE = 1U << 7,
  BQ76952_FAULT_PERMANENT_FAILURE = 1U << 8,
};

// Internal service-to-ESPHome snapshot. Raw protocol registers and the device
// coulomb-counter value are consumed inside the service and are not exposed.
struct BQ76952Snapshot {
  bool online{false};
  BQ76952OperatingState state{BQ76952OperatingState::OFFLINE};
  uint32_t fault_flags{BQ76952_FAULT_NONE};
  uint8_t fet_status_flags{0};

  std::array<int16_t, 16> cell_voltage_mv{};
  uint8_t cell_count{0};
  int16_t stack_voltage_mv{0};
  int16_t pack_voltage_mv{0};
  int16_t load_detect_voltage_mv{0};
  float current_a{0.0f};
  float state_of_charge_percent{0.0f};
  float die_temperature_c{0.0f};
  std::array<float, 3> thermistor_temperature_c{};
};

// Product-level BMS behaviour. The service owns desired configuration,
// connection recovery, configuration synchronization, measurement conversion,
// protection policy, runtime actions, and the ancillary SoC estimator. It does
// not know about ESPHome entities.
class BQ76952Service {
 public:
  explicit BQ76952Service(BQ76952Protocol &protocol);

  void setup();
  void set_config(const BQ76952Config &config);
  const BQ76952Config &config() const;

  bool poll(BQ76952Snapshot &snapshot);

  bool set_output_enabled(bool enabled);
  bool set_autonomous(bool enabled);
  bool clear_alarm_latches();
  bool program_factory_otp();

 private:
  // Normal audits verify data-memory settings and repair drift. Reconnect
  // restoration additionally reapplies runtime-only commands even when the
  // corresponding data-memory bytes already match.
  enum class ConfigurationSyncMode : uint8_t {
    AUDIT_AND_REPAIR = 0,
    RESTORE_RUNTIME_STATE,
  };

  bool establish_connection();
  void note_communication_failure();
  bool synchronize_configuration(ConfigurationSyncMode mode);

  bool apply_regulators(ConfigurationSyncMode mode);
  bool apply_thermistors();
  bool apply_fet_configuration(ConfigurationSyncMode mode);
  bool apply_balancing();
  bool apply_protections();
  bool apply_current_calibration();

  bool read_snapshot(BQ76952Snapshot &snapshot);
  bool read_fault_flags(uint32_t &fault_flags);
  bool read_coulomb_counter(float &charge_ah, float &integration_time_s);
  uint16_t cell_mode_mask() const;
  uint8_t raw_cell_channel(uint8_t logical_cell) const;

  BQ76952Protocol &protocol_;
  BQ76952Soc soc_;
  BQ76952Config config_{};
  bool config_set_{false};
  bool online_{false};
  bool configured_{false};
  uint32_t next_configuration_audit_ms_{0};
};

}  // namespace bq76952
}  // namespace esphome
