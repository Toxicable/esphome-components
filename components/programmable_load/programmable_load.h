#pragma once

#include <cstdint>
#include <vector>

#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include "../component_common/charger.h"

#include "load_types.h"
#include "procedure.h"

namespace esphome {
namespace programmable_load {

struct TemperatureInput {
  sensor::Sensor *sensor{nullptr};
  bool required{false};
};

class ProgrammableLoadComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Hardware boundary. The configured value may describe a lower-voltage board,
  // but can never raise the component's absolute 75 V ceiling.
  void set_dac_output(output::FloatOutput *output) { this->dac_output_ = output; }
  void set_hardware_maximum_voltage(float voltage_v) {
    this->hardware_limits_.maximum_voltage_v =
        normalize_hardware_maximum_voltage(voltage_v);
  }
  void set_fan_output(output::FloatOutput *output) { this->fan_output_ = output; }

  // Measurement inputs. Raw electrical values are calibrated before procedures
  // receive them or the core applies limits.
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void add_temperature_sensor(sensor::Sensor *sensor, bool required) {
    this->temperature_inputs_.push_back({sensor, required});
  }
  void set_sample_timeout_ms(uint32_t timeout_ms) {
    this->sample_timeout_ms_ = timeout_ms;
  }

  // Typed charger capability. Home Assistant entities remain optional
  // observers owned by the charger rather than an internal component API.
  void set_charger(::component_common::ChargerInterface *charger) {
    this->charger_ = charger;
  }
  void set_charger_sample_timeout_ms(uint32_t timeout_ms) {
    this->charger_sample_timeout_ms_ = timeout_ms;
  }
  void set_charger_control_timeout_ms(uint32_t timeout_ms) {
    this->charger_control_timeout_ms_ = timeout_ms;
  }

  // Calibration boundary. Configured coefficients are retained as the reset
  // defaults; a persisted calibration may replace the active copy at setup.
  void set_current_calibration(float scale, float offset_a) {
    this->configured_calibration_.current = {scale, offset_a};
    this->calibration_.current = {scale, offset_a};
  }
  void set_voltage_calibration(float scale, float offset_v) {
    this->configured_calibration_.voltage = {scale, offset_v};
    this->calibration_.voltage = {scale, offset_v};
  }
  void set_output_calibration(float zero_level, float full_scale_current_a) {
    this->configured_calibration_.output = {zero_level, full_scale_current_a};
    this->calibration_.output = {zero_level, full_scale_current_a};
  }
  void set_restore_calibration(bool restore) {
    this->restore_calibration_ = restore;
  }
  bool apply_calibration(const Calibration &calibration, bool persist,
                         CalibrationSource source = CalibrationSource::APPLIED);
  bool reset_calibration(bool persist);
  const Calibration &calibration() const { return this->calibration_; }
  CalibrationSource calibration_source() const {
    return this->calibration_source_;
  }
  void set_calibration_status_sensor(text_sensor::TextSensor *sensor) {
    this->calibration_status_sensor_ = sensor;
  }
  void set_current_scale_sensor(sensor::Sensor *sensor) {
    this->current_scale_sensor_ = sensor;
  }
  void set_current_offset_sensor(sensor::Sensor *sensor) {
    this->current_offset_sensor_ = sensor;
  }
  void set_voltage_scale_sensor(sensor::Sensor *sensor) {
    this->voltage_scale_sensor_ = sensor;
  }
  void set_voltage_offset_sensor(sensor::Sensor *sensor) {
    this->voltage_offset_sensor_ = sensor;
  }
  void set_output_zero_level_sensor(sensor::Sensor *sensor) {
    this->output_zero_level_sensor_ = sensor;
  }
  void set_output_full_scale_current_sensor(sensor::Sensor *sensor) {
    this->output_full_scale_current_sensor_ = sensor;
  }

  // Configurable operating limits. Runtime always checks both the operating
  // ceiling and the independent hardware ceiling.
  void set_maximum_current(float current_a) {
    this->limits_.maximum_current_a = current_a;
  }
  void set_minimum_voltage(float voltage_v) {
    this->limits_.minimum_voltage_v = voltage_v;
  }
  void set_maximum_voltage(float voltage_v) {
    this->limits_.maximum_voltage_v = voltage_v;
  }
  void set_maximum_power(float power_w) {
    this->limits_.maximum_power_w = power_w;
  }
  void set_maximum_temperature(float temperature_c) {
    this->limits_.maximum_temperature_c = temperature_c;
  }

  // Control policy.
  void set_control_period_ms(uint32_t period_ms) {
    this->control_period_ms_ = period_ms;
  }
  void set_deadband(float current_a) { this->deadband_a_ = current_a; }
  void set_rise_rate(float current_a_per_s) {
    this->rise_rate_a_per_s_ = current_a_per_s;
  }
  void set_fall_rate(float current_a_per_s) {
    this->fall_rate_a_per_s_ = current_a_per_s;
  }
  void set_proportional_gain(float gain) { this->proportional_gain_ = gain; }
  void set_integral_gain(float gain_per_s) {
    this->integral_gain_per_s_ = gain_per_s;
  }
  void set_log_control_samples(bool enabled) {
    this->log_control_samples_ = enabled;
  }

  // Cooling policy.
  void set_fan_temperature_range(float start_c, float full_c) {
    this->fan_start_temperature_c_ = start_c;
    this->fan_full_temperature_c_ = full_c;
  }

  // Fault policy. Auto-clear only changes FAULT -> IDLE after the original
  // fault condition has remained absent; it never resumes an operation.
  void set_fault_auto_clear(bool auto_clear) {
    this->fault_policy_.auto_clear = auto_clear;
  }
  void set_fault_clear_delay_ms(uint32_t delay_ms) {
    this->fault_policy_.clear_delay_ms = delay_ms;
  }

