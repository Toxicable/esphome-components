#include "battery_cycle.h"

#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "programmable_load.h"

namespace esphome {
namespace programmable_load {

namespace {
static const char *const BATTERY_CYCLE_TAG =
    "programmable_load.battery_cycle";
static constexpr double MILLISECONDS_PER_HOUR = 3600000.0;
}  // namespace

ProcedureResult BatteryCycle::start(const ProcedureContext &context) {
  if (!context.load.current_valid || !context.load.voltage_valid ||
      !context.charger.valid || !std::isfinite(this->discharge_current_a_) ||
      this->discharge_current_a_ <= 0.0f ||
      !std::isfinite(this->discharge_cutoff_voltage_v_) ||
      this->discharge_cutoff_voltage_v_ <= 0.0f ||
      context.charger.fault_active) {
    return this->failed_(context.charger.fault_active ? Fault::CHARGER_FAULT
                                                      : Fault::PROCEDURE_ERROR);
  }

  this->reset_integrators_();
  this->completed_ = false;
  this->cutoff_started_ms_ = 0;
  this->charge_started_ms_ = 0;
  this->charge_stall_started_ms_ = 0;
  this->termination_started_ms_ = 0;
  this->charge_activity_seen_ = false;
  this->last_load_sequence_ = context.load.sequence;
  this->last_charger_sequence_ = context.charger.sequence;
  this->set_phase_(BatteryCyclePhase::DISCHARGING);
  this->publish_result_("running");

  ESP_LOGI(BATTERY_CYCLE_TAG,
           "Starting battery cycle: discharge=%.3f A cutoff=%.3f V",
           this->discharge_current_a_, this->discharge_cutoff_voltage_v_);
  return this->running_(this->discharge_current_a_,
                        ChargerCommand::DISABLE);
}

ProcedureResult BatteryCycle::update(const ProcedureContext &context) {
  const uint32_t now = millis();

  switch (this->phase_) {
    case BatteryCyclePhase::DISCHARGING: {
      if (!context.load.current_valid || !context.load.voltage_valid) {
        return this->failed_(Fault::PROCEDURE_ERROR);
      }
      this->integrate_discharge_(context.load);

      if (context.load.voltage_v <= this->discharge_cutoff_voltage_v_) {
        if (this->cutoff_started_ms_ == 0) {
          this->cutoff_started_ms_ = now == 0 ? 1 : now;
        }
      } else if (context.load.voltage_v >=
                 this->discharge_cutoff_voltage_v_ +
                     this->discharge_cutoff_hysteresis_v_) {
        this->cutoff_started_ms_ = 0;
      }

      if (this->cutoff_started_ms_ != 0 &&
          (uint32_t) (now - this->cutoff_started_ms_) >=
              this->discharge_cutoff_hold_time_ms_) {
        this->publish_results_();
        this->set_phase_(BatteryCyclePhase::RESTING);
        return this->running_(0.0f, ChargerCommand::DISABLE);
      }
      return this->running_(this->discharge_current_a_,
                            ChargerCommand::DISABLE);
    }

    case BatteryCyclePhase::RESTING:
      if ((uint32_t) (now - this->phase_started_ms_) >= this->rest_time_ms_) {
        this->have_last_charger_sample_ = false;
        this->charge_activity_seen_ = false;
        this->charge_started_ms_ = now == 0 ? 1 : now;
        this->charge_stall_started_ms_ = 0;
        this->termination_started_ms_ = 0;
        this->set_phase_(BatteryCyclePhase::CHARGE_STARTING);
        return this->running_(0.0f, ChargerCommand::ENABLE);
      }
      return this->running_(0.0f, ChargerCommand::DISABLE);

    case BatteryCyclePhase::CHARGE_STARTING:
      if (context.charger.valid && context.charger.fault_active) {
        return this->failed_(Fault::CHARGER_FAULT);
      }
      if (context.charger.valid && context.charger.power_good &&
          context.charger.state == ChargerState::TERMINATION_DONE) {
        this->set_phase_(BatteryCyclePhase::TERMINATION_HOLD);
        this->termination_started_ms_ = now == 0 ? 1 : now;
        return this->running_(0.0f, ChargerCommand::ENABLE);
      }
      if (context.charger.valid && context.charger.power_good &&
          charge_state_active_(context.charger.state)) {
        this->charge_activity_seen_ = true;
        this->set_phase_(BatteryCyclePhase::CHARGING);
        this->integrate_charge_(context.charger);
        return this->running_(0.0f, ChargerCommand::ENABLE);
      }
      if ((uint32_t) (now - this->phase_started_ms_) >=
          this->charge_start_timeout_ms_) {
        return this->failed_(Fault::CHARGER_UNAVAILABLE);
      }
      return this->running_(0.0f, ChargerCommand::ENABLE);

    case BatteryCyclePhase::CHARGING:
      if (!context.charger.valid) {
        return this->failed_(Fault::CHARGER_UNAVAILABLE);
      }
      if (context.charger.fault_active) {
        return this->failed_(Fault::CHARGER_FAULT);
      }
      if (this->charge_started_ms_ != 0 &&
          (uint32_t) (now - this->charge_started_ms_) >=
              this->charge_timeout_ms_) {
        return this->failed_(Fault::CHARGE_TIMEOUT);
      }

      this->integrate_charge_(context.charger);

      if (context.charger.state == ChargerState::TERMINATION_DONE) {
        this->termination_started_ms_ = now == 0 ? 1 : now;
        this->set_phase_(BatteryCyclePhase::TERMINATION_HOLD);
        return this->running_(0.0f, ChargerCommand::ENABLE);
      }

      if (context.charger.power_good &&
          charge_state_active_(context.charger.state)) {
        this->charge_activity_seen_ = true;
        this->charge_stall_started_ms_ = 0;
      } else if (this->charge_activity_seen_) {
        if (this->charge_stall_started_ms_ == 0) {
          this->charge_stall_started_ms_ = now == 0 ? 1 : now;
        } else if ((uint32_t) (now - this->charge_stall_started_ms_) >=
                   this->charge_stall_timeout_ms_) {
          return this->failed_(Fault::CHARGER_FAULT);
        }
      }
      return this->running_(0.0f, ChargerCommand::ENABLE);

    case BatteryCyclePhase::TERMINATION_HOLD:
      if (!context.charger.valid) {
        return this->failed_(Fault::CHARGER_UNAVAILABLE);
      }
      if (context.charger.fault_active) {
        return this->failed_(Fault::CHARGER_FAULT);
      }
      if (context.charger.state != ChargerState::TERMINATION_DONE) {
        this->termination_started_ms_ = 0;
        this->set_phase_(BatteryCyclePhase::CHARGING);
        return this->running_(0.0f, ChargerCommand::ENABLE);
      }
      if (this->termination_started_ms_ == 0) {
        this->termination_started_ms_ = now == 0 ? 1 : now;
      }
      if ((uint32_t) (now - this->termination_started_ms_) >=
          this->termination_hold_time_ms_) {
        return this->complete_();
      }
      return this->running_(0.0f, ChargerCommand::ENABLE);

    case BatteryCyclePhase::IDLE:
    default:
      return this->failed_(Fault::PROCEDURE_ERROR);
  }
}

void BatteryCycle::stop(StopReason reason) {
  this->publish_results_();
  if (!this->completed_) {
    if (reason == StopReason::USER_REQUEST) {
      this->publish_result_("user_stopped");
    } else if (reason == StopReason::FAULTED) {
      this->publish_result_("fault");
    }
  }
  this->set_phase_(BatteryCyclePhase::IDLE);
}

bool BatteryCycle::charge_state_active_(ChargerState state) {
  return state == ChargerState::TRICKLE ||
         state == ChargerState::PRECHARGE || state == ChargerState::FAST_CC ||
         state == ChargerState::TAPER_CV || state == ChargerState::TOPOFF;
}

void BatteryCycle::set_phase_(BatteryCyclePhase phase) {
  this->phase_ = phase;
  this->phase_started_ms_ = millis();
  if (this->phase_sensor_ != nullptr) {
    this->phase_sensor_->publish_state(this->phase_to_string_());
  }
}

const char *BatteryCycle::phase_to_string_() const {
  switch (this->phase_) {
    case BatteryCyclePhase::IDLE:
      return "idle";
    case BatteryCyclePhase::DISCHARGING:
      return "discharging";
    case BatteryCyclePhase::RESTING:
      return "resting";
    case BatteryCyclePhase::CHARGE_STARTING:
      return "charge_starting";
    case BatteryCyclePhase::CHARGING:
      return "charging";
    case BatteryCyclePhase::TERMINATION_HOLD:
      return "termination_hold";
    default:
      return "unknown";
  }
}

void BatteryCycle::reset_integrators_() {
  this->discharged_capacity_ah_ = 0.0;
  this->discharged_energy_wh_ = 0.0;
  this->charged_capacity_ah_ = 0.0;
  this->charged_energy_wh_ = 0.0;
  this->have_last_load_sample_ = false;
  this->have_last_charger_sample_ = false;
}

void BatteryCycle::integrate_discharge_(const Measurement &measurement) {
  if (measurement.sequence == this->last_load_sequence_) {
    return;
  }
  this->last_load_sequence_ = measurement.sequence;

  const float current_a = std::fabs(measurement.current_a);
  const float power_w = std::fabs(measurement.power_w);
  if (this->have_last_load_sample_) {
    const uint32_t elapsed_ms =
        measurement.timestamp_ms - this->last_load_timestamp_ms_;
    const double elapsed_h =
        static_cast<double>(elapsed_ms) / MILLISECONDS_PER_HOUR;
    this->discharged_capacity_ah_ +=
        0.5 * static_cast<double>(this->last_load_current_a_ + current_a) *
        elapsed_h;
    this->discharged_energy_wh_ +=
        0.5 * static_cast<double>(this->last_load_power_w_ + power_w) *
        elapsed_h;
  }

  this->last_load_timestamp_ms_ = measurement.timestamp_ms;
  this->last_load_current_a_ = current_a;
  this->last_load_power_w_ = power_w;
  this->have_last_load_sample_ = true;
}

void BatteryCycle::integrate_charge_(const ChargerMeasurement &measurement) {
  if (measurement.sequence == this->last_charger_sequence_) {
    return;
  }
  this->last_charger_sequence_ = measurement.sequence;

  const float current_a = std::fabs(measurement.current_a);
  const float power_w = current_a * std::fabs(measurement.voltage_v);
  if (this->have_last_charger_sample_) {
    const uint32_t elapsed_ms =
        measurement.timestamp_ms - this->last_charger_timestamp_ms_;
    const double elapsed_h =
        static_cast<double>(elapsed_ms) / MILLISECONDS_PER_HOUR;
    this->charged_capacity_ah_ +=
        0.5 * static_cast<double>(this->last_charger_current_a_ + current_a) *
        elapsed_h;
    this->charged_energy_wh_ +=
        0.5 * static_cast<double>(this->last_charger_power_w_ + power_w) *
        elapsed_h;
  }

  this->last_charger_timestamp_ms_ = measurement.timestamp_ms;
  this->last_charger_current_a_ = current_a;
  this->last_charger_power_w_ = power_w;
  this->have_last_charger_sample_ = true;
}

void BatteryCycle::publish_results_() {
  if (this->discharged_capacity_sensor_ != nullptr) {
    this->discharged_capacity_sensor_->publish_state(
        static_cast<float>(this->discharged_capacity_ah_));
  }
  if (this->discharged_energy_sensor_ != nullptr) {
    this->discharged_energy_sensor_->publish_state(
        static_cast<float>(this->discharged_energy_wh_));
  }
  if (this->charged_capacity_sensor_ != nullptr) {
    this->charged_capacity_sensor_->publish_state(
        static_cast<float>(this->charged_capacity_ah_));
  }
  if (this->charged_energy_sensor_ != nullptr) {
    this->charged_energy_sensor_->publish_state(
        static_cast<float>(this->charged_energy_wh_));
  }
}

void BatteryCycle::publish_result_(const char *result) {
  if (this->result_sensor_ != nullptr) {
    this->result_sensor_->publish_state(result);
  }
}

ProcedureResult BatteryCycle::running_(float current_a,
                                       ChargerCommand command) const {
  return {ProcedureStatus::RUNNING, current_a, Fault::NONE, command};
}

ProcedureResult BatteryCycle::failed_(Fault fault) const {
  return {ProcedureStatus::FAILED, 0.0f,
          fault == Fault::NONE ? Fault::PROCEDURE_ERROR : fault,
          ChargerCommand::DISABLE};
}

ProcedureResult BatteryCycle::complete_() {
  this->completed_ = true;
  this->publish_results_();
  this->publish_result_("complete");
  ESP_LOGI(BATTERY_CYCLE_TAG,
           "Battery cycle complete: discharge=%.4f Ah %.3f Wh charge=%.4f Ah %.3f Wh",
           static_cast<float>(this->discharged_capacity_ah_),
           static_cast<float>(this->discharged_energy_wh_),
           static_cast<float>(this->charged_capacity_ah_),
           static_cast<float>(this->charged_energy_wh_));
  return {ProcedureStatus::COMPLETE, 0.0f, Fault::NONE,
          ChargerCommand::DISABLE};
}

void BatteryCycleStartButton::press_action() {
  if (this->host_ != nullptr && this->procedure_ != nullptr) {
    this->host_->start_procedure(this->procedure_);
  }
}

}  // namespace programmable_load
}  // namespace esphome
