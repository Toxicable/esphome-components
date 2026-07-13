#pragma once

#include <vector>

#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include "load_procedure.h"
#include "load_types.h"

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

  // Hardware boundary.
  void set_dac_output(output::FloatOutput *output) { this->dac_output_ = output; }
  void set_dac_full_scale_current(float current_a) {
    this->dac_full_scale_current_a_ = current_a;
  }
  void set_fan_output(output::FloatOutput *output) { this->fan_output_ = output; }

  // Measurement inputs. The component owns no measurement entities and only
  // consumes the values required to control and protect the load.
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void add_temperature_sensor(sensor::Sensor *sensor, bool required) {
    this->temperature_inputs_.push_back({sensor, required});
  }
  void set_sample_timeout_ms(uint32_t timeout_ms) {
    this->sample_timeout_ms_ = timeout_ms;
  }

  // Absolute operating limits.
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

  // Cooling policy.
  void set_fan_temperature_range(float start_c, float full_c) {
    this->fan_start_temperature_c_ = start_c;
    this->fan_full_temperature_c_ = full_c;
  }

  // Fault policy. A fault always stops the active operation. Auto-clear only
  // controls whether the FAULT state returns to IDLE after the fault condition
  // has remained absent for the configured delay.
  void set_fault_auto_clear(bool auto_clear) {
    this->fault_policy_.auto_clear = auto_clear;
  }
  void set_fault_clear_delay_ms(uint32_t delay_ms) {
    this->fault_policy_.clear_delay_ms = delay_ms;
  }

  // User-facing entities kept intentionally small.
  void set_manual_current_number(number::Number *number) {
    this->manual_current_number_ = number;
  }
  void set_state_sensor(text_sensor::TextSensor *sensor) {
    this->state_sensor_ = sensor;
  }
  void set_fault_sensor(text_sensor::TextSensor *sensor) {
    this->fault_sensor_ = sensor;
  }

  // Manual operation.
  bool set_manual_current(float current_a);
  void stop();
  bool pause();
  bool resume();

  // Fault API.
  bool clear_fault();
  LoadFault fault() const { return this->fault_; }
  LoadState state() const { return this->state_; }

  // Procedure API. Exactly one procedure or manual operation may own the load.
  void add_procedure(LoadProcedure *procedure);
  bool start_procedure(LoadProcedure *procedure);
  bool request_procedure_current(LoadProcedure *procedure, float current_a);
  void finish_procedure(LoadProcedure *procedure);
  void fail_procedure(LoadProcedure *procedure, LoadFault fault);

  const LoadMeasurement &measurement() const { return this->measurement_; }
  const LoadLimits &limits() const { return this->limits_; }

 protected:
  void update_measurement_();
  void update_faults_();
  void update_operation_();
  void update_control_();
  void update_fan_();

  void set_state_(LoadState state);
  void trip_fault_(LoadFault fault);
  void publish_status_();
  float effective_current_limit_() const;

  output::FloatOutput *dac_output_{nullptr};
  output::FloatOutput *fan_output_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  std::vector<TemperatureInput> temperature_inputs_;

  number::Number *manual_current_number_{nullptr};
  text_sensor::TextSensor *state_sensor_{nullptr};
  text_sensor::TextSensor *fault_sensor_{nullptr};

  std::vector<LoadProcedure *> procedures_;
  LoadProcedure *active_procedure_{nullptr};

  LoadMeasurement measurement_{};
  LoadLimits limits_{};
  LoadFaultPolicy fault_policy_{};

  LoadState state_{LoadState::IDLE};
  LoadFault fault_{LoadFault::NONE};

  float manual_current_a_{0.0f};
  float requested_current_a_{0.0f};
  float commanded_current_a_{0.0f};
  float dac_full_scale_current_a_{0.0f};

  uint32_t sample_timeout_ms_{250};
  uint32_t control_period_ms_{50};
  uint32_t fault_clear_candidate_since_ms_{0};

  float deadband_a_{0.01f};
  float rise_rate_a_per_s_{2.0f};
  float fall_rate_a_per_s_{4.0f};
  float fan_start_temperature_c_{35.0f};
  float fan_full_temperature_c_{70.0f};
};

class ProgrammableLoadManualCurrentNumber : public number::Number {
 public:
  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  void control(float value) override;
  ProgrammableLoadComponent *parent_{nullptr};
};

class ProgrammableLoadClearFaultButton : public button::Button {
 public:
  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  ProgrammableLoadComponent *parent_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome

#include "procedures/dcr_test.h"
