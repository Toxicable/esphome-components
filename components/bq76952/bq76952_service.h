#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bq76952_config.h"
#include "bq76952_i2c_transport.h"
#include "bq76952_status.h"
#include "bq76952_soc.h"

namespace esphome {
namespace bq76952 {

// Product-level BMS behaviour. The service owns desired configuration,
// connection recovery, configuration synchronization, measurement conversion,
// protection policy, runtime actions, and the ancillary SoC estimator. It does
// not know about ESPHome entities.
class BQ76952Service {
 public:
  explicit BQ76952Service(BQ76952I2CTransport &transport);

  void setup();
  void set_config(const BQ76952Config &config);
  const BQ76952Config &config() const;

  bool poll(::bq76952_core::Snapshot &snapshot);
  const char *capacity_calibration_status() const;

  bool set_output_enabled(bool enabled);
  bool clear_alarm_latches();
  bool program_factory_otp();

 private:
  enum class ConfigurationSyncMode : uint8_t {
    AUDIT_AND_REPAIR = 0,
    RESTORE_RUNTIME_STATE,
  };

  bool establish_connection();
  void note_communication_failure();
  bool synchronize_configuration(ConfigurationSyncMode mode);
  bool configuration_matches(bool &matches);
  bool write_configuration();
  bool restore_runtime_state();

  bool apply_regulators(bool write, bool &matches);
  bool apply_thermistors(bool write, bool &matches);
  bool apply_fet_configuration(bool write, bool &matches);
  bool apply_balancing(bool write, bool &matches);
  bool apply_protections(bool write, bool &matches);
  bool apply_current_calibration(bool write, bool &matches);

  bool sync_data(uint16_t address, const uint8_t *desired, const uint8_t *mask, size_t length, bool write,
                 bool &matches, const char *label);
  bool sync_u8(uint16_t address, uint8_t desired, uint8_t mask, bool write, bool &matches, const char *label);
  bool sync_u16(uint16_t address, uint16_t desired, uint16_t mask, bool write, bool &matches, const char *label);

  bool require_full_access();
  bool load_unit_scaling();
  bool read_snapshot(::bq76952_core::Snapshot &snapshot);
  bool read_coulomb_counter(float &charge_ah);
  uint16_t cell_mode_mask() const;
  uint8_t raw_cell_channel(uint8_t logical_cell) const;

  BQ76952I2CTransport &transport_;
  BQ76952Soc soc_;
  BQ76952Config config_{};
  bool config_set_{false};
  component_common::LifecycleState lifecycle_{component_common::LifecycleState::DISCONNECTED};
  bool online_{false};
  bool configured_{false};
  int32_t current_lsb_ua_{1000};
  bool direct_voltage_centivolts_{true};
  bool output_request_pending_{false};
  bool output_request_expected_enabled_{false};
  uint32_t output_request_started_ms_{0};
  uint32_t next_configuration_retry_ms_{0};
  uint32_t next_configuration_audit_ms_{0};
};

}  // namespace bq76952
}  // namespace esphome
