#pragma once

#include <array>
#include <cstdint>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include "bq76952_config.h"
#include "bq76952_protocol.h"
#include "bq76952_service.h"

namespace esphome {
namespace bq76952 {

// ESPHome-facing facade only. Device protocol, desired-state synchronization,
// and ancillary SoC estimation live behind BQ76952Service. Method bodies live
// in implementation files, not in this header.
class BQ76952Component : public PollingComponent, public BQ76952Protocol {
 public:
  BQ76952Component();

  void set_config(const BQ76952Config &config);

  void set_bat_voltage_sensor(sensor::Sensor *sensor);
  void set_pack_voltage_sensor(sensor::Sensor *sensor);
  void set_ld_voltage_sensor(sensor::Sensor *sensor);
  void set_largest_intercell_voltage_sensor(sensor::Sensor *sensor);
  void set_cell_voltage_sensor(uint8_t logical_cell, sensor::Sensor *sensor);
  void set_current_sensor(sensor::Sensor *sensor);
  void set_state_of_charge_sensor(sensor::Sensor *sensor);
  void set_learned_capacity_sensor(sensor::Sensor *sensor);
  void set_die_temperature_sensor(sensor::Sensor *sensor);
  void set_ts1_temperature_sensor(sensor::Sensor *sensor);
  void set_ts2_temperature_sensor(sensor::Sensor *sensor);
  void set_ts3_temperature_sensor(sensor::Sensor *sensor);

  void set_state_sensor(text_sensor::TextSensor *sensor);
  void set_fault_sensor(text_sensor::TextSensor *sensor);
  void set_capacity_calibration_status_sensor(text_sensor::TextSensor *sensor);

  void set_output_enabled_switch(switch_::Switch *control);

  void setup() override;
  void update() override;
  void dump_config() override;

  bool set_output_enabled(bool enabled);
  bool clear_alarm_latches();
  bool program_factory_otp();

 private:
  void publish_snapshot(const BQ76952Snapshot &snapshot);
  void publish_faults(const BQ76952Snapshot &snapshot);

  BQ76952Service service_;

  sensor::Sensor *bat_voltage_sensor_{nullptr};
  sensor::Sensor *pack_voltage_sensor_{nullptr};
  sensor::Sensor *ld_voltage_sensor_{nullptr};
  sensor::Sensor *largest_intercell_voltage_sensor_{nullptr};
  std::array<sensor::Sensor *, 16> cell_voltage_sensors_{};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *state_of_charge_sensor_{nullptr};
  sensor::Sensor *learned_capacity_sensor_{nullptr};
  sensor::Sensor *die_temperature_sensor_{nullptr};
  std::array<sensor::Sensor *, 3> thermistor_temperature_sensors_{};

  text_sensor::TextSensor *state_sensor_{nullptr};
  text_sensor::TextSensor *fault_sensor_{nullptr};
  text_sensor::TextSensor *capacity_calibration_status_sensor_{nullptr};

  switch_::Switch *output_enabled_switch_{nullptr};
};

}  // namespace bq76952
}  // namespace esphome

#include "bq76952_controls.h"
