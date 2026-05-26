#include "programmable_load.h"

#include <cmath>

#include "esphome/core/log.h"

namespace esphome {
namespace programmable_load {

static const char *const TAG = "programmable_load";

void ProgrammableLoadComponent::setup() {
  ESP_LOGCONFIG(TAG, "ProgrammableLoadComponent: control_period=%u ms", this->control_period_ms_);

  // Clamp control period to reasonable bounds.
  if (this->control_period_ms_ < 10) {
    this->control_period_ms_ = 10;
    ESP_LOGW(TAG, "control_period_ms clamped to 10ms (minimum)");
  }
  if (this->control_period_ms_ > 1000) {
    this->control_period_ms_ = 1000;
    ESP_LOGW(TAG, "control_period_ms clamped to 1000ms (maximum)");
  }

  // Ensure DAC starts at zero.
  if (this->dac_output_ != nullptr) {
    this->dac_output_->set_level(0.0f);
  }

  // Fan starts off.
  if (this->fan_output_ != nullptr) {
    this->fan_output_->set_level(0.0f);
  }

  // Register the tight control loop.
  this->set_interval("pl_control", this->control_period_ms_,
                     [this]() { this->control_loop_(); });

  // Register the slow update (fan, publishing).
  this->set_interval("pl_slow", 500,
                     [this]() { this->slow_update_(); });
}

void ProgrammableLoadComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ProgrammableLoad:");
  ESP_LOGCONFIG(TAG, "  Max current: %.1f A", this->max_current_a_);
  ESP_LOGCONFIG(TAG, "  Voltage min: %.2f V", this->voltage_min_v_);
  ESP_LOGCONFIG(TAG, "  Max temp: %.1f °C", this->max_temp_c_);
  ESP_LOGCONFIG(TAG, "  Control period: %u ms", this->control_period_ms_);
  ESP_LOGCONFIG(TAG, "  Deadband: %.3f A", this->deadband_a_);
  ESP_LOGCONFIG(TAG, "  Max unconfirmed rise: %.3f A, fall: %.3f A",
                this->max_unconfirmed_rise_a_, this->max_unconfirmed_fall_a_);
  ESP_LOGCONFIG(TAG, "  Ramp fast: %.1f A/s, medium: %.1f A/s", this->ramp_fast_a_per_s_, this->ramp_medium_a_per_s_);
  ESP_LOGCONFIG(TAG, "  Fan start: %.1f °C, full: %.1f °C", this->fan_start_temp_c_, this->fan_full_temp_c_);

  if (this->dac_output_ == nullptr) {
    ESP_LOGW(TAG, "  WARNING: No DAC output configured!");
  }
  if (this->current_sensor_ == nullptr) {
    ESP_LOGW(TAG, "  WARNING: No current sensor configured!");
  }
  if (this->voltage_sensor_ == nullptr) {
    ESP_LOGW(TAG, "  WARNING: No voltage sensor configured!");
  }
  if (this->temperature_sensors_.empty()) {
    ESP_LOGW(TAG, "  WARNING: No temperature sensors configured!");
  }
  if (this->ntc_present_sensors_.empty()) {
    ESP_LOGW(TAG, "  WARNING: No NTC present sensor configured; using first temperature sensor state as fallback");
  }
}

void ProgrammableLoadComponent::set_target(float amps) {
  float target = amps;
  if (target < 0.0f) target = 0.0f;
  if (target > this->max_current_a_) target = this->max_current_a_;

  if (target <= 0.0f) {
    this->force_off();
    return;
  }

  this->current_target_a_ = target;

  // Reset ramping state.
  this->unconfirmed_rise_a_ = 0.0f;
  this->unconfirmed_fall_a_ = 0.0f;

  // Capture DCR baseline (start voltage/current at setpoint time).
  bool current_ok = this->current_sensor_ != nullptr
                    && this->current_sensor_->has_state()
                    && !std::isnan(this->current_sensor_->state);
  bool voltage_ok = this->voltage_sensor_ != nullptr
                    && this->voltage_sensor_->has_state()
                    && !std::isnan(this->voltage_sensor_->state);

  this->reset_dcr_();

  if (current_ok && voltage_ok) {
    this->dcr_start_current_a_ = this->current_sensor_->state;
    this->dcr_start_voltage_v_ = this->voltage_sensor_->state;
  } else {
    this->dcr_start_current_a_ = std::numeric_limits<float>::quiet_NaN();
    this->dcr_start_voltage_v_ = std::numeric_limits<float>::quiet_NaN();
  }

  // Clear faults.
  this->clear_faults_();

  ESP_LOGI(TAG, "SETPOINT %.3fA", target);

  if (this->setpoint_number_ != nullptr) {
    this->setpoint_number_->publish_state(target);
  }

  this->ramp_state_ = RampState::HOLDING;
}

void ProgrammableLoadComponent::force_off() {
  // Stop targeting.
  this->current_target_a_ = 0.0f;
  this->current_command_a_ = 0.0f;
  this->unconfirmed_rise_a_ = 0.0f;
  this->unconfirmed_fall_a_ = 0.0f;

  // Reset DCR tracking.
  this->dcr_start_voltage_v_ = std::numeric_limits<float>::quiet_NaN();
  this->dcr_start_current_a_ = std::numeric_limits<float>::quiet_NaN();
  this->reset_dcr_();

  // DAC to zero.
  if (this->dac_output_ != nullptr) {
    this->dac_output_->set_level(0.0f);
  }

  // Update setpoint display.
  if (this->setpoint_number_ != nullptr) {
    this->setpoint_number_->publish_state(0.0f);
  }

  this->ramp_state_ = RampState::OFF;
  ESP_LOGI(TAG, "LOAD OFF: DAC=0, command=0");
}

bool ProgrammableLoadComponent::check_safety_() {
  // Check INA current available.
  bool ina_ok = this->current_sensor_ != nullptr
                && this->current_sensor_->has_state()
                && !std::isnan(this->current_sensor_->state);

  if (!ina_ok) {
    ESP_LOGW(TAG, "FAULT: INA current unavailable; forcing load off");
    this->force_off();
    return false;
  }

  // Check NTC 1 present (only the first NTC is required).
  bool ntc1_present = true;
  if (!this->ntc_present_sensors_.empty()) {
    binary_sensor::BinarySensor *bs = this->ntc_present_sensors_[0];
    if (bs != nullptr && !bs->state) {
      ntc1_present = false;
    }
  } else if (!this->temperature_sensors_.empty()) {
    // No explicit NTC-present signals; check first temperature sensor.
    sensor::Sensor *s = this->temperature_sensors_[0];
    if (s == nullptr || !s->has_state() || std::isnan(s->state)) {
      ntc1_present = false;
    }
  }

  if (!ntc1_present) {
    this->fault_ntc_missing_ = true;
    ESP_LOGW(TAG, "FAULT: NTC missing; forcing load off");
    this->force_off();
    return false;
  }

  // Check input voltage present.
  bool vbus_ok = this->voltage_sensor_ != nullptr
                 && this->voltage_sensor_->has_state()
                 && !std::isnan(this->voltage_sensor_->state)
                 && this->voltage_sensor_->state > this->voltage_min_v_;

  if (!vbus_ok) {
    this->fault_no_voltage_ = true;
    ESP_LOGW(TAG, "FAULT: input voltage missing; forcing load off");
    this->force_off();
    return false;
  }

  // Check temperature.
  float max_temp = this->get_max_temp_();
  bool temp_ok = std::isnan(max_temp) || max_temp < this->max_temp_c_;

  if (!temp_ok) {
    this->fault_over_temp_ = true;
    ESP_LOGW(TAG, "FAULT: over temperature (%.1f°C >= %.1f°C); forcing load off", max_temp, this->max_temp_c_);
    this->force_off();
    return false;
  }

  // All checks passed — clear faults.
  this->clear_faults_();
  return true;
}

void ProgrammableLoadComponent::clear_faults_() {
  this->fault_ntc_missing_ = false;
  this->fault_no_voltage_ = false;
  this->fault_over_temp_ = false;
}

void ProgrammableLoadComponent::reset_dcr_() {
  this->dcr_sample_count_ = 0;
  this->dcr_delta_voltage_mv_ = std::numeric_limits<float>::quiet_NaN();
  this->dcr_delta_current_a_ = std::numeric_limits<float>::quiet_NaN();
  this->dcr_mohm_ = std::numeric_limits<float>::quiet_NaN();
}

// Capture a sample during upward ramping.
void ProgrammableLoadComponent::capture_dcr_sample_() {
  if (this->current_sensor_ == nullptr || this->voltage_sensor_ == nullptr)
    return;
  if (std::isnan(this->dcr_start_voltage_v_) || std::isnan(this->dcr_start_current_a_))
    return;

  float i = this->current_sensor_->state;
  float v = this->voltage_sensor_->state;

  if (std::isnan(i) || std::isnan(v))
    return;

  // Only record samples where current has moved up from baseline.
  if (i <= this->dcr_start_current_a_)
    return;

  DcrSample &s = this->dcr_samples_[this->dcr_sample_count_ % DCR_SAMPLE_MAX];
  s.current_a = i;
  s.voltage_v = v;
  if (this->dcr_sample_count_ < DCR_SAMPLE_MAX)
    this->dcr_sample_count_++;
}

// Compute DCR from the sample buffer using least-squares fit.
void ProgrammableLoadComponent::compute_dcr_() {
  if (this->dcr_sample_count_ < 2)
    return;

  // Accumulate sums over all samples.
  float sum_di = 0.0f;
  float sum_dv = 0.0f;
  float sum_di2 = 0.0f;
  float sum_di_dv = 0.0f;
  int n = 0;

  for (int j = 0; j < this->dcr_sample_count_; j++) {
    const float di = this->dcr_samples_[j].current_a - this->dcr_start_current_a_;
    const float dv = this->dcr_start_voltage_v_ - this->dcr_samples_[j].voltage_v;
    if (di < 0.0f || dv < 0.0f)
      continue;  // skip invalid samples
    n++;
    sum_di += di;
    sum_dv += dv;
    sum_di2 += di * di;
    sum_di_dv += di * dv;
  }

  if (n < 2)
    return;

  // Least-squares: R = (n*sum(di*dv) - sum_di*sum_dv) / (n*sum_di2 - sum_di*sum_di)
  const float denom = n * sum_di2 - sum_di * sum_di;
  if (denom <= 0.0f)
    return;

  const float r_ohm = (n * sum_di_dv - sum_di * sum_dv) / denom;
  if (r_ohm < 0.0f)
    return;

  this->dcr_mohm_ = r_ohm * 1000.0f;

  // Also publish the last sample's deltas.
  int last = (this->dcr_sample_count_ - 1) % DCR_SAMPLE_MAX;
  const DcrSample &ls = this->dcr_samples_[last];
  this->dcr_delta_current_a_ = ls.current_a - this->dcr_start_current_a_;
  this->dcr_delta_voltage_mv_ = (this->dcr_start_voltage_v_ - ls.voltage_v) * 1000.0f;
}

float ProgrammableLoadComponent::get_max_temp_() const {
  float max_t = std::numeric_limits<float>::quiet_NaN();

  for (sensor::Sensor *s : this->temperature_sensors_) {
    if (s == nullptr)
      continue;
    float t = s->state;
    if (std::isnan(t))
      continue;
    if (std::isnan(max_t) || t > max_t)
      max_t = t;
  }

  return max_t;
}

void ProgrammableLoadComponent::update_fan_() {
  if (this->fan_output_ == nullptr)
    return;

  float t = this->get_max_temp_();

  if (std::isnan(t)) {
    this->fan_output_->set_level(0.0f);
    return;
  }
  if (t <= this->fan_start_temp_c_) {
    this->fan_output_->set_level(0.0f);
    return;
  }
  if (t >= this->fan_full_temp_c_) {
    this->fan_output_->set_level(1.0f);
    return;
  }

  float level = (t - this->fan_start_temp_c_) / (this->fan_full_temp_c_ - this->fan_start_temp_c_);
  this->fan_output_->set_level(level);
}

float ProgrammableLoadComponent::clamp_command_(float cmd) const {
  if (cmd < 0.0f) cmd = 0.0f;
  if (cmd > this->max_current_a_) cmd = this->max_current_a_;
  return cmd;
}

const char *ProgrammableLoadComponent::ramp_state_to_string_(RampState state) const {
  switch (state) {
    case RampState::OFF: return "OFF";
    case RampState::RAMPING_UP: return "RAMPING_UP";
    case RampState::RAMPING_DOWN: return "RAMPING_DOWN";
    case RampState::HOLDING: return "HOLDING";
    default: return "UNKNOWN";
  }
}

void ProgrammableLoadComponent::publish_state_() {
  // Publish DCR.
  if (this->dcr_sensor_ != nullptr) {
    this->dcr_sensor_->publish_state(this->dcr_mohm_);
  }

  // Publish voltage drop.
  if (this->voltage_drop_sensor_ != nullptr) {
    this->voltage_drop_sensor_->publish_state(this->dcr_delta_voltage_mv_);
  }

  // Publish current delta.
  if (this->current_delta_sensor_ != nullptr) {
    this->current_delta_sensor_->publish_state(this->dcr_delta_current_a_);
  }

  if (this->ramp_state_sensor_ != nullptr) {
    this->ramp_state_sensor_->publish_state(this->ramp_state_to_string_(this->ramp_state_));
  }

  // Publish fault states.
  if (this->fault_ntc_missing_sensor_ != nullptr) {
    this->fault_ntc_missing_sensor_->publish_state(this->fault_ntc_missing_);
  }
  if (this->fault_no_voltage_sensor_ != nullptr) {
    this->fault_no_voltage_sensor_->publish_state(this->fault_no_voltage_);
  }
  if (this->fault_over_temp_sensor_ != nullptr) {
    this->fault_over_temp_sensor_->publish_state(this->fault_over_temp_);
  }
}

void ProgrammableLoadComponent::control_loop_() {
  // If target is zero, we're off.
  if (this->current_target_a_ <= 0.0f) {
    return;
  }

  // Safety gate.
  if (!this->check_safety_()) {
    return;
  }

  const float target = this->current_target_a_;
  const float i = this->current_sensor_->state;
  const float vbus = this->voltage_sensor_->state;

  const float err = target - i;
  const float abs_err = fabsf(err);

  const float control_period_s = (float) this->control_period_ms_ / 1000.0f;

  // Clamp current command.
  float cmd = this->current_command_a_;
  if (std::isnan(cmd) || cmd < 0.0f) cmd = 0.0f;
  if (cmd > this->max_current_a_) cmd = this->max_current_a_;

  float next_cmd = cmd;

  if (err > this->deadband_a_) {
    // --- RAMPING UP ---
    this->unconfirmed_fall_a_ = 0.0f;
    this->ramp_state_ = RampState::RAMPING_UP;

    const float max_rise = this->max_unconfirmed_rise_a_;
    float rise_remaining = max_rise - this->unconfirmed_rise_a_;
    if (rise_remaining < 0.0f) rise_remaining = 0.0f;

    if (rise_remaining <= 0.0005f) {
      // Unconfirmed rise cap hit — hold.
      next_cmd = cmd;
      this->ramp_state_ = RampState::HOLDING;
    } else {
      float inc = 0.002f;

      if (err > 5.0f) {
        inc = this->ramp_fast_a_per_s_ * control_period_s;
      } else if (err > 2.0f) {
        inc = this->ramp_medium_a_per_s_ * control_period_s;
      } else if (err > 0.75f) {
        inc = 0.050f;
      } else if (err > 0.25f) {
        inc = 0.020f;
      } else if (err > 0.060f) {
        inc = 0.010f;
      } else {
        inc = 0.003f;
      }

      if (inc > rise_remaining) inc = rise_remaining;

      next_cmd = cmd + inc;
      this->unconfirmed_rise_a_ += inc;
    }

  } else if (err < -this->deadband_a_) {
    // --- RAMPING DOWN ---
    this->unconfirmed_rise_a_ = 0.0f;
    this->ramp_state_ = RampState::RAMPING_DOWN;

    const float max_fall = this->max_unconfirmed_fall_a_;
    float fall_remaining = max_fall - this->unconfirmed_fall_a_;
    if (fall_remaining < 0.0f) fall_remaining = 0.0f;

    if (fall_remaining <= 0.0005f) {
      next_cmd = cmd;
      this->ramp_state_ = RampState::HOLDING;
    } else {
      float dec = 0.002f;

      if (abs_err > 5.0f) {
        dec = 0.500f;
      } else if (abs_err > 2.0f) {
        dec = 0.250f;
      } else if (abs_err > 0.75f) {
        dec = 0.100f;
      } else if (abs_err > 0.25f) {
        dec = 0.040f;
      } else if (abs_err > 0.08f) {
        dec = 0.015f;
      } else if (abs_err > 0.025f) {
        dec = 0.005f;
      } else {
        dec = 0.002f;
      }

      if (dec > fall_remaining) dec = fall_remaining;

      next_cmd = cmd - dec;
      this->unconfirmed_fall_a_ += dec;
    }

  } else {
    // --- WITHIN DEADBAND ---
    this->unconfirmed_rise_a_ = 0.0f;
    this->unconfirmed_fall_a_ = 0.0f;
    next_cmd = cmd;
    this->ramp_state_ = RampState::HOLDING;
  }

  next_cmd = this->clamp_command_(next_cmd);
  this->current_command_a_ = next_cmd;

  // Capture DCR sample during upward ramping.
  if (this->ramp_state_ == RampState::RAMPING_UP) {
    this->capture_dcr_sample_();
  }

  // Compute DCR from accumulated samples (runs every loop but only updates when we have data).
  this->compute_dcr_();

  // Drive DAC (normalized 0..1).
  if (this->dac_output_ != nullptr) {
    this->dac_output_->set_level(next_cmd / this->max_current_a_);
  }

  // Conditional logging.
  this->log_divider_++;
  const bool log_now =
    this->log_divider_ >= 4
    || err < -this->deadband_a_
    || fabsf(next_cmd - cmd) >= 0.020f;

  if (log_now) {
    this->log_divider_ = 0;
    ESP_LOGI(TAG,
             "CTRL target=%.3fA ina=%.3fA err=%.3fA db=%.3fA rise=%.3f fall=%.3f cmd %.3f->%.3f vbus=%.2fV",
             target, i, err, this->deadband_a_,
             this->unconfirmed_rise_a_, this->unconfirmed_fall_a_,
             cmd, next_cmd, vbus);
  }
}

void ProgrammableLoadComponent::slow_update_() {
  // Safety checks even when not actively ramping, as long as the output is on.
  if (this->current_command_a_ > 0.01f && !this->check_safety_()) {
    return;
  }

  // Fan control.
  this->update_fan_();

  // Publish derived sensors.
  this->publish_state_();
}

void ProgrammableLoadSetpointNumber::control(float value) {
  if (this->parent_ == nullptr)
    return;
  this->parent_->set_target(value);
}

}  // namespace programmable_load
}  // namespace esphome
