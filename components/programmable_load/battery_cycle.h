#pragma once

#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include "procedure.h"

namespace esphome {
namespace programmable_load {

class ProgrammableLoadComponent;

enum class BatteryCyclePhase : uint8_t {
  IDLE = 0,
  DISCHARGING,
  RESTING,
  CHARGE_STARTING,
  CHARGING,
  TERMINATION_HOLD,
};

class BatteryCycle : public Procedure {
 public:
  const char *name() const override { return "battery_cycle"; }

  void set_discharge_current(float current_a) {
    this->discharge_current_a_ = current_a;
  }
  void set_discharge_cutoff_voltage(float voltage_v) {
    this->discharge_cutoff_voltage_v_ = voltage_v;
  }
  void set_discharge_cutoff_hysteresis(float voltage_v) {
    this->discharge_cutoff_hysteresis_v_ = voltage_v;
  }
  void set_discharge_cutoff_hold_time_ms(uint32_t time_ms) {
    this->discharge_cutoff_hold_time_ms_ = time_ms;
  }
  void set_rest_time_ms(uint32_t time_ms) { this->rest_time_ms_ = time_ms; }
  void set_charge_start_timeout_ms(uint32_t time_ms) {
    this->charge_start_timeout_ms_ = time_ms;
  }
  void set_charge_stall_timeout_ms(uint32_t time_ms) {
    this->charge_stall_timeout_ms_ = time_ms;
  }
  void set_charge_timeout_ms(uint32_t time_ms) {
    this->charge_timeout_ms_ = time_ms;
  }
  void set_termination_hold_time_ms(uint32_t time_ms) {
    this->termination_hold_time_ms_ = time_ms;
  }

  void set_phase_sensor(text_sensor::TextSensor *sensor) {
    this->phase_sensor_ = sensor;
  }
  void set_result_sensor(text_sensor::TextSensor *sensor) {
    this->result_sensor_ = sensor;
  }
  void set_discharged_capacity_sensor(sensor::Sensor *sensor) {
    this->discharged_capacity_sensor_ = sensor;
  }
  void set_discharged_energy_sensor(sensor::Sensor *sensor) {
    this->discharged_energy_sensor_ = sensor;
  }
  void set_charged_capacity_sensor(sensor::Sensor *sensor) {
    this->charged_capacity_sensor_ = sensor;
  }
  void set_charged_energy_sensor(sensor::Sensor *sensor) {
    this->charged_energy_sensor_ = sensor;
  }

  ProcedureResult start(const ProcedureContext &context) override;
  ProcedureResult update(const ProcedureContext &context) override;
  void stop(StopReason reason) override;

 protected:
  static bool charge_state_active_(ChargerState state);
  void set_phase_(BatteryCyclePhase phase);
  const char *phase_to_string_() const;
  void reset_integrators_();
  void integrate_discharge_(const Measurement &measurement);
  void integrate_charge_(const ChargerMeasurement &measurement);
  void publish_results_();
  void publish_result_(const char *result);
  ProcedureResult running_(float current_a, ChargerCommand command) const;
  ProcedureResult failed_(Fault fault) const;
  ProcedureResult complete_();

  text_sensor::TextSensor *phase_sensor_{nullptr};
  text_sensor::TextSensor *result_sensor_{nullptr};
  sensor::Sensor *discharged_capacity_sensor_{nullptr};
  sensor::Sensor *discharged_energy_sensor_{nullptr};
  sensor::Sensor *charged_capacity_sensor_{nullptr};
  sensor::Sensor *charged_energy_sensor_{nullptr};

  BatteryCyclePhase phase_{BatteryCyclePhase::IDLE};
  float discharge_current_a_{0.0f};
  float discharge_cutoff_voltage_v_{0.0f};
  float discharge_cutoff_hysteresis_v_{0.1f};
  uint32_t discharge_cutoff_hold_time_ms_{2000};
  uint32_t rest_time_ms_{5000};
  uint32_t charge_start_timeout_ms_{30000};
  uint32_t charge_stall_timeout_ms_{30000};
  uint32_t charge_timeout_ms_{24UL * 60UL * 60UL * 1000UL};
  uint32_t termination_hold_time_ms_{2000};

  uint32_t phase_started_ms_{0};
  uint32_t cutoff_started_ms_{0};
  uint32_t charge_started_ms_{0};
  uint32_t charge_stall_started_ms_{0};
  uint32_t termination_started_ms_{0};
  bool charge_activity_seen_{false};
  bool completed_{false};

  uint32_t last_load_sequence_{0};
  uint32_t last_load_timestamp_ms_{0};
  float last_load_current_a_{0.0f};
  float last_load_power_w_{0.0f};
  bool have_last_load_sample_{false};

  uint32_t last_charger_sequence_{0};
  uint32_t last_charger_timestamp_ms_{0};
  float last_charger_current_a_{0.0f};
  float last_charger_power_w_{0.0f};
  bool have_last_charger_sample_{false};

  double discharged_capacity_ah_{0.0};
  double discharged_energy_wh_{0.0};
  double charged_capacity_ah_{0.0};
  double charged_energy_wh_{0.0};
};

class BatteryCycleStartButton : public button::Button {
 public:
  void set_host(ProgrammableLoadComponent *host) { this->host_ = host; }
  void set_procedure(BatteryCycle *procedure) { this->procedure_ = procedure; }

 protected:
  void press_action() override;
  ProgrammableLoadComponent *host_{nullptr};
  BatteryCycle *procedure_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome
