#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include "bq76952_config.h"
#include "bq76952_service.h"
#include "bq76952_soc.h"

namespace esphome {
namespace bq76952 {

class BQ76952Component : public PollingComponent,
                           public i2c::I2CDevice,
                           protected BQ76952Config,
                           protected BQ76952ServiceState,
                           protected BQ76952SocState {
 public:
  void set_config(const BQ76952Config& config) {
    static_cast<BQ76952Config&>(*this) = config;

    // All supported boards wire an S-cell pack as VC1..VC(S-1), VC16.
    // The array remains as an internal compatibility detail until the protocol
    // implementation is moved out of bq76952.cpp.
    for (uint8_t index = 0; index < cell_read_map_.size(); ++index) {
      cell_read_map_[index] = index;
    }
    for (uint8_t index = 0; index < cell_count_; ++index) {
      cell_read_map_[index] = index == (cell_count_ - 1) ? 15 : index;
    }
    explicit_cell_map_ = true;
  }

  void set_bat_voltage_sensor(sensor::Sensor* sensor) { bat_voltage_sensor_ = sensor; }
  void set_pack_voltage_sensor(sensor::Sensor* sensor) { pack_voltage_sensor_ = sensor; }
  void set_ld_voltage_sensor(sensor::Sensor* sensor) { ld_voltage_sensor_ = sensor; }
  void set_largest_intercell_voltage_sensor(sensor::Sensor* sensor) { largest_intercell_voltage_sensor_ = sensor; }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor);
  void set_current_sensor(sensor::Sensor* sensor) { current_sensor_ = sensor; }
  void set_state_of_charge_sensor(sensor::Sensor* sensor) { state_of_charge_sensor_ = sensor; }
  void set_die_temperature_sensor(sensor::Sensor* sensor) { die_temperature_sensor_ = sensor; }
  void set_ts1_temperature_sensor(sensor::Sensor* sensor) { ts1_temperature_sensor_ = sensor; }
  void set_ts2_temperature_sensor(sensor::Sensor* sensor) { ts2_temperature_sensor_ = sensor; }
  void set_ts3_temperature_sensor(sensor::Sensor* sensor) { ts3_temperature_sensor_ = sensor; }

  void set_bms_state_sensor(text_sensor::TextSensor* sensor) { bms_state_sensor_ = sensor; }
  void set_fault_sensor(text_sensor::TextSensor* sensor) { fault_sensor_ = sensor; }
  void set_fet_status_flags_sensor(text_sensor::TextSensor* sensor) { fet_status_flags_sensor_ = sensor; }

  void set_output_enabled_switch(switch_::Switch* sw) { output_enabled_switch_ = sw; }
  void set_autonomous_fet_switch(switch_::Switch* sw) { autonomous_fet_switch_ = sw; }

  void setup() override;
  void update() override;
  void dump_config() override;

  bool set_output_enabled(bool enabled);
  bool set_autonomous_fet_control(bool enabled);
  bool set_sleep_allowed(bool allowed);
  bool clear_alarm_latches();
  bool reset_passed_charge_counter();
  bool apply_requested_configuration();
  bool program_factory_otp_defaults();

 protected:
  bool read_byte_(uint8_t reg, uint8_t& value);
  bool read_bytes_(uint8_t reg, uint8_t* data, size_t len);
  bool write_byte_(uint8_t reg, uint8_t value);
  bool write_bytes_(uint8_t reg, const uint8_t* data, size_t len);

  bool read_u16_(uint8_t reg, uint16_t& value);
  bool read_i16_(uint8_t reg, int16_t& value);
  bool write_u16_(uint8_t reg, uint16_t value);

  bool write_subcommand_(uint16_t subcommand);
  bool wait_subcommand_ready_(uint16_t subcommand, uint32_t timeout_ms = 20);
  bool read_subcommand_(uint16_t subcommand, uint8_t* data, size_t len);
  bool read_subcommand_u16_(uint16_t subcommand, uint16_t& value);
  bool write_subcommand_data_(uint16_t subcommand, const uint8_t* data, size_t len);
  bool read_data_memory_u8_(uint16_t address, uint8_t& value);
  bool read_data_memory_u16_(uint16_t address, uint16_t& value);
  bool write_data_memory_u8_(uint16_t address, uint8_t value);
  bool write_data_memory_u16_(uint16_t address, uint16_t value);
  bool set_cfgupdate_mode_(bool enabled);

  bool has_current_limit_config_() const;
  bool has_regulator_config_() const;
  bool has_ts_pin_config_() const;
  bool has_predischarge_config_() const;
  bool has_autonomous_balancing_config_() const;
  bool has_boot_mode_config_() const;
  bool has_requested_configuration_() const;
  bool apply_boot_modes_();
  bool apply_requested_configuration_(bool force_live_reapply);
  void request_configuration_reconciliation_(const char* reason, uint32_t delay_ms, bool force_live_reapply);
  bool run_configuration_reconciliation_(uint32_t now);
  void note_communication_failure_();
  bool apply_boot_mode_startup_defaults_();
  bool apply_regulator_config_(bool force_live_reapply);
  bool load_unit_scaling_();
  bool apply_ts_pin_config_();
  bool apply_predischarge_config_();
  bool apply_autonomous_balancing_config_();
  bool apply_current_limit_config_();

  void maybe_log_event_(uint16_t control_status, uint16_t battery_status, uint8_t fet_status, uint16_t alarm_status,
                        bool have_alarm_status, uint8_t safety_status_a, uint8_t safety_status_b,
                        uint8_t safety_status_c, bool have_safety_status);
  uint8_t encode_current_threshold_code_(float current_a, uint8_t min_code, uint8_t max_code, const char* label);
  uint8_t encode_current_delay_code_(uint16_t delay_ms, const char* label);
  uint8_t encode_cell_voltage_threshold_code_(uint16_t threshold_mv, uint8_t min_code, uint8_t max_code,
                                               const char* label);
  uint16_t encode_voltage_delay_code_(uint16_t delay_ms, uint16_t min_code, uint16_t max_code, const char* label);
  bool precheck_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& needs_write);
  bool precheck_data_memory_value_(uint16_t address, uint8_t desired_value, const char* label, bool& needs_write);
  bool precheck_data_memory_value_u16_(uint16_t address, uint16_t desired_value, const char* label, bool& needs_write);
  bool ensure_data_memory_mask_(uint16_t address, uint8_t required_bits, const char* label, bool& ok);
  bool write_data_memory_value_if_needed_(uint16_t address, uint8_t desired_value, const char* label, bool& ok);
  bool write_data_memory_value_u16_if_needed_(uint16_t address, uint16_t desired_value, const char* label, bool& ok);

  const char* bms_state_to_string_(uint16_t battery_status, uint16_t control_status) const;
  const char* power_path_to_string_(uint8_t fet_status) const;
  std::string fault_to_string_(uint16_t battery_status, uint8_t status_a, uint8_t status_b, uint8_t status_c) const;
  std::string alarm_flags_to_string_(uint16_t alarm_status) const;
  std::string safety_status_flags_to_string_(uint8_t status_a, uint8_t status_b, uint8_t status_c) const;
  std::string fet_status_flags_to_string_(uint8_t fet_status) const;
  void append_flag_(std::string& flags, const char* flag) const;

  float estimate_soc_from_voltage_(int16_t cell_mv) const;
  float update_soc_(float current_a, float raw_passq_ah, int16_t min_cell_mv, int16_t max_cell_mv,
                    int16_t avg_cell_mv, uint8_t safety_status_a);
  void load_soc_state_();
  void save_soc_state_(bool force);
  void mark_soc_full_();
  void mark_soc_empty_();

  std::array<uint8_t, 16> cell_read_map_{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  bool explicit_cell_map_{true};

  sensor::Sensor* bat_voltage_sensor_{nullptr};
  sensor::Sensor* pack_voltage_sensor_{nullptr};
  sensor::Sensor* ld_voltage_sensor_{nullptr};
  sensor::Sensor* largest_intercell_voltage_sensor_{nullptr};
  std::array<sensor::Sensor*, 16> cell_voltage_sensors_{};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* state_of_charge_sensor_{nullptr};
  sensor::Sensor* die_temperature_sensor_{nullptr};
  sensor::Sensor* ts1_temperature_sensor_{nullptr};
  sensor::Sensor* ts2_temperature_sensor_{nullptr};
  sensor::Sensor* ts3_temperature_sensor_{nullptr};

  text_sensor::TextSensor* bms_state_sensor_{nullptr};
  text_sensor::TextSensor* fault_sensor_{nullptr};
  text_sensor::TextSensor* fet_status_flags_sensor_{nullptr};

  switch_::Switch* output_enabled_switch_{nullptr};
  switch_::Switch* autonomous_fet_switch_{nullptr};
};

}  // namespace bq76952
}  // namespace esphome

#include "bq76952_controls.h"
