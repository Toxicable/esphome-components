#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/sensor/sensor.h"

#include "procedure.h"

namespace esphome {
namespace programmable_load {

class ProgrammableLoadComponent;

enum class DcrPhase : uint8_t {
  IDLE = 0,
  BASELINE_SETTLE,
  BASELINE_SAMPLE,
  PULSE_SETTLE,
  PULSE_SAMPLE,
  RECOVERY,
};

class DcrTest : public Procedure {
 public:
  const char *name() const override { return "dcr_test"; }

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

  ProcedureResult start(const Measurement &measurement) override;
  ProcedureResult update(const Measurement &measurement) override;
  void stop(StopReason reason) override;

 protected:
  void begin_phase_(DcrPhase phase);
  void accumulate_sample_(const Measurement &measurement);
  bool finish_repeat_();

  sensor::Sensor *resistance_sensor_{nullptr};

  DcrPhase phase_{DcrPhase::IDLE};
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

class DcrStartButton : public button::Button {
 public:
  void set_host(ProgrammableLoadComponent *host) { this->host_ = host; }
  void set_procedure(DcrTest *procedure) { this->procedure_ = procedure; }

 protected:
  void press_action() override;
  ProgrammableLoadComponent *host_{nullptr};
  DcrTest *procedure_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome
