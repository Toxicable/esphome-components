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

bool finite_positive(float value) {
  return std::isfinite(value) && value > 0.0f;
}

float clampf(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}
}  // namespace

const char *state_to_string(State state) {
  switch (state) {
    case State::IDLE:
      return "idle";
    case State::RUNNING:
      return "running";
    case State::FAULT:
      return "fault";
    default:
      return "unknown";
  }
}

const char *fault_to_string(Fault fault) {
  switch (fault) {
    case Fault::NONE:
      return "none";
    case Fault::CURRENT_MEASUREMENT_UNAVAILABLE:
      return "current_measurement_unavailable";
    case Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE:
      return "voltage_measurement_unavailable";
    case Fault::REQUIRED_TEMPERATURE_UNAVAILABLE:
      return "required_temperature_unavailable";
    case Fault::INPUT_UNDERVOLTAGE:
      return "input_undervoltage";
    case Fault::INPUT_OVERVOLTAGE:
      return "input_overvoltage";
    case Fault::HARDWARE_OVERVOLTAGE:
      return "hardware_overvoltage";
    case Fault::OVERCURRENT:
      return "overcurrent";
    case Fault::OVERPOWER:
      return "overpower";
    case Fault::OVERTEMPERATURE:
      return "overtemperature";
    case Fault::CONTROL_ERROR:
      return "control_error";
    case Fault::PROCEDURE_ERROR:
      return "procedure_error";
    default:
      return "unknown";
  }
}

void ProgrammableLoadComponent::setup() {
  if (this->control_period_ms_ < 10) {
    ESP_LOGW(TAG, "Control period clamped to 10 ms");
    this->control_period_ms_ = 10;
  } else if (this->control_period_ms_ > 1000) {
    ESP_LOGW(TAG, "Control period clamped to 1000 ms");
    this->control_period_ms_ = 1000;
  }

  if (!finite_positive(this->hardware_limits_.maximum_voltage_v) ||
      this->hardware_limits_.maximum_voltage_v > ABSOLUTE_MAXIMUM_VOLTAGE_V) {
    ESP_LOGW(TAG, "Hardware voltage limit clamped to absolute %.1f V ceiling",
             ABSOLUTE_MAXIMUM_VOLTAGE_V);
    this->hardware_limits_.maximum_voltage_v = ABSOLUTE_MAXIMUM_VOLTAGE_V;
  }

  this->configured_calibration_.version = CALIBRATION_VERSION;
  this->calibration_.version = CALIBRATION_VERSION;

  if (global_preferences != nullptr) {
    this->calibration_preference_ =
        global_preferences->make_preference<Calibration>(CALIBRATION_PREFERENCE_KEY);
    this->calibration_preference_valid_ = true;

    if (this->restore_calibration_) {
      Calibration persisted{};
      if (this->calibration_preference_.load(&persisted)) {
        if (this->calibration_valid_(persisted)) {
          this->calibration_ = persisted;
          ESP_LOGI(TAG, "Restored programmable-load calibration");
        } else {
          ESP_LOGW(TAG, "Ignoring invalid persisted calibration");
        }
      }
    }
  }

  const uint32_t now = millis();
  if (this->current_sensor_ != nullptr) {
    this->current_sensor_->add_on_state_callback([this](float) {
      this->current_updated_ms_ = millis();
      this->current_seen_ = true;
      this->measurement_sequence_++;
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
  if (this->fan_output_ != nullptr) {
    this->fan_output_->set_level(0.0f);
  }

  this->update_measurement_();
  this->publish_status_();
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
  ESP_LOGCONFIG(TAG, "  Fault: %s", fault_to_string(this->fault_));
  ESP_LOGCONFIG(TAG, "  Hardware maximum voltage: %.2f V",
                this->hardware_limits_.maximum_voltage_v);
  ESP_LOGCONFIG(TAG, "  Absolute maximum voltage: %.2f V",
                ABSOLUTE_MAXIMUM_VOLTAGE_V);
  ESP_LOGCONFIG(TAG, "  Operating voltage: %.2f to %.2f V",
                this->limits_.minimum_voltage_v, this->limits_.maximum_voltage_v);
  ESP_LOGCONFIG(TAG, "  Maximum current: %.3f A",
                this->limits_.maximum_current_a);
  ESP_LOGCONFIG(TAG, "  Maximum power: %.1f W", this->limits_.maximum_power_w);
  ESP_LOGCONFIG(TAG, "  Maximum temperature: %.1f °C",
                this->limits_.maximum_temperature_c);
  ESP_LOGCONFIG(TAG, "  DAC zero level: %.6f",
                this->calibration_.output.zero_level);
  ESP_LOGCONFIG(TAG, "  DAC full-scale current: %.3f A",
                this->calibration_.output.full_scale_current_a);
  ESP_LOGCONFIG(TAG, "  Control period: %u ms", this->control_period_ms_);
  ESP_LOGCONFIG(TAG, "  Current deadband: %.4f A", this->deadband_a_);
  ESP_LOGCONFIG(TAG, "  Rise/fall rates: %.3f / %.3f A/s",
                this->rise_rate_a_per_s_, this->fall_rate_a_per_s_);
  ESP_LOGCONFIG(TAG, "  Fault auto-clear: %s",
                this->fault_policy_.auto_clear ? "enabled" : "disabled");

  if (this->dac_output_ == nullptr) {
    ESP_LOGE(TAG, "  DAC output is not configured");
  }
  if (this->current_sensor_ == nullptr) {
    ESP_LOGE(TAG, "  Current sensor is not configured");
  }
  if (this->voltage_sensor_ == nullptr) {
    ESP_LOGE(TAG, "  Voltage sensor is not configured");
  }
}

void ProgrammableLoadComponent::loop() {
  const uint32_t now = millis();
  if ((uint32_t) (now - this->last_control_ms_) < this->control_period_ms_) {
    return;
  }
  this->last_control_ms_ = now;

  this->update_measurement_();
  this->update_faults_();

  if (this->state_ == State::RUNNING) {
    this->update_operation_();
    if (this->state_ == State::RUNNING) {
      this->update_faults_();
    }
    if (this->state_ == State::RUNNING) {
      this->update_control_();
    }
  } else {
    this->force_output_off_();
  }

  if ((uint32_t) (now - this->last_fan_update_ms_) >= 500) {
    this->last_fan_update_ms_ = now;
    this->update_fan_();
  }
}

bool ProgrammableLoadComponent::start_manual(float current_a) {
  if (this->state_ != State::IDLE || this->owner_ != Owner::NONE ||
      !std::isfinite(current_a) || current_a <= 0.0f) {
    return false;
  }

  this->update_measurement_();
  const Fault fault = this->detect_running_fault_();
  if (fault != Fault::NONE) {
    this->trip_fault_(fault);
    return false;
  }

  this->owner_ = Owner::MANUAL;
  this->requested_current_a_ =
      clampf(current_a, 0.0f, this->limits_.maximum_current_a);
  this->commanded_current_a_ = 0.0f;
  this->set_state_(State::RUNNING);

  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(this->requested_current_a_);
  }
  return true;
}

bool ProgrammableLoadComponent::update_manual(float current_a) {
  if (this->state_ != State::RUNNING || this->owner_ != Owner::MANUAL ||
      !std::isfinite(current_a)) {
    return false;
  }

  if (current_a <= 0.0f) {
    this->stop();
    return true;
  }

  this->requested_current_a_ =
      clampf(current_a, 0.0f, this->limits_.maximum_current_a);
  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(this->requested_current_a_);
  }
  return true;
}

bool ProgrammableLoadComponent::start_procedure(Procedure *procedure) {
  if (procedure == nullptr || this->state_ != State::IDLE ||
      this->owner_ != Owner::NONE) {
    return false;
  }

  this->update_measurement_();
  const Fault fault = this->detect_running_fault_();
  if (fault != Fault::NONE) {
    this->trip_fault_(fault);
    return false;
  }

  const ProcedureResult result = procedure->start(this->measurement_);
  if (result.status == ProcedureStatus::FAILED) {
    this->trip_fault_(result.fault == Fault::NONE ? Fault::PROCEDURE_ERROR
                                                  : result.fault);
    return false;
  }
  if (result.status == ProcedureStatus::COMPLETE) {
    procedure->stop(StopReason::COMPLETED);
    return true;
  }

  this->owner_ = Owner::PROCEDURE;
  this->active_procedure_ = procedure;
  this->commanded_current_a_ = 0.0f;
  this->set_state_(State::RUNNING);
  this->apply_procedure_result_(result);
  return this->state_ == State::RUNNING;
}

void ProgrammableLoadComponent::stop() {
  if (this->state_ == State::RUNNING) {
    this->release_owner_(StopReason::USER_REQUEST);
  } else {
    this->force_output_off_();
  }
}

bool ProgrammableLoadComponent::clear_fault() {
  if (this->state_ != State::FAULT) {
    return this->fault_ == Fault::NONE;
  }

  this->update_measurement_();
  if (this->fault_condition_active_(this->fault_)) {
    return false;
  }

  this->fault_ = Fault::NONE;
  this->fault_clear_candidate_since_ms_ = 0;
  this->set_state_(State::IDLE);
  this->publish_status_();
  return true;
}

void ProgrammableLoadComponent::update_measurement_() {
  const uint32_t now = millis();
  this->measurement_.timestamp_ms = now;
  this->measurement_.sequence = this->measurement_sequence_;

  this->measurement_.current_valid =
      this->current_sensor_ != nullptr && this->current_sensor_->has_state() &&
      std::isfinite(this->current_sensor_->state) &&
      this->current_seen_ &&
      (uint32_t) (now - this->current_updated_ms_) <= this->sample_timeout_ms_;
  if (this->measurement_.current_valid) {
    this->measurement_.current_a =
        this->calibration_.current.apply(this->current_sensor_->state);
    this->measurement_.current_valid =
        std::isfinite(this->measurement_.current_a);
  } else {
    this->measurement_.current_a = 0.0f;
  }

  this->measurement_.voltage_valid =
      this->voltage_sensor_ != nullptr && this->voltage_sensor_->has_state() &&
      std::isfinite(this->voltage_sensor_->state) &&
      this->voltage_seen_ &&
      (uint32_t) (now - this->voltage_updated_ms_) <= this->sample_timeout_ms_;
  if (this->measurement_.voltage_valid) {
    this->measurement_.voltage_v =
        this->calibration_.voltage.apply(this->voltage_sensor_->state);
    this->measurement_.voltage_valid =
        std::isfinite(this->measurement_.voltage_v);
  } else {
    this->measurement_.voltage_v = 0.0f;
  }

  float maximum_temperature = std::numeric_limits<float>::quiet_NaN();
  for (const TemperatureInput &input : this->temperature_inputs_) {
    if (input.sensor == nullptr || !input.sensor->has_state() ||
        !std::isfinite(input.sensor->state)) {
      continue;
    }
    if (!std::isfinite(maximum_temperature) ||
        input.sensor->state > maximum_temperature) {
      maximum_temperature = input.sensor->state;
    }
  }
  this->measurement_.maximum_temperature_c = maximum_temperature;
  this->measurement_.temperature_valid =
      !this->required_temperature_unavailable_();

  if (this->measurement_.current_valid &&
      this->measurement_.voltage_valid) {
    this->measurement_.power_w =
        this->measurement_.current_a * this->measurement_.voltage_v;
  } else {
    this->measurement_.power_w = 0.0f;
  }
}

void ProgrammableLoadComponent::update_faults_() {
  if (this->state_ == State::RUNNING) {
    const Fault fault = this->detect_running_fault_();
    if (fault != Fault::NONE) {
      this->trip_fault_(fault);
    }
    return;
  }

  if (this->state_ == State::IDLE) {
    // The board remains electrically connected while the load is idle, so the
    // absolute hardware ceiling is reported even when no operation owns the DAC.
    if (this->measurement_.voltage_valid &&
        this->measurement_.voltage_v >
            this->hardware_limits_.maximum_voltage_v) {
      this->trip_fault_(Fault::HARDWARE_OVERVOLTAGE);
    }
    return;
  }

  if (this->state_ != State::FAULT || !this->fault_policy_.auto_clear) {
    return;
  }

  if (this->fault_condition_active_(this->fault_)) {
    this->fault_clear_candidate_since_ms_ = 0;
    return;
  }

  const uint32_t now = millis();
  if (this->fault_clear_candidate_since_ms_ == 0) {
    this->fault_clear_candidate_since_ms_ = now == 0 ? 1 : now;
    return;
  }

  if ((uint32_t) (now - this->fault_clear_candidate_since_ms_) >=
      this->fault_policy_.clear_delay_ms) {
    this->clear_fault();
  }
}

void ProgrammableLoadComponent::update_operation_() {
  if (this->owner_ != Owner::PROCEDURE) {
    return;
  }
  if (this->active_procedure_ == nullptr) {
    this->trip_fault_(Fault::PROCEDURE_ERROR);
    return;
  }

  this->apply_procedure_result_(
      this->active_procedure_->update(this->measurement_));
}

void ProgrammableLoadComponent::update_control_() {
  const float current_limit = this->effective_current_limit_();
  if (!std::isfinite(current_limit) || current_limit < 0.0f ||
      !this->measurement_.current_valid) {
    this->trip_fault_(Fault::CONTROL_ERROR);
    return;
  }

  const float target =
      clampf(this->requested_current_a_, 0.0f, current_limit);
  const float measured = this->measurement_.current_a;
  const float error = target - measured;
  const float period_s =
      static_cast<float>(this->control_period_ms_) / 1000.0f;

  float next_command = this->commanded_current_a_;
  if (!std::isfinite(next_command)) {
    this->trip_fault_(Fault::CONTROL_ERROR);
    return;
  }

  if (error > this->deadband_a_) {
    next_command +=
        std::min(error, this->rise_rate_a_per_s_ * period_s);
  } else if (error < -this->deadband_a_) {
    next_command -=
        std::min(-error, this->fall_rate_a_per_s_ * period_s);
  }

  next_command = clampf(next_command, 0.0f, current_limit);
  this->commanded_current_a_ = next_command;
  this->drive_output_(next_command);
}

void ProgrammableLoadComponent::update_fan_() {
  if (this->fan_output_ == nullptr) {
    return;
  }

  const float temperature = this->measurement_.maximum_temperature_c;
  if (!std::isfinite(temperature)) {
    // Unknown temperature while active or faulted is treated conservatively.
    this->fan_output_->set_level(
        this->state_ == State::IDLE ? 0.0f : 1.0f);
    return;
  }

  if (temperature <= this->fan_start_temperature_c_) {
    this->fan_output_->set_level(0.0f);
  } else if (temperature >= this->fan_full_temperature_c_) {
    this->fan_output_->set_level(1.0f);
  } else {
    const float level =
        (temperature - this->fan_start_temperature_c_) /
        (this->fan_full_temperature_c_ -
         this->fan_start_temperature_c_);
    this->fan_output_->set_level(clampf(level, 0.0f, 1.0f));
  }
}

Fault ProgrammableLoadComponent::detect_running_fault_() const {
  if (!this->measurement_.current_valid) {
    return Fault::CURRENT_MEASUREMENT_UNAVAILABLE;
  }
  if (!this->measurement_.voltage_valid) {
    return Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE;
  }
  if (this->required_temperature_unavailable_()) {
    return Fault::REQUIRED_TEMPERATURE_UNAVAILABLE;
  }
  if (this->measurement_.voltage_v >
      this->hardware_limits_.maximum_voltage_v) {
    return Fault::HARDWARE_OVERVOLTAGE;
  }
  if (this->measurement_.voltage_v < this->limits_.minimum_voltage_v) {
    return Fault::INPUT_UNDERVOLTAGE;
  }
  if (this->measurement_.voltage_v > this->limits_.maximum_voltage_v) {
    return Fault::INPUT_OVERVOLTAGE;
  }
  if (std::fabs(this->measurement_.current_a) >
      this->limits_.maximum_current_a + this->deadband_a_) {
    return Fault::OVERCURRENT;
  }
  if (std::fabs(this->measurement_.power_w) >
      this->limits_.maximum_power_w) {
    return Fault::OVERPOWER;
  }
  if (std::isfinite(this->measurement_.maximum_temperature_c) &&
      this->measurement_.maximum_temperature_c >
          this->limits_.maximum_temperature_c) {
    return Fault::OVERTEMPERATURE;
  }
  return Fault::NONE;
}

bool ProgrammableLoadComponent::fault_condition_active_(Fault fault) const {
  switch (fault) {
    case Fault::NONE:
      return false;
    case Fault::CURRENT_MEASUREMENT_UNAVAILABLE:
      return !this->measurement_.current_valid;
    case Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE:
      return !this->measurement_.voltage_valid;
    case Fault::REQUIRED_TEMPERATURE_UNAVAILABLE:
      return this->required_temperature_unavailable_();
    case Fault::INPUT_UNDERVOLTAGE:
      return !this->measurement_.voltage_valid ||
             this->measurement_.voltage_v < this->limits_.minimum_voltage_v;
    case Fault::INPUT_OVERVOLTAGE:
      return !this->measurement_.voltage_valid ||
             this->measurement_.voltage_v > this->limits_.maximum_voltage_v;
    case Fault::HARDWARE_OVERVOLTAGE:
      return !this->measurement_.voltage_valid ||
             this->measurement_.voltage_v >
                 this->hardware_limits_.maximum_voltage_v;
    case Fault::OVERCURRENT:
      return !this->measurement_.current_valid ||
             std::fabs(this->measurement_.current_a) >
                 this->limits_.maximum_current_a + this->deadband_a_;
    case Fault::OVERPOWER:
      return !this->measurement_.current_valid ||
             !this->measurement_.voltage_valid ||
             std::fabs(this->measurement_.power_w) >
                 this->limits_.maximum_power_w;
    case Fault::OVERTEMPERATURE:
      return this->required_temperature_unavailable_() ||
             (std::isfinite(this->measurement_.maximum_temperature_c) &&
              this->measurement_.maximum_temperature_c >
                  this->limits_.maximum_temperature_c);
    case Fault::CONTROL_ERROR:
    case Fault::PROCEDURE_ERROR:
      return false;
    default:
      return true;
  }
}

bool ProgrammableLoadComponent::required_temperature_unavailable_() const {
  for (const TemperatureInput &input : this->temperature_inputs_) {
    if (!input.required) {
      continue;
    }
    if (input.sensor == nullptr || !input.sensor->has_state() ||
        !std::isfinite(input.sensor->state)) {
      return true;
    }
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
      result.requested_current_a < 0.0f) {
    this->trip_fault_(Fault::PROCEDURE_ERROR);
    return;
  }

  this->requested_current_a_ = result.requested_current_a;
}

void ProgrammableLoadComponent::release_owner_(StopReason reason) {
  Procedure *procedure = this->active_procedure_;
  this->active_procedure_ = nullptr;
  this->owner_ = Owner::NONE;
  this->requested_current_a_ = 0.0f;
  this->commanded_current_a_ = 0.0f;
  this->force_output_off_();

  if (procedure != nullptr) {
    procedure->stop(reason);
  }

  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(0.0f);
  }

  if (this->state_ != State::FAULT) {
    this->set_state_(State::IDLE);
  }
}

void ProgrammableLoadComponent::set_state_(State state) {
  if (this->state_ == state) {
    return;
  }
  this->state_ = state;
  this->publish_status_();
}

void ProgrammableLoadComponent::trip_fault_(Fault fault) {
  if (fault == Fault::NONE) {
    return;
  }

  Procedure *procedure = this->active_procedure_;
  this->active_procedure_ = nullptr;
  this->owner_ = Owner::NONE;
  this->requested_current_a_ = 0.0f;
  this->commanded_current_a_ = 0.0f;
  this->force_output_off_();

  if (procedure != nullptr) {
    procedure->stop(StopReason::FAULTED);
  }
  if (this->manual_current_number_ != nullptr) {
    this->manual_current_number_->publish_state(0.0f);
  }

  this->fault_ = fault;
  this->state_ = State::FAULT;
  this->fault_clear_candidate_since_ms_ = 0;
  this->publish_status_();
  ESP_LOGE(TAG, "Load fault: %s", fault_to_string(fault));
}

void ProgrammableLoadComponent::publish_status_() {
  if (this->state_sensor_ != nullptr) {
    this->state_sensor_->publish_state(state_to_string(this->state_));
  }
  if (this->fault_sensor_ != nullptr) {
    this->fault_sensor_->publish_state(fault_to_string(this->fault_));
  }
}

float ProgrammableLoadComponent::effective_current_limit_() const {
  if (!this->measurement_.voltage_valid ||
      this->measurement_.voltage_v <= 0.0f) {
    return 0.0f;
  }

  float limit = std::min(this->limits_.maximum_current_a,
                         this->calibration_.output.full_scale_current_a);
  if (this->limits_.maximum_power_w > 0.0f) {
    limit = std::min(
        limit,
        this->limits_.maximum_power_w /
            std::fabs(this->measurement_.voltage_v));
  }
  return std::max(0.0f, limit);
}

void ProgrammableLoadComponent::drive_output_(float current_a) {
  if (this->dac_output_ == nullptr ||
      !this->calibration_valid_(this->calibration_)) {
    if (this->dac_output_ != nullptr) {
      this->dac_output_->set_level(0.0f);
    }
    return;
  }

  const float zero = this->calibration_.output.zero_level;
  const float normalized_current =
      clampf(current_a / this->calibration_.output.full_scale_current_a,
             0.0f, 1.0f);
  const float level = zero + normalized_current * (1.0f - zero);
  this->dac_output_->set_level(clampf(level, 0.0f, 1.0f));
}

void ProgrammableLoadComponent::force_output_off_() {
  if (this->dac_output_ == nullptr) {
    return;
  }
  const float zero = this->calibration_valid_(this->calibration_)
                         ? this->calibration_.output.zero_level
                         : 0.0f;
  this->dac_output_->set_level(clampf(zero, 0.0f, 1.0f));
}

bool ProgrammableLoadComponent::calibration_valid_(
    const Calibration &calibration) const {
  return calibration.version == CALIBRATION_VERSION &&
         finite_positive(calibration.current.scale) &&
         std::isfinite(calibration.current.offset) &&
         finite_positive(calibration.voltage.scale) &&
         std::isfinite(calibration.voltage.offset) &&
         std::isfinite(calibration.output.zero_level) &&
         calibration.output.zero_level >= 0.0f &&
         calibration.output.zero_level < 1.0f &&
         finite_positive(calibration.output.full_scale_current_a);
}

bool ProgrammableLoadComponent::save_calibration_() {
  if (!this->calibration_preference_valid_) {
    return false;
  }
  return this->calibration_preference_.save(&this->calibration_);
}

bool ProgrammableLoadComponent::apply_calibration(
    const Calibration &calibration, bool persist) {
  if (this->state_ != State::IDLE ||
      !this->calibration_valid_(calibration)) {
    return false;
  }

  const Calibration previous = this->calibration_;
  this->calibration_ = calibration;
  this->force_output_off_();

  if (persist && !this->save_calibration_()) {
    this->calibration_ = previous;
    this->force_output_off_();
    return false;
  }
  return true;
}

bool ProgrammableLoadComponent::reset_calibration(bool persist) {
  return this->apply_calibration(this->configured_calibration_, persist);
}

void ManualCurrentNumber::control(float value) {
  if (this->parent_ == nullptr || !std::isfinite(value)) {
    return;
  }

  if (value <= 0.0f) {
    this->parent_->stop();
    this->publish_state(0.0f);
    return;
  }

  bool accepted = false;
  if (this->parent_->state() == State::IDLE) {
    accepted = this->parent_->start_manual(value);
  } else if (this->parent_->state() == State::RUNNING) {
    accepted = this->parent_->update_manual(value);
  }

  if (!accepted) {
    this->publish_state(0.0f);
  }
}

void ClearFaultButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->clear_fault();
  }
}

}  // namespace programmable_load
}  // namespace esphome
