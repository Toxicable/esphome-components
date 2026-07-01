#include "husb238.h"

#include <cstdio>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace husb238 {

static const char *const TAG = "husb238";

static constexpr uint32_t BOOT_REQUEST_DELAY_MS = 3000;

HUSB238Component::HUSB238Component() : service_(this) {}

void HUSB238Component::setup() {
  if (!this->service_.probe()) {
    ESP_LOGE(TAG, "HUSB238 not found");
    this->mark_failed();
    return;
  }

  if (this->request_on_boot_ && this->initial_request_voltage_ != 0) {
    this->boot_request_pending_ = true;
    this->boot_request_due_ms_ = millis() + BOOT_REQUEST_DELAY_MS;
    ESP_LOGI(
      TAG, "Deferring %uV boot PDO request for %ums to avoid renegotiating during ESP startup",
      this->initial_request_voltage_, BOOT_REQUEST_DELAY_MS
    );
  }
}

void HUSB238Component::dump_config() {
  ESP_LOGCONFIG(TAG, "HUSB238:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Request on boot: %s", YESNO(this->request_on_boot_));
  if (this->initial_request_voltage_ != 0) {
    ESP_LOGCONFIG(TAG, "  Initial request voltage: %uV", this->initial_request_voltage_);
  }
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Power", this->power_sensor_);
  LOG_BINARY_SENSOR("  ", "Attached", this->attached_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "CC2 Connected", this->cc2_connected_binary_sensor_);
  LOG_TEXT_SENSOR("  ", "PD Response", this->pd_response_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Available PDOs", this->available_pdos_text_sensor_);
}

void HUSB238Component::update() {
  ::husb238_core::Status status;
  if (!this->service_.read_status(&status)) {
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();

  this->maybe_run_boot_request_(status.attached);

  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(status.voltage);
  if (this->current_sensor_ != nullptr)
    this->current_sensor_->publish_state(status.current);
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(status.power);
  if (this->attached_binary_sensor_ != nullptr)
    this->attached_binary_sensor_->publish_state(status.attached);
  if (this->cc2_connected_binary_sensor_ != nullptr)
    this->cc2_connected_binary_sensor_->publish_state(status.attached && status.cc2_connected);
  if (this->pd_response_text_sensor_ != nullptr)
    this->pd_response_text_sensor_->publish_state(::husb238_core::pd_response_to_string(status.pd_response));

  ::husb238_core::SourcePdo pdos[6] = {};
  const bool pdo_ok = this->service_.read_source_pdos(pdos, 6);
  if (pdo_ok && this->available_pdos_text_sensor_ != nullptr) {
    this->available_pdos_text_sensor_->publish_state(this->build_available_pdos_string_(pdos));
  }

  if (status.voltage != 0) {
    this->publish_voltage_select_(status.voltage);
  } else if (this->service_.last_requested_voltage() != 0) {
    this->publish_voltage_select_(this->service_.last_requested_voltage());
  }
}

bool HUSB238Component::request_voltage(uint8_t voltage) {
  if (::husb238_core::pdo_select_code(voltage) == 0) {
    ESP_LOGW(TAG, "Unsupported voltage request: %uV", voltage);
    return false;
  }

  if (!this->service_.request_voltage(voltage))
    return false;

  this->publish_voltage_select_(voltage);
  ESP_LOGI(TAG, "Requested %uV PDO", voltage);
  return true;
}

bool HUSB238Component::request_source_capabilities() {
  ESP_LOGD(TAG, "Requesting source capabilities");
  return this->service_.request_source_capabilities();
}

bool HUSB238Component::hard_reset() {
  ESP_LOGW(TAG, "Sending USB-PD hard reset");
  return this->service_.hard_reset();
}

void HUSB238Component::maybe_run_boot_request_(bool attached) {
  if (!this->boot_request_pending_)
    return;

  if (!attached)
    return;

  const uint32_t now = millis();
  if (now < this->boot_request_due_ms_)
    return;

  if (!this->request_voltage(this->initial_request_voltage_))
    return;

  this->boot_request_pending_ = false;
}

bool HUSB238Component::read_register(uint8_t reg, uint8_t *value) {
  if (!this->read_byte(reg, value)) {
    ESP_LOGW(TAG, "I2C read failed at register 0x%02X", reg);
    return false;
  }
  return true;
}

bool HUSB238Component::write_register(uint8_t reg, uint8_t value) {
  if (!this->write_byte(reg, value)) {
    ESP_LOGW(TAG, "I2C write failed at register 0x%02X", reg);
    return false;
  }
  return true;
}

void HUSB238Component::delay_ms(uint32_t ms) { delay(ms); }

void HUSB238Component::publish_voltage_select_(uint8_t voltage) {
  if (this->voltage_select_ == nullptr)
    return;

  char text[8];
  std::snprintf(text, sizeof(text), "%uV", voltage);
  this->voltage_select_->publish_state(text);
}

std::string HUSB238Component::build_available_pdos_string_(const ::husb238_core::SourcePdo *pdos) const {
  std::string out;

  for (uint8_t i = 0; i < 6; i++) {
    const ::husb238_core::SourcePdo &pdo = pdos[i];
    if (!pdo.available)
      continue;

    char item[24];
    std::snprintf(item, sizeof(item), "%uV %.2fA", pdo.voltage, pdo.current);

    if (!out.empty())
      out += ", ";
    out += item;
  }

  return out.empty() ? "none" : out;
}

void HUSB238VoltageSelect::control(const std::string &value) {
  if (this->parent_ == nullptr)
    return;

  uint8_t voltage = 0;
  if (value == "5V")
    voltage = 5;
  else if (value == "9V")
    voltage = 9;
  else if (value == "12V")
    voltage = 12;
  else if (value == "15V")
    voltage = 15;
  else if (value == "18V")
    voltage = 18;
  else if (value == "20V")
    voltage = 20;

  if (voltage != 0 && this->parent_->request_voltage(voltage)) {
    this->publish_state(value);
  }
}

void HUSB238HardResetButton::press_action() {
  if (this->parent_ != nullptr)
    this->parent_->hard_reset();
}

void HUSB238RefreshCapabilitiesButton::press_action() {
  if (this->parent_ != nullptr)
    this->parent_->request_source_capabilities();
}

}  // namespace husb238
}  // namespace esphome
