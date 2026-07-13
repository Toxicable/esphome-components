#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/sensor/sensor.h"

#include "../load_procedure.h"

namespace esphome {
namespace programmable_load {

enum class DcrTestPhase : uint8_t {
  IDLE = 0,
  BASELINE_SETTLE,
  BASELINE_SAMPLE,
  PULSE_SETTLE,
  PULSE_SAMPLE,
  RECOVERY,
  COMPLETE,
};

class ProgrammableLoadDcrTest : public LoadProcedure {
 public:
  const char *procedure_name() const override { return "dcr_test"; }

  void set_baseline_current(float current_a) {
    this->baseline_current_a_ = current_a;
  }
  void set_pulse_current(float current_a) { this->pulse_current_a_ = current_a; }
  void set_timing(uint32_t settle_ms, uint32_t sample_ms, uint32_t recovery_ms) {
    this->settle_time_ms_ = settle_ms;
    this->sample_time_ms_ = sample_ms;
    this->recovery_time_ms_ = recovery_ms;
  }
  void set_repeats(uint8_t repeats) { this->repeats_ = repeats; }
  void set_resistance_sensor(sensor::Sensor *sensor) {
    this->resistance_sensor_ = sensor;
  }

  // Called by the user-facing start button. The procedure still has to acquire
  // the parent operation host, so it cannot start while manual control or
  // another procedure owns the load.
  bool request_start();

  bool start(const LoadMeasurement &measurement) override;
  void update(const LoadMeasurement &measurement) override;
  void stop(LoadStopReason reason) override;

  float requested_current_a() const override;
  bool is_complete() const override {
    return this->phase_ == DcrTestPhase::COMPLETE;
  }

 protected:
  void begin_phase_(DcrTestPhase phase);
  void accumulate_sample_(const LoadMeasurement &measurement);
  bool finish_repeat_();

  sensor::Sensor *resistance_sensor_{nullptr};

  DcrTestPhase phase_{DcrTestPhase::IDLE};
  float baseline_current_a_{0.0f};
  float pulse_current_a_{0.0f};

  uint32_t settle_time_ms_{100};
  uint32_t sample_time_ms_{500};
  uint32_t recovery_time_ms_{1000};
  uint32_t phase_started_ms_{0};

  uint8_t repeats_{3};
  uint8_t completed_repeats_{0};

  double baseline_voltage_sum_{0.0};
  double baseline_current_sum_{0.0};
  double pulse_voltage_sum_{0.0};
  double pulse_current_sum_{0.0};
  uint32_t baseline_sample_count_{0};
  uint32_t pulse_sample_count_{0};
};

class ProgrammableLoadDcrStartButton : public button::Button {
 public:
  void set_parent(ProgrammableLoadDcrTest *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  ProgrammableLoadDcrTest *parent_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome
