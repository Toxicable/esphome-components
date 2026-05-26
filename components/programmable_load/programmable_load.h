#pragma once

#include <limits>
#include <vector>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace programmable_load {

enum class RampState : int8_t {
  OFF = 0,
  RAMPING_UP = 1,
  RAMPING_DOWN = 2,
  HOLDING = 3,
};

// Circular buffer of DCR samples taken during ramping.
static const int DCR_SAMPLE_MAX = 64;

struct DcrSample {
  float current_a;
  float voltage_v;
};

class ProgrammableLoadComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;

  // --- External references (hardware wiring) ---
  void set_dac_output(output::FloatOutput *out) { dac_output_ = out; }
  void set_fan_output(output::FloatOutput *out) { fan_output_ = out; }
  void set_current_sensor(sensor::Sensor *s) { current_sensor_ = s; }
  void set_voltage_sensor(sensor::Sensor *s) { voltage_sensor_ = s; }
  void add_temperature_sensor(sensor::Sensor *s) { temperature_sensors_.push_back(s); }
  void add_ntc_present_sensor(binary_sensor::BinarySensor *s) { ntc_present_sensors_.push_back(s); }

  // --- Tunables ---
  void set_max_current_a(float v) { max_current_a_ = v; }
  void set_voltage_min_v(float v) { voltage_min_v_ = v; }
  void set_max_temp_c(float v) { max_temp_c_ = v; }
  void set_control_period_ms(uint32_t ms) { control_period_ms_ = ms; }
  void set_deadband_a(float v) { deadband_a_ = v; }
  void set_max_unconfirmed_rise_a(float v) { max_unconfirmed_rise_a_ = v; }
  void set_max_unconfirmed_fall_a(float v) { max_unconfirmed_fall_a_ = v; }
  void set_ramp_fast_a_per_s(float v) { ramp_fast_a_per_s_ = v; }
  void set_ramp_medium_a_per_s(float v) { ramp_medium_a_per_s_ = v; }
  void set_fan_start_temp_c(float v) { fan_start_temp_c_ = v; }
  void set_fan_full_temp_c(float v) { fan_full_temp_c_ = v; }

  // --- Control API ---
  void set_target(float amps);
  void force_off();
  float get_target() const { return current_target_a_; }
  float get_command() const { return current_command_a_; }
  RampState get_ramp_state() const { return ramp_state_; }

  // --- Generated entity setters ---
  void set_setpoint_number(number::Number *n) { setpoint_number_ = n; }
  void set_dcr_sensor(sensor::Sensor *s) { dcr_sensor_ = s; }
  void set_voltage_drop_sensor(sensor::Sensor *s) { voltage_drop_sensor_ = s; }
  void set_current_delta_sensor(sensor::Sensor *s) { current_delta_sensor_ = s; }
  void set_ramp_state_sensor(text_sensor::TextSensor *s) { ramp_state_sensor_ = s; }
  void set_fault_ntc_missing_sensor(binary_sensor::BinarySensor *s) { fault_ntc_missing_sensor_ = s; }
  void set_fault_no_voltage_sensor(binary_sensor::BinarySensor *s) { fault_no_voltage_sensor_ = s; }
  void set_fault_over_temp_sensor(binary_sensor::BinarySensor *s) { fault_over_temp_sensor_ = s; }

 protected:
  // --- Core loop (runs at control_period_ms) ---
  void control_loop_();

  // --- Slow update (runs at 500ms) ---
  void slow_update_();

  // --- Safety ---
  bool check_safety_();
  void clear_faults_();

  // --- DCR tracking ---
  void capture_dcr_sample_();
  void compute_dcr_();
  void reset_dcr_();

  // --- Fan ---
  void update_fan_();

  // --- State publishing ---
  void publish_state_();

  // --- Helpers ---
  float clamp_command_(float cmd) const;
  float get_max_temp_() const;
  const char *ramp_state_to_string_(RampState state) const;

  // --- State ---
  float current_target_a_{0.0f};
  float current_command_a_{0.0f};
  float unconfirmed_rise_a_{0.0f};
  float unconfirmed_fall_a_{0.0f};
  float last_confirmed_current_a_{std::numeric_limits<float>::quiet_NaN()};

  // DCR: baseline captured when setpoint is set, samples taken during ramp.
  float dcr_start_voltage_v_{std::numeric_limits<float>::quiet_NaN()};
  float dcr_start_current_a_{std::numeric_limits<float>::quiet_NaN()};
  DcrSample dcr_samples_[DCR_SAMPLE_MAX];
  int dcr_sample_count_{0};

  // Published DCR values.
  float dcr_delta_voltage_mv_{std::numeric_limits<float>::quiet_NaN()};
  float dcr_delta_current_a_{std::numeric_limits<float>::quiet_NaN()};
  float dcr_mohm_{std::numeric_limits<float>::quiet_NaN()};

  bool fault_ntc_missing_{false};
  bool fault_no_voltage_{false};
  bool fault_over_temp_{false};
  RampState ramp_state_{RampState::OFF};

  // --- Tunable config ---
  float max_current_a_{40.0f};
  float voltage_min_v_{1.0f};
  float max_temp_c_{100.0f};
  uint32_t control_period_ms_{50};
  float deadband_a_{0.010f};
  float max_unconfirmed_rise_a_{1.000f};
  float max_unconfirmed_fall_a_{2.000f};
  float ramp_fast_a_per_s_{8.0f};
  float ramp_medium_a_per_s_{4.0f};
  float fan_start_temp_c_{35.0f};
  float fan_full_temp_c_{65.0f};

  // --- External references ---
  output::FloatOutput *dac_output_{nullptr};
  output::FloatOutput *fan_output_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  std::vector<sensor::Sensor *> temperature_sensors_;
  std::vector<binary_sensor::BinarySensor *> ntc_present_sensors_;

  // --- Generated entity pointers ---
  number::Number *setpoint_number_{nullptr};
  sensor::Sensor *dcr_sensor_{nullptr};
  sensor::Sensor *voltage_drop_sensor_{nullptr};
  sensor::Sensor *current_delta_sensor_{nullptr};
  text_sensor::TextSensor *ramp_state_sensor_{nullptr};
  binary_sensor::BinarySensor *fault_ntc_missing_sensor_{nullptr};
  binary_sensor::BinarySensor *fault_no_voltage_sensor_{nullptr};
  binary_sensor::BinarySensor *fault_over_temp_sensor_{nullptr};

  // --- Logging ---
  uint32_t log_divider_{0};
};


class ProgrammableLoadSetpointNumber : public number::Number {
 public:
  void set_parent(ProgrammableLoadComponent* parent) {
    parent_ = parent;
  }

 protected:
  void control(float value) override;
  ProgrammableLoadComponent* parent_{nullptr};
};
}  // namespace programmable_load
}  // namespace esphome