  // User-facing entities.
  void set_manual_current_number(number::Number *number) {
    this->manual_current_number_ = number;
  }
  void set_state_sensor(text_sensor::TextSensor *sensor) {
    this->state_sensor_ = sensor;
  }
  void set_fault_sensor(text_sensor::TextSensor *sensor) {
    this->fault_sensor_ = sensor;
  }

  // Manual operation and procedures share one exclusive owner slot.
  bool start_manual(float current_a);
  bool update_manual(float current_a);
  bool start_procedure(Procedure *procedure);
  bool stop_procedure(Procedure *procedure);
  void stop();

  // Fault API. The public fault text is the deterministic comma-delimited
  // list of every latched cause.
  bool clear_fault();
  FaultFlags faults() const { return this->faults_; }
  State state() const { return this->state_; }

  const Measurement &measurement() const { return this->measurement_; }
  const ChargerMeasurement &charger_measurement() const {
    return this->charger_measurement_;
  }
  const HardwareLimits &hardware_limits() const {
    return this->hardware_limits_;
  }
  const Limits &limits() const { return this->limits_; }

 protected:
  void update_measurement_();
  void update_charger_measurement_();
  void update_faults_();
  void update_operation_();
  void update_control_();
  void reset_control_();
  void reset_control_integrator_();
  void update_fan_();

  FaultFlags detect_running_faults_() const;
  bool fault_conditions_active_(FaultFlags faults) const;
  bool required_temperature_unavailable_() const;
  void apply_procedure_result_(const ProcedureResult &result);
  bool apply_charger_command_(ChargerCommand command);
  bool charger_control_mismatch_() const;
  void release_owner_(StopReason reason);
  void set_state_(State state);
  void trip_fault_(Fault fault);
  void trip_faults_(FaultFlags faults);
  void publish_status_();
  void publish_calibration_();
  float effective_current_limit_() const;
  void drive_output_(float current_a);
  void force_output_off_();

  bool save_calibration_();

  output::FloatOutput *dac_output_{nullptr};
  output::FloatOutput *fan_output_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  std::vector<TemperatureInput> temperature_inputs_;

  ::component_common::ChargerInterface *charger_{nullptr};

  number::Number *manual_current_number_{nullptr};
  text_sensor::TextSensor *state_sensor_{nullptr};
  text_sensor::TextSensor *fault_sensor_{nullptr};
  text_sensor::TextSensor *calibration_status_sensor_{nullptr};
  sensor::Sensor *current_scale_sensor_{nullptr};
  sensor::Sensor *current_offset_sensor_{nullptr};
  sensor::Sensor *voltage_scale_sensor_{nullptr};
  sensor::Sensor *voltage_offset_sensor_{nullptr};
  sensor::Sensor *output_zero_level_sensor_{nullptr};
  sensor::Sensor *output_full_scale_current_sensor_{nullptr};

  Procedure *active_procedure_{nullptr};
  OperationLock operation_lock_{};

  Measurement measurement_{};
  ChargerMeasurement charger_measurement_{};
  HardwareLimits hardware_limits_{};
  Limits limits_{};
  FaultPolicy fault_policy_{};
  Calibration configured_calibration_{};
  Calibration calibration_{};

  State state_{State::IDLE};
  FaultFlags faults_{0u};
  CalibrationSource calibration_source_{CalibrationSource::CONFIGURED};

  float requested_current_a_{0.0f};
  float commanded_current_a_{0.0f};
  float control_integrator_a_{0.0f};

  bool restore_calibration_{true};
  uint32_t sample_timeout_ms_{250};
  uint32_t control_period_ms_{50};
  uint32_t fault_clear_candidate_since_ms_{0};
  uint32_t current_updated_ms_{0};
  uint32_t voltage_updated_ms_{0};
  uint32_t measurement_sequence_{0};
  uint32_t current_sequence_{0};
  uint32_t last_control_current_sequence_{0};
  uint32_t last_control_current_timestamp_ms_{0};
  bool control_has_current_sample_{false};
  bool current_seen_{false};
  bool voltage_seen_{false};

  uint32_t charger_sample_timeout_ms_{3000};
  uint32_t charger_control_timeout_ms_{5000};
  bool charger_command_known_{false};
  bool charger_commanded_enabled_{false};
  uint32_t charger_command_changed_ms_{0};
  uint32_t last_charger_command_attempt_ms_{0};

  uint32_t last_control_ms_{0};
  uint32_t last_fan_update_ms_{0};

  float deadband_a_{0.01f};
  float rise_rate_a_per_s_{2.0f};
  float fall_rate_a_per_s_{4.0f};
  float proportional_gain_{0.2f};
  float integral_gain_per_s_{0.4f};
  bool log_control_samples_{false};
  float fan_start_temperature_c_{35.0f};
  float fan_full_temperature_c_{70.0f};

  static constexpr uint32_t CALIBRATION_PREFERENCE_KEY = 0x504C4341u;
  decltype(global_preferences->make_preference<Calibration>(0))
      calibration_preference_{};
  bool calibration_preference_valid_{false};
};

class ManualCurrentNumber : public number::Number {
 public:
  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  void control(float value) override;
  ProgrammableLoadComponent *parent_{nullptr};
};

class ClearFaultButton : public button::Button {
 public:
  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  ProgrammableLoadComponent *parent_{nullptr};
};

class ResetCalibrationButton : public button::Button {
 public:
  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  ProgrammableLoadComponent *parent_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome

#include "battery_cycle.h"
#include "dcr_test.h"
#include "calibration_actions.h"
