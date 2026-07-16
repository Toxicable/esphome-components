#include "programmable_load.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace programmable_load {

namespace {
static const char *const TAG = "programmable_load";
static constexpr uint32_t CHARGER_COMMAND_RETRY_MS = 1000;

bool finite_positive(float value) {
  return std::isfinite(value) && value > 0.0f;
}

float clampf(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}







}  // namespace

void ProgrammableLoadComponent::setup() {
  this->control_period_ms_ =
      std::max<uint32_t>(10, std::min<uint32_t>(1000, this->control_period_ms_));
  this->hardware_limits_.maximum_voltage_v =
      normalize_hardware_maximum_voltage(
          this->hardware_limits_.maximum_voltage_v);

  this->configured_calibration_.version = CALIBRATION_VERSION;
  this->calibration_.version = CALIBRATION_VERSION;
  if (global_preferences != nullptr) {
    this->calibration_preference_ =
        global_preferences->make_preference<Calibration>(CALIBRATION_PREFERENCE_KEY);
    this->calibration_preference_valid_ = true;
    if (this->restore_calibration_) {
      Calibration persisted{};
      if (this->calibration_preference_.load(&persisted) &&
          ::programmable_load_core::calibration_valid(persisted)) {
        this->calibration_ = persisted;
        this->calibration_source_ = CalibrationSource::RESTORED;
        ESP_LOGI(TAG, "Restored programmable-load calibration");
      }
    }
  }

  const uint32_t now = millis();
  if (this->current_sensor_ != nullptr) {
    this->current_sensor_->add_on_state_callback([this](float) {
      this->current_updated_ms_ = millis();
      this->current_seen_ = true;
      this->measurement_sequence_++;
      this->current_sequence_++;
    });
    if (this->current_sensor_->has_state()) {
      this->current_updated_ms_ = now;
      this->current_seen_ = true;
    }
  }
  if (this->voltage_sensor_ != nullptr) {
    this->voltage_sensor_->add_on_state_callback([this](float) {
      this->voltage_updated_ms_ = millis();
      this->voltage_seen_ = true;
      this->measurement_sequence_++;
    });
    if (this->voltage_sensor_->has_state()) {
      this->voltage_updated_ms_ = now;
      this->voltage_seen_ = true;
    }
  }
  this->force_output_off_();
  if (this->fan_output_ != nullptr) this->fan_output_->set_level(0.0f);
  this->apply_charger_command_(ChargerCommand::DISABLE);
  this->update_measurement_();
  this->update_charger_measurement_();
  this->publish_status_();
  this->publish_calibration_();
  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(0.0f);
  }
  this->last_control_ms_ = now;
  this->last_fan_update_ms_ = now;
  ESP_LOGCONFIG(TAG, "Programmable load initialized");
}

void ProgrammableLoadComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Programmable Load:");
  ESP_LOGCONFIG(TAG, "  State: %s", state_to_string(this->state_));
  char faults[256];
  format_faults(this->faults_, faults, sizeof(faults));
  ESP_LOGCONFIG(TAG, "  Fault: %s", faults);
  ESP_LOGCONFIG(TAG, "  Calibration: %s",
                calibration_source_to_string(this->calibration_source_));
  ESP_LOGCONFIG(TAG, "    Current: raw * %.7g + %.7g A",
                this->calibration_.current.scale,
                this->calibration_.current.offset);
  ESP_LOGCONFIG(TAG, "    Voltage: raw * %.7g + %.7g V",
                this->calibration_.voltage.scale,
                this->calibration_.voltage.offset);
  ESP_LOGCONFIG(TAG, "    Output: zero=%.7g full-scale=%.7g A",
                this->calibration_.output.zero_level,
                this->calibration_.output.full_scale_current_a);
  ESP_LOGCONFIG(TAG, "  Hardware maximum voltage: %.2f V",
                this->hardware_limits_.maximum_voltage_v);
  ESP_LOGCONFIG(TAG, "  Operating voltage: %.2f to %.2f V",
                this->limits_.minimum_voltage_v, this->limits_.maximum_voltage_v);
  ESP_LOGCONFIG(TAG, "  Maximum current/power: %.3f A / %.1f W",
                this->limits_.maximum_current_a, this->limits_.maximum_power_w);
  ESP_LOGCONFIG(TAG, "  Charger capability: %s",
                this->charger_ != nullptr ? "configured" : "not configured");
  if (this->dac_output_ == nullptr) ESP_LOGE(TAG, "  DAC output is not configured");
  if (this->current_sensor_ == nullptr) ESP_LOGE(TAG, "  Current sensor is not configured");
  if (this->voltage_sensor_ == nullptr) ESP_LOGE(TAG, "  Voltage sensor is not configured");
}

void ProgrammableLoadComponent::loop() {
  const uint32_t now = millis();
  if ((uint32_t) (now - this->last_control_ms_) < this->control_period_ms_) return;
  this->last_control_ms_ = now;

  this->update_measurement_();
  this->update_charger_measurement_();
  this->update_faults_();
  if (this->state_ == State::RUNNING) {
    this->update_operation_();
    if (this->state_ == State::RUNNING) this->update_faults_();
    if (this->state_ == State::RUNNING) this->update_control_();
  } else {
    this->force_output_off_();
  }
  if ((uint32_t) (now - this->last_fan_update_ms_) >= 500) {
    this->last_fan_update_ms_ = now;
    this->update_fan_();
  }
}

bool ProgrammableLoadComponent::start_manual(float current_a) {
  if (this->state_ != State::IDLE || !finite_positive(current_a) ||
      !this->operation_lock_.acquire_manual()) return false;
  this->update_measurement_();
  this->update_charger_measurement_();
  const FaultFlags faults = this->detect_running_faults_();
  if (faults != 0u) {
    this->trip_faults_(faults);
    return false;
  }
  this->apply_charger_command_(ChargerCommand::DISABLE);
  this->requested_current_a_ =
      clampf(current_a, 0.0f, this->limits_.maximum_current_a);
  this->reset_control_();
  this->set_state_(State::RUNNING);
  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(this->requested_current_a_);
  }
  return true;
}

bool ProgrammableLoadComponent::update_manual(float current_a) {
  if (this->state_ != State::RUNNING ||
      !this->operation_lock_.owns_manual() || !std::isfinite(current_a))
    return false;
  if (current_a <= 0.0f) {
    this->stop();
    return true;
  }
  this->requested_current_a_ =
      clampf(current_a, 0.0f, this->limits_.maximum_current_a);
  this->reset_control_integrator_();
  this->apply_charger_command_(ChargerCommand::DISABLE);
  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(this->requested_current_a_);
  }
  return true;
}

bool ProgrammableLoadComponent::start_procedure(Procedure *procedure) {
  if (procedure == nullptr || this->state_ != State::IDLE ||
      !this->operation_lock_.acquire_procedure(procedure)) return false;
  this->update_measurement_();
  this->update_charger_measurement_();
  const FaultFlags faults = this->detect_running_faults_();
  if (faults != 0u) {
    this->trip_faults_(faults);
    return false;
  }
  this->apply_charger_command_(ChargerCommand::DISABLE);
  const ProcedureContext context{this->measurement_, this->charger_measurement_};
  const ProcedureResult result = procedure->start(context);
  if (result.status == ProcedureStatus::FAILED) {
    this->trip_fault_(result.fault == Fault::NONE ? Fault::PROCEDURE_ERROR
                                                  : result.fault);
    return false;
  }
  if (result.status == ProcedureStatus::COMPLETE) {
    this->operation_lock_.release_procedure(procedure);
    procedure->stop(StopReason::COMPLETED);
    return true;
  }
  this->active_procedure_ = procedure;
  this->reset_control_();
  this->set_state_(State::RUNNING);
  this->apply_procedure_result_(result);
  return this->state_ == State::RUNNING;
}

bool ProgrammableLoadComponent::stop_procedure(Procedure *procedure) {
  if (procedure == nullptr || this->state_ != State::RUNNING ||
      !this->operation_lock_.owns_procedure(procedure) ||
      this->active_procedure_ != procedure) {
    return false;
  }
  this->release_owner_(StopReason::USER_REQUEST);
  return true;
}

void ProgrammableLoadComponent::stop() {
  if (this->state_ == State::RUNNING) {
    this->release_owner_(StopReason::USER_REQUEST);
  } else {
    this->force_output_off_();
    this->apply_charger_command_(ChargerCommand::DISABLE);
  }
}

bool ProgrammableLoadComponent::clear_fault() {
  if (this->state_ != State::FAULT) return this->faults_ == 0u;
  this->update_measurement_();
  this->update_charger_measurement_();
  this->apply_charger_command_(ChargerCommand::DISABLE);
  if (this->fault_conditions_active_(this->faults_)) return false;
  this->faults_ = 0u;
  this->fault_clear_candidate_since_ms_ = 0;
  this->set_state_(State::IDLE);
  return true;
}

void ProgrammableLoadComponent::update_measurement_() {
  const uint32_t now = millis();
  this->measurement_.timestamp_ms = now;
  this->measurement_.sequence = this->measurement_sequence_;
  this->measurement_.current_valid =
      this->current_sensor_ != nullptr && this->current_sensor_->has_state() &&
      std::isfinite(this->current_sensor_->state) && this->current_seen_ &&
      (uint32_t) (now - this->current_updated_ms_) <= this->sample_timeout_ms_;
  this->measurement_.current_a = this->measurement_.current_valid
                                     ? this->calibration_.current.apply(
                                           this->current_sensor_->state)
                                     : 0.0f;
  this->measurement_.current_valid &=
      std::isfinite(this->measurement_.current_a);

  this->measurement_.voltage_valid =
      this->voltage_sensor_ != nullptr && this->voltage_sensor_->has_state() &&
      std::isfinite(this->voltage_sensor_->state) && this->voltage_seen_ &&
      (uint32_t) (now - this->voltage_updated_ms_) <= this->sample_timeout_ms_;
  this->measurement_.voltage_v = this->measurement_.voltage_valid
                                     ? this->calibration_.voltage.apply(
                                           this->voltage_sensor_->state)
                                     : 0.0f;
  this->measurement_.voltage_valid &=
      std::isfinite(this->measurement_.voltage_v);

  float maximum_temperature = std::numeric_limits<float>::quiet_NaN();
  for (const TemperatureInput &input : this->temperature_inputs_) {
    if (input.sensor == nullptr || !input.sensor->has_state() ||
        !std::isfinite(input.sensor->state)) continue;
    if (!std::isfinite(maximum_temperature) ||
        input.sensor->state > maximum_temperature) {
      maximum_temperature = input.sensor->state;
    }
  }
  this->measurement_.maximum_temperature_c = maximum_temperature;
  this->measurement_.temperature_valid =
      !this->required_temperature_unavailable_();
  this->measurement_.power_w =
      this->measurement_.current_valid && this->measurement_.voltage_valid
          ? this->measurement_.current_a * this->measurement_.voltage_v
          : 0.0f;
}

void ProgrammableLoadComponent::update_charger_measurement_() {
  this->charger_measurement_ = {};
  if (this->charger_ == nullptr) return;

  const auto capabilities = this->charger_->capabilities();
  if (!capabilities.enable_control || !capabilities.battery_current ||
      !capabilities.battery_voltage || !capabilities.charge_state ||
      !capabilities.power_good || !capabilities.fault_status) {
    return;
  }

  this->charger_measurement_ = this->charger_->snapshot();
  const uint32_t now = millis();
  const bool fresh = this->charger_measurement_.timestamp_ms != 0 &&
                     (uint32_t) (now - this->charger_measurement_.timestamp_ms) <=
                         this->charger_sample_timeout_ms_;
  this->charger_measurement_.valid =
      this->charger_measurement_.valid && fresh &&
      std::isfinite(this->charger_measurement_.current_a) &&
      std::isfinite(this->charger_measurement_.voltage_v);
}

void ProgrammableLoadComponent::update_faults_() {
  if (this->state_ == State::RUNNING) {
    const FaultFlags faults = this->detect_running_faults_();
    if (faults != 0u) this->trip_faults_(faults);
    return;
  }
  if (this->state_ == State::IDLE) {
    if (this->measurement_.voltage_valid &&
        this->measurement_.voltage_v >
            this->hardware_limits_.maximum_voltage_v) {
      this->trip_fault_(Fault::HARDWARE_OVERVOLTAGE);
    }
    return;
  }
  if (this->state_ != State::FAULT || !this->fault_policy_.auto_clear) return;
  if (this->fault_conditions_active_(this->faults_)) {
    this->fault_clear_candidate_since_ms_ = 0;
    return;
  }
  const uint32_t now = millis();
  if (this->fault_clear_candidate_since_ms_ == 0) {
    this->fault_clear_candidate_since_ms_ = now == 0 ? 1 : now;
  } else if ((uint32_t) (now - this->fault_clear_candidate_since_ms_) >=
             this->fault_policy_.clear_delay_ms) {
    this->clear_fault();
  }
}

void ProgrammableLoadComponent::update_operation_() {
  if (this->active_procedure_ == nullptr) {
    this->trip_fault_(Fault::PROCEDURE_ERROR);
    return;
  }
  if (!this->operation_lock_.owns_procedure(this->active_procedure_)) {
    this->trip_fault_(Fault::PROCEDURE_ERROR);
    return;
  }
  const ProcedureContext context{this->measurement_, this->charger_measurement_};
  this->apply_procedure_result_(this->active_procedure_->update(context));
}

void ProgrammableLoadComponent::update_control_() {
  const bool charger_enabled =
      this->charger_measurement_.valid && this->charger_measurement_.enabled;
  if (this->charger_commanded_enabled_ || charger_enabled) {
    this->reset_control_();
    this->force_output_off_();
    if (this->charger_control_mismatch_()) {
      this->trip_fault_(Fault::CHARGER_CONTROL_ERROR);
    }
    return;
  }
  if (this->charger_control_mismatch_()) {
    this->trip_fault_(Fault::CHARGER_CONTROL_ERROR);
    return;
  }
  const float current_limit = this->effective_current_limit_();
  if (!std::isfinite(current_limit) || current_limit < 0.0f ||
      !this->measurement_.current_valid) {
    this->trip_fault_(Fault::CONTROL_ERROR);
    return;
  }
  if (this->control_has_current_sample_ &&
      this->current_sequence_ == this->last_control_current_sequence_) {
    return;
  }
  const uint32_t current_timestamp_ms = this->current_updated_ms_;
  const float dt_s = this->control_has_current_sample_
                         ? static_cast<float>(current_timestamp_ms -
                                              this->last_control_current_timestamp_ms_) /
                               1000.0f
                         : static_cast<float>(this->control_period_ms_) / 1000.0f;
  this->last_control_current_sequence_ = this->current_sequence_;
  this->last_control_current_timestamp_ms_ = current_timestamp_ms;
  this->control_has_current_sample_ = true;
  if (!std::isfinite(dt_s) || dt_s <= 0.0f || dt_s > 1.0f) {
    return;
  }

  const float target = clampf(this->requested_current_a_, 0.0f, current_limit);
  const float error = target - this->measurement_.current_a;
  const float bounded_error = std::fabs(error) <= this->deadband_a_ ? 0.0f : error;
  if (!std::isfinite(this->commanded_current_a_) ||
      !std::isfinite(this->control_integrator_a_)) {
    this->trip_fault_(Fault::CONTROL_ERROR);
    return;
  }

  const float proportional = this->proportional_gain_ * bounded_error;
  float desired = target + proportional + this->control_integrator_a_;
  const bool saturated_low = desired < 0.0f;
  const bool saturated_high = desired > current_limit;
  if ((!saturated_low && !saturated_high) ||
      (saturated_low && bounded_error > 0.0f) ||
      (saturated_high && bounded_error < 0.0f)) {
    this->control_integrator_a_ = clampf(
        this->control_integrator_a_ +
            this->integral_gain_per_s_ * bounded_error * dt_s,
        -current_limit, current_limit);
    desired = target + proportional + this->control_integrator_a_;
  }
  desired = clampf(desired, 0.0f, current_limit);
  const float maximum_rise = this->rise_rate_a_per_s_ * dt_s;
  const float maximum_fall = this->fall_rate_a_per_s_ * dt_s;
  const float next = clampf(desired,
                            this->commanded_current_a_ - maximum_fall,
                            this->commanded_current_a_ + maximum_rise);
  this->commanded_current_a_ = clampf(next, 0.0f, current_limit);
  this->drive_output_(this->commanded_current_a_);
  if (this->log_control_samples_) {
    ESP_LOGI(TAG,
             "PI sample=%u dt=%.3fs target=%.3fA measured=%.3fA error=%.3fA "
             "p=%.3fA i=%.3fA desired=%.3fA command=%.3fA",
             this->current_sequence_, dt_s, target,
             this->measurement_.current_a, error, proportional,
             this->control_integrator_a_, desired, this->commanded_current_a_);
  }
}

void ProgrammableLoadComponent::reset_control_() {
  this->commanded_current_a_ = 0.0f;
  this->reset_control_integrator_();
}

void ProgrammableLoadComponent::reset_control_integrator_() {
  this->control_integrator_a_ = 0.0f;
  // Do not reuse the sample that preceded a target or ownership change.
  // The next command is produced only after the current sensor publishes again.
  this->control_has_current_sample_ = true;
  this->last_control_current_sequence_ = this->current_sequence_;
  this->last_control_current_timestamp_ms_ = this->current_updated_ms_;
}

void ProgrammableLoadComponent::update_fan_() {
  if (this->fan_output_ == nullptr) return;
  const float temperature = this->measurement_.maximum_temperature_c;
  if (!std::isfinite(temperature)) {
    this->fan_output_->set_level(this->state_ == State::IDLE ? 0.0f : 1.0f);
  } else if (temperature <= this->fan_start_temperature_c_) {
    this->fan_output_->set_level(0.0f);
  } else if (temperature >= this->fan_full_temperature_c_) {
    this->fan_output_->set_level(1.0f);
  } else {
    this->fan_output_->set_level(clampf(
        (temperature - this->fan_start_temperature_c_) /
            (this->fan_full_temperature_c_ - this->fan_start_temperature_c_),
        0.0f, 1.0f));
  }
}

FaultFlags ProgrammableLoadComponent::detect_running_faults_() const {
  return ::programmable_load_core::detect_safety_faults(
      this->measurement_, this->hardware_limits_, this->limits_,
      this->deadband_a_, this->required_temperature_unavailable_(),
      this->charger_control_mismatch_());
}

bool ProgrammableLoadComponent::fault_conditions_active_(
    FaultFlags faults) const {
  return ::programmable_load_core::fault_conditions_active(
      faults, this->measurement_, this->hardware_limits_, this->limits_,
      this->deadband_a_, this->required_temperature_unavailable_(),
      this->charger_control_mismatch_(), this->charger_ != nullptr,
      this->charger_measurement_.valid,
      this->charger_measurement_.fault_active);
}

bool ProgrammableLoadComponent::required_temperature_unavailable_() const {
  for (const TemperatureInput &input : this->temperature_inputs_) {
    if (!input.required) continue;
    if (input.sensor == nullptr || !input.sensor->has_state() ||
        !std::isfinite(input.sensor->state)) return true;
  }
  return false;
}

void ProgrammableLoadComponent::apply_procedure_result_(
    const ProcedureResult &result) {
  if (result.status == ProcedureStatus::FAILED) {
    this->trip_fault_(result.fault == Fault::NONE ? Fault::PROCEDURE_ERROR
                                                  : result.fault);
    return;
  }
  if (result.status == ProcedureStatus::COMPLETE) {
    this->release_owner_(StopReason::COMPLETED);
    return;
  }
  if (!std::isfinite(result.requested_current_a) ||
      result.requested_current_a < 0.0f ||
      (result.charger_command == ChargerCommand::ENABLE &&
       result.requested_current_a > this->deadband_a_)) {
    this->trip_fault_(Fault::PROCEDURE_ERROR);
    return;
  }
  if (!this->apply_charger_command_(result.charger_command)) {
    this->trip_fault_(Fault::CHARGER_CONTROL_ERROR);
    return;
  }
  if (this->requested_current_a_ != result.requested_current_a) {
    this->reset_control_integrator_();
  }
  this->requested_current_a_ = result.requested_current_a;
  if (result.charger_command == ChargerCommand::ENABLE) {
    this->reset_control_();
    this->force_output_off_();
  }
}

bool ProgrammableLoadComponent::apply_charger_command_(
    ChargerCommand command) {
  const bool enabled = command == ChargerCommand::ENABLE;
  if (this->charger_ == nullptr) return !enabled;
  if (!this->charger_->capabilities().enable_control) return false;

  const uint32_t now = millis();
  if (!this->charger_command_known_ ||
      this->charger_commanded_enabled_ != enabled) {
    this->charger_command_known_ = true;
    this->charger_commanded_enabled_ = enabled;
    this->charger_command_changed_ms_ = now == 0 ? 1 : now;
    this->last_charger_command_attempt_ms_ = 0;
  }
  if ((!this->charger_measurement_.valid ||
       this->charger_measurement_.enabled != enabled) &&
      (this->last_charger_command_attempt_ms_ == 0 ||
       (uint32_t) (now - this->last_charger_command_attempt_ms_) >=
           CHARGER_COMMAND_RETRY_MS)) {
    this->last_charger_command_attempt_ms_ = now == 0 ? 1 : now;
    if (!this->charger_->request_enabled(enabled)) return false;
  }
  return true;
}

bool ProgrammableLoadComponent::charger_control_mismatch_() const {
  if (!this->charger_command_known_ || this->charger_ == nullptr ||
      !this->charger_measurement_.valid ||
      this->charger_measurement_.enabled == this->charger_commanded_enabled_) {
    return false;
  }
  return this->charger_command_changed_ms_ != 0 &&
         (uint32_t) (millis() - this->charger_command_changed_ms_) >=
             this->charger_control_timeout_ms_;
}

void ProgrammableLoadComponent::release_owner_(StopReason reason) {
  Procedure *procedure = this->active_procedure_;
  this->active_procedure_ = nullptr;
  this->operation_lock_.force_release();
  this->requested_current_a_ = 0.0f;
  this->reset_control_();
  this->force_output_off_();
  this->apply_charger_command_(ChargerCommand::DISABLE);
  if (procedure != nullptr) procedure->stop(reason);
  if (this->manual_current_number_ != nullptr)
    this->manual_current_number_->publish_state(0.0f);
  if (this->state_ != State::FAULT) this->set_state_(State::IDLE);
}

void ProgrammableLoadComponent::set_state_(State state) {
  if (this->state_ == state) return;
  this->state_ = state;
  this->publish_status_();
}

void ProgrammableLoadComponent::trip_fault_(Fault fault) {
  this->trip_faults_(fault_flag(fault));
}

void ProgrammableLoadComponent::trip_faults_(FaultFlags faults) {
  if (faults == 0u) return;
  Procedure *procedure = this->active_procedure_;
  this->active_procedure_ = nullptr;
  this->operation_lock_.force_release();
  this->requested_current_a_ = 0.0f;
  this->reset_control_();
  this->force_output_off_();
  this->apply_charger_command_(ChargerCommand::DISABLE);
  if (procedure != nullptr) procedure->stop(StopReason::FAULTED);
  if (this->manual_current_number_ != nullptr)
    this->manual_current_number_->publish_state(0.0f);
  this->faults_ |= faults;
  this->state_ = State::FAULT;
  this->fault_clear_candidate_since_ms_ = 0;
  this->publish_status_();
  char formatted[256];
  format_faults(this->faults_, formatted, sizeof(formatted));
  ESP_LOGE(TAG, "Load fault: %s", formatted);
}

void ProgrammableLoadComponent::publish_status_() {
  if (this->state_sensor_ != nullptr)
    this->state_sensor_->publish_state(state_to_string(this->state_));
  if (this->fault_sensor_ != nullptr) {
    char faults[256];
    format_faults(this->faults_, faults, sizeof(faults));
    this->fault_sensor_->publish_state(faults);
  }
}

void ProgrammableLoadComponent::publish_calibration_() {
  if (this->calibration_status_sensor_ != nullptr)
    this->calibration_status_sensor_->publish_state(
        calibration_source_to_string(this->calibration_source_));
  if (this->current_scale_sensor_ != nullptr)
    this->current_scale_sensor_->publish_state(this->calibration_.current.scale);
  if (this->current_offset_sensor_ != nullptr)
    this->current_offset_sensor_->publish_state(this->calibration_.current.offset);
  if (this->voltage_scale_sensor_ != nullptr)
    this->voltage_scale_sensor_->publish_state(this->calibration_.voltage.scale);
  if (this->voltage_offset_sensor_ != nullptr)
    this->voltage_offset_sensor_->publish_state(this->calibration_.voltage.offset);
  if (this->output_zero_level_sensor_ != nullptr)
    this->output_zero_level_sensor_->publish_state(
        this->calibration_.output.zero_level);
  if (this->output_full_scale_current_sensor_ != nullptr)
    this->output_full_scale_current_sensor_->publish_state(
        this->calibration_.output.full_scale_current_a);
}

float ProgrammableLoadComponent::effective_current_limit_() const {
  return ::programmable_load_core::effective_current_limit(
      this->measurement_, this->limits_, this->calibration_);
}

void ProgrammableLoadComponent::drive_output_(float current_a) {
  if (this->dac_output_ == nullptr ||
      !::programmable_load_core::calibration_valid(this->calibration_)) {
    if (this->dac_output_ != nullptr) this->dac_output_->set_level(0.0f);
    return;
  }
  const float zero = this->calibration_.output.zero_level;
  const float normalized =
      clampf(current_a / this->calibration_.output.full_scale_current_a,
             0.0f, 1.0f);
  this->dac_output_->set_level(
      clampf(zero + normalized * (1.0f - zero), 0.0f, 1.0f));
}

void ProgrammableLoadComponent::force_output_off_() {
  if (this->dac_output_ == nullptr) return;
  const float zero = ::programmable_load_core::calibration_valid(this->calibration_)
                         ? this->calibration_.output.zero_level
                         : 0.0f;
  this->dac_output_->set_level(clampf(zero, 0.0f, 1.0f));
}

bool ProgrammableLoadComponent::save_calibration_() {
  return this->calibration_preference_valid_ &&
         this->calibration_preference_.save(&this->calibration_);
}

bool ProgrammableLoadComponent::apply_calibration(
    const Calibration &calibration, bool persist, CalibrationSource source) {
  if (this->state_ != State::IDLE ||
      !::programmable_load_core::calibration_valid(calibration)) {
    ESP_LOGW(TAG, "Rejected calibration while busy or with invalid values");
    return false;
  }
  const Calibration previous = this->calibration_;
  const CalibrationSource previous_source = this->calibration_source_;
  this->calibration_ = calibration;
  this->calibration_source_ = source;
  this->force_output_off_();
  if (persist && !this->save_calibration_()) {
    this->calibration_ = previous;
    this->calibration_source_ = previous_source;
    this->force_output_off_();
    ESP_LOGW(TAG, "Failed to persist calibration; active values were restored");
    return false;
  }
  this->publish_calibration_();
  ESP_LOGI(TAG, "Applied programmable-load calibration (%s)",
           calibration_source_to_string(source));
  return true;
}

bool ProgrammableLoadComponent::reset_calibration(bool persist) {
  return this->apply_calibration(this->configured_calibration_, persist,
                                 CalibrationSource::CONFIGURED);
}

void ManualCurrentNumber::control(float value) {
  if (this->parent_ == nullptr || !std::isfinite(value)) return;
  if (value <= 0.0f) {
    this->parent_->stop();
    this->publish_state(0.0f);
    return;
  }
  bool accepted = false;
  if (this->parent_->state() == State::IDLE)
    accepted = this->parent_->start_manual(value);
  else if (this->parent_->state() == State::RUNNING)
    accepted = this->parent_->update_manual(value);
  if (!accepted) this->publish_state(0.0f);
}

void ClearFaultButton::press_action() {
  if (this->parent_ != nullptr) this->parent_->clear_fault();
}

void ResetCalibrationButton::press_action() {
  if (this->parent_ != nullptr) this->parent_->reset_calibration(true);
}

}  // namespace programmable_load
}  // namespace esphome

