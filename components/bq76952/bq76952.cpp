#include "bq76952.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char *const TAG = "bq76952";
}  // namespace

BQ76952Component::BQ76952Component() : service_(*this) {}

void BQ76952Component::set_config(const BQ76952Config &config) {
  this->service_.set_config(config);
}

void BQ76952Component::set_bat_voltage_sensor(sensor::Sensor *sensor) {
  this->bat_voltage_sensor_ = sensor;
}

void BQ76952Component::set_pack_voltage_sensor(sensor::Sensor *sensor) {
  this->pack_voltage_sensor_ = sensor;
}

void BQ76952Component::set_ld_voltage_sensor(sensor::Sensor *sensor) {
  this->ld_voltage_sensor_ = sensor;
}

void BQ76952Component::set_largest_intercell_voltage_sensor(sensor::Sensor *sensor) {
  this->largest_intercell_voltage_sensor_ = sensor;
}

void BQ76952Component::set_cell_voltage_sensor(uint8_t logical_cell, sensor::Sensor *sensor) {
  if (logical_cell == 0 || logical_cell > this->cell_voltage_sensors_.size()) {
    return;
  }
  this->cell_voltage_sensors_[logical_cell - 1] = sensor;
}

void BQ76952Component::set_current_sensor(sensor::Sensor *sensor) {
  this->current_sensor_ = sensor;
}

void BQ76952Component::set_state_of_charge_sensor(sensor::Sensor *sensor) {
  this->state_of_charge_sensor_ = sensor;
}

void BQ76952Component::set_learned_capacity_sensor(sensor::Sensor *sensor) {
  this->learned_capacity_sensor_ = sensor;
}

void BQ76952Component::set_die_temperature_sensor(sensor::Sensor *sensor) {
  this->die_temperature_sensor_ = sensor;
}

void BQ76952Component::set_ts1_temperature_sensor(sensor::Sensor *sensor) {
  this->thermistor_temperature_sensors_[0] = sensor;
}

void BQ76952Component::set_ts2_temperature_sensor(sensor::Sensor *sensor) {
  this->thermistor_temperature_sensors_[1] = sensor;
}

void BQ76952Component::set_ts3_temperature_sensor(sensor::Sensor *sensor) {
  this->thermistor_temperature_sensors_[2] = sensor;
}

void BQ76952Component::set_connection_state_sensor(text_sensor::TextSensor *sensor) {
  this->connection_state_sensor_ = sensor;
}

void BQ76952Component::set_state_sensor(text_sensor::TextSensor *sensor) {
  this->state_sensor_ = sensor;
}

void BQ76952Component::set_fault_sensor(text_sensor::TextSensor *sensor) {
  this->fault_sensor_ = sensor;
}


void BQ76952Component::set_capacity_calibration_status_sensor(text_sensor::TextSensor *sensor) {
  this->capacity_calibration_status_sensor_ = sensor;
}

void BQ76952Component::set_output_enabled_switch(switch_::Switch *control) {
  this->output_enabled_switch_ = control;
}

void BQ76952Component::setup() {
  this->service_.setup();
}

