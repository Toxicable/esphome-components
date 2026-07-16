#pragma once

#include <cstddef>
#include <cstdint>

#include "bq25756_bus.h"
#include "bq25756_protocol.h"
#include "bq25756_register_manifest.h"

namespace bq25756_core {

enum class AdcEnsureResult : uint8_t {
  OK,
  REPAIRED,
  IO_ERROR,
  VERIFICATION_MISMATCH,
};

enum class MeasurementReadResult : uint8_t {
  OK,
  CONFIGURATION_CHANGED,
  IO_ERROR,
  CONFIGURATION_VERIFY_MISMATCH,
};

struct AdcConfigurationState {
  uint8_t old_reg2b{0};
  uint8_t persistent_reg2b{0};
  uint8_t transient_reg2b{0};
  uint8_t old_reg2c{0};
  uint8_t requested_reg2c{0};
};

struct ConfigurationReconcileResult {
  bool io_ok{true};
  bool matches{true};
  bool repaired{false};
  size_t mismatch_count{0};
  size_t repaired_count{0};
  size_t remaining_mismatch_count{0};
  uint16_t first_mismatch_address{0};
  uint32_t desired_fingerprint{0};
  uint32_t observed_fingerprint{0};
};

class Bq25756Service {
 public:
  explicit Bq25756Service(RegisterBus *bus) : bus_(bus) {}

  bool read_byte(uint8_t reg, uint8_t &value);
  bool read_bytes(uint8_t reg, uint8_t *data, size_t len);
  bool write_byte(uint8_t reg, uint8_t value);
  bool write_bytes(uint8_t reg, const uint8_t *data, size_t len);
  bool write_u16_le(uint8_t reg, uint16_t value);
  bool read_u16_le(uint8_t reg, Reg16Value &value);
  bool update_register_bits(uint8_t reg, uint8_t mask, uint8_t value_bits);
  bool probe(uint8_t &part_info);
  bool set_charge_enabled(bool enabled);
  bool set_hiz_mode(bool enabled);
  bool set_pfm_enabled(bool enabled);
  bool set_reverse_mode(bool enabled);
  bool set_watchdog_code(uint8_t code);
  bool reset_watchdog();
  bool read_status(Status &status);
  MeasurementReadResult read_measurements(Measurements &measurements, bool include_vfb,
                                           uint8_t requested_adc_config, AdcConfigurationState &adc_state);
  bool read_control_states(ControlStates &states);
  AdcEnsureResult ensure_adc_enabled(bool include_vfb, uint8_t requested_adc_config,
                                      AdcConfigurationState &adc_state);
  bool apply_limits(bool has_charge_voltage_limit_mv, uint16_t charge_voltage_limit_mv,
                    bool has_charge_current_limit_ma, uint16_t charge_current_limit_ma,
                    bool has_input_current_dpm_limit_ma, uint16_t input_current_dpm_limit_ma,
                    bool has_input_voltage_dpm_limit_mv, uint16_t input_voltage_dpm_limit_mv);
  bool apply_pin_overrides(bool disable_ce_pin, bool disable_ilim_hiz_pin, bool disable_ichg_pin);
  bool read_charge_precheck(ChargePrecheckSnapshot &snapshot);
  bool reconcile_configuration(const Bq25756ConfigurationImage &image, bool repair,
                               ConfigurationReconcileResult &result);

 private:
  bool read_register_value_(const component_common::RegisterImageEntry &entry, uint32_t &value);
  bool write_register_value_(const component_common::RegisterImageEntry &entry, uint32_t value);

  RegisterBus *bus_{nullptr};
};

}  // namespace bq25756_core
