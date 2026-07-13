#include "dcr_test.h"

#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "programmable_load.h"

namespace esphome {
namespace programmable_load {

namespace {
static const char *const DCR_TAG = "programmable_load.dcr";
static constexpr float MINIMUM_CURRENT_DELTA_A = 0.001f;
}  // namespace

ProcedureResult DcrTest::start(const ProcedureContext &context) {
  const Measurement &measurement = context.load;
  if (!measurement.current_valid || !measurement.voltage_valid ||
      !std::isfinite(this->baseline_current_a_) ||
      !std::isfinite(this->pulse_current_a_) ||
      this->baseline_current_a_ < 0.0f || this->pulse_current_a_ < 0.0f ||
      std::fabs(this->pulse_current_a_ - this->baseline_current_a_) <
          MINIMUM_CURRENT_DELTA_A ||
      this->repeats_ == 0) {
    return this->failed_();
  }

  this->completed_repeats_ = 0;
  this->valid_repeats_ = 0;
  this->resistance_sum_ohm_ = 0.0;
  this->last_sample_sequence_ = measurement.sequence;
  this->reset_baseline_samples_();
  this->reset_pulse_samples_();
  this->begin_phase_(DcrPhase::BASELINE_SETTLE);

  ESP_LOGI(DCR_TAG,
           "Starting DCR test: baseline=%.3f A pulse=%.3f A repeats=%u",
           this->baseline_current_a_, this->pulse_current_a_, this->repeats_);
  return this->running_(this->baseline_current_a_);
}

ProcedureResult DcrTest::update(const ProcedureContext &context) {
  const Measurement &measurement = context.load;
  if (!measurement.current_valid || !measurement.voltage_valid) {
    return this->failed_();
  }

  const uint32_t elapsed = millis() - this->phase_started_ms_;

  switch (this->phase_) {
    case DcrPhase::BASELINE_SETTLE:
      if (elapsed >= this->settle_time_ms_) {
        this->reset_baseline_samples_();
        this->last_sample_sequence_ = measurement.sequence;
        this->begin_phase_(DcrPhase::BASELINE_SAMPLE);
      }
      return this->running_(this->baseline_current_a_);

    case DcrPhase::BASELINE_SAMPLE:
      this->accumulate_baseline_(measurement);
      if (elapsed >= this->sample_time_ms_) {
        if (this->baseline_sample_count_ == 0) {
          return this->failed_();
        }
        this->reset_pulse_samples_();
        this->last_sample_sequence_ = measurement.sequence;
        this->begin_phase_(DcrPhase::PULSE_SETTLE);
      }
      return this->running_(this->baseline_current_a_);

    case DcrPhase::PULSE_SETTLE:
      if (elapsed >= this->settle_time_ms_) {
        this->reset_pulse_samples_();
        this->last_sample_sequence_ = measurement.sequence;
        this->begin_phase_(DcrPhase::PULSE_SAMPLE);
      }
      return this->running_(this->pulse_current_a_);

    case DcrPhase::PULSE_SAMPLE:
      this->accumulate_pulse_(measurement);
      if (elapsed >= this->sample_time_ms_) {
        if (!this->finish_repeat_()) {
          return this->failed_();
        }
        this->completed_repeats_++;
        this->begin_phase_(DcrPhase::RECOVERY);
      }
      return this->running_(this->pulse_current_a_);

    case DcrPhase::RECOVERY:
      if (elapsed < this->recovery_time_ms_) {
        return this->running_(this->baseline_current_a_);
      }

      if (this->completed_repeats_ >= this->repeats_) {
        if (this->valid_repeats_ == 0) {
          return this->failed_();
        }
        const float resistance_mohm =
            static_cast<float>(this->resistance_sum_ohm_ /
                               static_cast<double>(this->valid_repeats_) *
                               1000.0);
        if (!std::isfinite(resistance_mohm) || resistance_mohm <= 0.0f) {
          return this->failed_();
        }
        if (this->resistance_sensor_ != nullptr) {
          this->resistance_sensor_->publish_state(resistance_mohm);
        }
        ESP_LOGI(DCR_TAG, "DCR test complete: %.3f mΩ from %u repeats",
                 resistance_mohm, this->valid_repeats_);
        return {ProcedureStatus::COMPLETE, 0.0f, Fault::NONE,
                ChargerCommand::DISABLE};
      }

      this->reset_baseline_samples_();
      this->last_sample_sequence_ = measurement.sequence;
      this->begin_phase_(DcrPhase::BASELINE_SETTLE);
      return this->running_(this->baseline_current_a_);

    case DcrPhase::IDLE:
    default:
      return this->failed_();
  }
}

void DcrTest::stop(StopReason reason) {
  if (reason != StopReason::COMPLETED) {
    ESP_LOGW(DCR_TAG, "DCR test stopped before completion");
  }
  this->phase_ = DcrPhase::IDLE;
  this->reset_baseline_samples_();
  this->reset_pulse_samples_();
}

void DcrTest::begin_phase_(DcrPhase phase) {
  this->phase_ = phase;
  this->phase_started_ms_ = millis();
}

void DcrTest::reset_baseline_samples_() {
  this->baseline_voltage_sum_ = 0.0;
  this->baseline_current_sum_ = 0.0;
  this->baseline_sample_count_ = 0;
}

void DcrTest::reset_pulse_samples_() {
  this->pulse_voltage_sum_ = 0.0;
  this->pulse_current_sum_ = 0.0;
  this->pulse_sample_count_ = 0;
}

void DcrTest::accumulate_baseline_(const Measurement &measurement) {
  if (measurement.sequence == this->last_sample_sequence_) {
    return;
  }
  this->last_sample_sequence_ = measurement.sequence;
  this->baseline_voltage_sum_ += measurement.voltage_v;
  this->baseline_current_sum_ += measurement.current_a;
  this->baseline_sample_count_++;
}

void DcrTest::accumulate_pulse_(const Measurement &measurement) {
  if (measurement.sequence == this->last_sample_sequence_) {
    return;
  }
  this->last_sample_sequence_ = measurement.sequence;
  this->pulse_voltage_sum_ += measurement.voltage_v;
  this->pulse_current_sum_ += measurement.current_a;
  this->pulse_sample_count_++;
}

bool DcrTest::finish_repeat_() {
  if (this->baseline_sample_count_ == 0 || this->pulse_sample_count_ == 0) {
    return false;
  }

  const double baseline_voltage =
      this->baseline_voltage_sum_ /
      static_cast<double>(this->baseline_sample_count_);
  const double baseline_current =
      this->baseline_current_sum_ /
      static_cast<double>(this->baseline_sample_count_);
  const double pulse_voltage =
      this->pulse_voltage_sum_ /
      static_cast<double>(this->pulse_sample_count_);
  const double pulse_current =
      this->pulse_current_sum_ /
      static_cast<double>(this->pulse_sample_count_);

  const double delta_current = pulse_current - baseline_current;
  const double delta_voltage = baseline_voltage - pulse_voltage;

  if (!std::isfinite(delta_current) || !std::isfinite(delta_voltage) ||
      std::fabs(delta_current) < MINIMUM_CURRENT_DELTA_A) {
    return false;
  }

  const double resistance_ohm = delta_voltage / delta_current;
  if (!std::isfinite(resistance_ohm) || resistance_ohm <= 0.0) {
    return false;
  }

  this->resistance_sum_ohm_ += resistance_ohm;
  this->valid_repeats_++;
  return true;
}

ProcedureResult DcrTest::running_(float requested_current_a) const {
  return {ProcedureStatus::RUNNING, requested_current_a, Fault::NONE,
          ChargerCommand::DISABLE};
}

ProcedureResult DcrTest::failed_() const {
  return {ProcedureStatus::FAILED, 0.0f, Fault::PROCEDURE_ERROR,
          ChargerCommand::DISABLE};
}

void DcrStartButton::press_action() {
  if (this->host_ != nullptr && this->procedure_ != nullptr) {
    this->host_->start_procedure(this->procedure_);
  }
}

}  // namespace programmable_load
}  // namespace esphome
