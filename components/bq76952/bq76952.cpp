#include "bq76952.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char *const TAG = "bq76952";

const char *state_name(BQ76952OperatingState state) {
  switch (state) {
    case BQ76952OperatingState::OFFLINE:
      return "offline";
    case BQ76952OperatingState::NORMAL:
      return "normal";
    case BQ76952OperatingState::SLEEP:
      return "sleep";
    case BQ76952OperatingState::DEEP_SLEEP:
      return "deep_sleep";
    case BQ76952OperatingState::CONFIG_UPDATE:
      return "config_update";
    case BQ76952OperatingState::SHUTDOWN_PENDING:
      return "shutdown_pending";
    default:
      return "unknown";
  }
}

void append_fault(std::string &faults, const char *name) {
  if (!faults.empty()) {
    faults += ',';
  }
  faults += name;
}
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
  BQ76952Snapshot snapshot{};
  if (!this->service_.poll(snapshot)) {
    if (this->state_sensor_ != nullptr) {
      this->state_sensor_->publish_state("offline");
    }
    this->status_set_warning();
    return;
  }

  this->publish_snapshot(snapshot);
  this->status_clear_warning();
}

void BQ76952Component::publish_snapshot(const BQ76952Snapshot &snapshot) {
  if (this->state_sensor_ != nullptr) {
    this->state_sensor_->publish_state(state_name(snapshot.state));
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

void BQ76952Component::publish_faults(const BQ76952Snapshot &snapshot) {
  if (this->fault_sensor_ == nullptr) {
    return;
  }

  std::string faults;
  const uint32_t flags = snapshot.fault_flags;
  if ((flags & BQ76952_FAULT_CELL_UNDERVOLTAGE) != 0)
    append_fault(faults, "cell_undervoltage");
  if ((flags & BQ76952_FAULT_CELL_OVERVOLTAGE) != 0)
    append_fault(faults, "cell_overvoltage");
  if ((flags & BQ76952_FAULT_CHARGE_OVERCURRENT) != 0)
    append_fault(faults, "charge_overcurrent");
  if ((flags & BQ76952_FAULT_DISCHARGE_OVERCURRENT) != 0)
    append_fault(faults, "discharge_overcurrent");
  if ((flags & BQ76952_FAULT_DISCHARGE_SEVERE_OVERCURRENT) != 0)
    append_fault(faults, "discharge_severe_overcurrent");
  if ((flags & BQ76952_FAULT_DISCHARGE_SUSTAINED_OVERCURRENT) != 0)
    append_fault(faults, "discharge_sustained_overcurrent");
  if ((flags & BQ76952_FAULT_DISCHARGE_SHORT_CIRCUIT) != 0)
    append_fault(faults, "discharge_short_circuit");
  if ((flags & BQ76952_FAULT_TEMPERATURE) != 0)
    append_fault(faults, "temperature");
  if ((flags & BQ76952_FAULT_PRECHARGE_TIMEOUT) != 0)
    append_fault(faults, "precharge_timeout");
  if ((flags & BQ76952_FAULT_PERMANENT_FAILURE) != 0)
    append_fault(faults, "permanent_failure");

  this->fault_sensor_->publish_state(faults.empty() ? "none" : faults);
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