void BQ76952Component::update() {
  ::bq76952_core::Snapshot snapshot{};
  const bool valid = this->service_.poll(snapshot);
  this->publish_connection_state(snapshot.connection_state);

  if (!valid) {
    if (this->state_sensor_ != nullptr) {
      this->state_sensor_->publish_state("unknown");
    }
    if (this->fault_sensor_ != nullptr) {
      this->fault_sensor_->publish_state("unknown");
    }
    this->status_set_warning();
    return;
  }

  this->publish_snapshot(snapshot);
  if (snapshot.configuration_ready) {
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

void BQ76952Component::publish_connection_state(component_common::ConnectionState connection_state) {
  if (this->connection_state_sensor_ != nullptr) {
    this->connection_state_sensor_->publish_state(
        component_common::connection_state_to_string(connection_state));
  }
}

void BQ76952Component::publish_snapshot(const ::bq76952_core::Snapshot &snapshot) {
  if (this->state_sensor_ != nullptr) {
    this->state_sensor_->publish_state(::bq76952_core::operating_state_to_string(snapshot.operating_state));
  }
  this->publish_faults(snapshot);
  if (this->capacity_calibration_status_sensor_ != nullptr) {
    this->capacity_calibration_status_sensor_->publish_state(this->service_.capacity_calibration_status());
  }

  if (this->output_enabled_switch_ != nullptr) {
    this->output_enabled_switch_->publish_state(snapshot.output_enabled);
  }
  if (this->bat_voltage_sensor_ != nullptr) {
    this->bat_voltage_sensor_->publish_state(static_cast<float>(snapshot.stack_voltage_mv) / 1000.0F);
  }
  if (this->pack_voltage_sensor_ != nullptr) {
    this->pack_voltage_sensor_->publish_state(static_cast<float>(snapshot.pack_voltage_mv) / 1000.0F);
  }
  if (this->ld_voltage_sensor_ != nullptr) {
    this->ld_voltage_sensor_->publish_state(static_cast<float>(snapshot.load_detect_voltage_mv) / 1000.0F);
  }

  int16_t minimum_cell_mv = 0;
  int16_t maximum_cell_mv = 0;
  if (snapshot.cell_count > 0) {
    minimum_cell_mv = snapshot.cell_voltage_mv[0];
    maximum_cell_mv = snapshot.cell_voltage_mv[0];
  }
  for (uint8_t i = 0; i < snapshot.cell_count; i++) {
    if (this->cell_voltage_sensors_[i] != nullptr) {
      this->cell_voltage_sensors_[i]->publish_state(static_cast<float>(snapshot.cell_voltage_mv[i]) / 1000.0F);
    }
    minimum_cell_mv = std::min(minimum_cell_mv, snapshot.cell_voltage_mv[i]);
    maximum_cell_mv = std::max(maximum_cell_mv, snapshot.cell_voltage_mv[i]);
  }
  if (this->largest_intercell_voltage_sensor_ != nullptr && snapshot.cell_count >= 2) {
    this->largest_intercell_voltage_sensor_->publish_state(
        static_cast<float>(maximum_cell_mv - minimum_cell_mv) / 1000.0F);
  }

  if (this->current_sensor_ != nullptr) {
    this->current_sensor_->publish_state(snapshot.current_a);
  }
  if (this->state_of_charge_sensor_ != nullptr) {
    this->state_of_charge_sensor_->publish_state(snapshot.state_of_charge_percent);
  }
  if (this->learned_capacity_sensor_ != nullptr && std::isfinite(snapshot.learned_capacity_ah)) {
    this->learned_capacity_sensor_->publish_state(snapshot.learned_capacity_ah);
  }
  if (this->die_temperature_sensor_ != nullptr) {
    this->die_temperature_sensor_->publish_state(snapshot.die_temperature_c);
  }
  for (size_t i = 0; i < this->thermistor_temperature_sensors_.size(); i++) {
    if (this->thermistor_temperature_sensors_[i] != nullptr &&
        std::isfinite(snapshot.thermistor_temperature_c[i])) {
      this->thermistor_temperature_sensors_[i]->publish_state(snapshot.thermistor_temperature_c[i]);
    }
  }
}

void BQ76952Component::publish_faults(const ::bq76952_core::Snapshot &snapshot) {
  if (this->fault_sensor_ != nullptr) {
    char faults[256];
    ::bq76952_core::format_faults(snapshot.active_faults, faults, sizeof(faults));
    this->fault_sensor_->publish_state(faults);
  }
}

void BQ76952Component::dump_config() {
  const auto &config = this->service_.config();
  ESP_LOGCONFIG(TAG, "BQ76952:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Cells: %u (VC1..VC%u, VC16)", static_cast<unsigned>(config.cell_count),
                static_cast<unsigned>(config.cell_count - 1));
  ESP_LOGCONFIG(TAG, "  Sense resistor: %.3f mOhm", config.sense_resistor_milliohm);
  ESP_LOGCONFIG(TAG, "  I2C CRC: %s", YESNO(config.i2c_crc_enabled));
  ESP_LOGCONFIG(TAG, "  Autonomous FET control: %s", YESNO(config.fet.autonomous));
  ESP_LOGCONFIG(TAG, "  Precharge: %s", YESNO(config.fet.precharge.enabled));
  ESP_LOGCONFIG(TAG, "  Predischarge: %s", YESNO(config.fet.predischarge.enabled));
  ESP_LOGCONFIG(TAG, "  SoC endpoints: empty=%u mV full=%u mV",
                static_cast<unsigned>(config.soc.empty_cell_voltage_mv),
                static_cast<unsigned>(config.soc.full_cell_voltage_mv));

  LOG_SENSOR("  ", "BAT Voltage", this->bat_voltage_sensor_);
  LOG_SENSOR("  ", "PACK Voltage", this->pack_voltage_sensor_);
  LOG_SENSOR("  ", "LD Voltage", this->ld_voltage_sensor_);
  LOG_SENSOR("  ", "Largest Inter-Cell Voltage", this->largest_intercell_voltage_sensor_);
  for (size_t i = 0; i < this->cell_voltage_sensors_.size(); i++) {
    char label[24];
    std::snprintf(label, sizeof(label), "Cell %u Voltage", static_cast<unsigned>(i + 1));
    LOG_SENSOR("  ", label, this->cell_voltage_sensors_[i]);
  }
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "State of Charge", this->state_of_charge_sensor_);
  LOG_SENSOR("  ", "Learned Capacity", this->learned_capacity_sensor_);
  LOG_SENSOR("  ", "Die Temperature", this->die_temperature_sensor_);
  LOG_SENSOR("  ", "TS1 Temperature", this->thermistor_temperature_sensors_[0]);
  LOG_SENSOR("  ", "TS2 Temperature", this->thermistor_temperature_sensors_[1]);
  LOG_SENSOR("  ", "TS3 Temperature", this->thermistor_temperature_sensors_[2]);
  LOG_TEXT_SENSOR("  ", "Connection State", this->connection_state_sensor_);
  LOG_TEXT_SENSOR("  ", "State", this->state_sensor_);
  LOG_TEXT_SENSOR("  ", "Fault", this->fault_sensor_);
  LOG_TEXT_SENSOR("  ", "Capacity Calibration Status", this->capacity_calibration_status_sensor_);
  LOG_SWITCH("  ", "Output Enabled", this->output_enabled_switch_);
}

bool BQ76952Component::set_output_enabled(bool enabled) {
  return this->service_.set_output_enabled(enabled);
}

bool BQ76952Component::clear_alarm_latches() {
  return this->service_.clear_alarm_latches();
}

bool BQ76952Component::program_factory_otp() {
  return this->service_.program_factory_otp();
}

void BQ76952OutputEnabledSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && this->parent_->set_output_enabled(state)) {
    this->publish_state(state);
  }
}

void BQ76952ClearAlarmsButton::press_action() {
  if (this->parent_ == nullptr || !this->parent_->clear_alarm_latches()) {
    ESP_LOGW(TAG, "Failed clearing BQ76952 alarm latches");
  }
}

void BQ76952ProgramFactoryOtpButton::press_action() {
  ESP_LOGE(TAG, "DANGER: OTP programming requested; this operation is one-time and irreversible");
  if (this->parent_ == nullptr || !this->parent_->program_factory_otp()) {
    ESP_LOGE(TAG, "BQ76952 OTP programming failed or was rejected");
  }
}

}  // namespace bq76952
}  // namespace esphome
