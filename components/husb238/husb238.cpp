#include "husb238.h"

#include <cstdio>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace husb238 {

static const char *const TAG = "husb238";

static constexpr uint8_t REG_PD_STATUS0 = 0x00;
static constexpr uint8_t REG_PD_STATUS1 = 0x01;
static constexpr uint8_t REG_SRC_PDO_5V = 0x02;
static constexpr uint8_t REG_SRC_PDO = 0x08;
static constexpr uint8_t REG_GO_COMMAND = 0x09;

static constexpr uint8_t CMD_REQUEST_SELECTED_PDO = 0x01;
static constexpr uint8_t CMD_GET_SRC_CAP = 0x04;
static constexpr uint8_t CMD_HARD_RESET = 0x10;

void HUSB238Component::setup() {
  uint8_t value;
  if (!this->read_reg_(REG_PD_STATUS0, &value)) {
    ESP_LOGE(TAG, "HUSB238 not found");
    this->mark_failed();
    return;
  }

  // Ask the chip to refresh source capabilities now that ESPHome is online.
  this->request_source_capabilities();

  if (this->request_on_boot_ && this->initial_request_voltage_ != 0) {
    this->request_voltage(this->initial_request_voltage_);
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
  uint8_t status0 = 0;
  uint8_t status1 = 0;

  if (!this->read_reg_(REG_PD_STATUS0, &status0) || !this->read_reg_(REG_PD_STATUS1, &status1)) {
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();

  const bool attached = (status1 & 0x40) != 0;
  const bool cc2_connected = (status1 & 0x80) != 0;
  const uint8_t pd_response = (status1 >> 3) & 0x07;

  uint8_t actual_voltage = this->status_voltage_to_volts_(status0 >> 4);
  float actual_current = this->current_code_to_amps_(status0 & 0x0F);

  // When there is no explicit PD contract, PD_STATUS1 reports the Type-C/legacy 5V current mode.
  if (attached && actual_voltage == 0 && ((status1 & 0x04) != 0)) {
    actual_voltage = 5;
    actual_current = this->legacy_5v_current_to_amps_(status1 & 0x03);
  }

  if (!attached) {
    actual_voltage = 0;
    actual_current = 0.0f;
  }

  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(actual_voltage);
  if (this->current_sensor_ != nullptr)
    this->current_sensor_->publish_state(actual_current);
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(actual_voltage * actual_current);
  if (this->attached_binary_sensor_ != nullptr)
    this->attached_binary_sensor_->publish_state(attached);
  if (this->cc2_connected_binary_sensor_ != nullptr)
    this->cc2_connected_binary_sensor_->publish_state(attached && cc2_connected);
  if (this->pd_response_text_sensor_ != nullptr)
    this->pd_response_text_sensor_->publish_state(this->pd_response_to_string_(pd_response));

  uint8_t pdo_regs[6] = {};
  bool pdo_ok = true;
  for (uint8_t i = 0; i < 6; i++) {
    if (!this->read_reg_(REG_SRC_PDO_5V + i, &pdo_regs[i])) {
      pdo_ok = false;
      break;
    }
  }
  if (pdo_ok && this->available_pdos_text_sensor_ != nullptr) {
    this->available_pdos_text_sensor_->publish_state(this->build_available_pdos_string_(pdo_regs));
  }

  if (actual_voltage != 0) {
    this->publish_voltage_select_(actual_voltage);
  } else if (this->last_requested_voltage_ != 0) {
    this->publish_voltage_select_(this->last_requested_voltage_);
  }
}

bool HUSB238Component::request_voltage(uint8_t voltage) {
  const uint8_t pdo_code = this->pdo_select_code_(voltage);
  if (pdo_code == 0) {
    ESP_LOGW(TAG, "Unsupported voltage request: %uV", voltage);
    return false;
  }

  if (!this->write_reg_(REG_SRC_PDO, pdo_code << 4))
    return false;

  delay(5);

  if (!this->write_reg_(REG_GO_COMMAND, CMD_REQUEST_SELECTED_PDO))
    return false;

  this->last_requested_voltage_ = voltage;
  this->publish_voltage_select_(voltage);
  ESP_LOGI(TAG, "Requested %uV PDO", voltage);
  return true;
}

bool HUSB238Component::request_source_capabilities() {
  ESP_LOGD(TAG, "Requesting source capabilities");
  return this->write_reg_(REG_GO_COMMAND, CMD_GET_SRC_CAP);
}

bool HUSB238Component::hard_reset() {
  ESP_LOGW(TAG, "Sending USB-PD hard reset");
  return this->write_reg_(REG_GO_COMMAND, CMD_HARD_RESET);
}

bool HUSB238Component::read_reg_(uint8_t reg, uint8_t *value) {
  if (!this->read_byte(reg, value)) {
    ESP_LOGW(TAG, "I2C read failed at register 0x%02X", reg);
    return false;
  }
  return true;
}

bool HUSB238Component::write_reg_(uint8_t reg, uint8_t value) {
  if (!this->write_byte(reg, value)) {
    ESP_LOGW(TAG, "I2C write failed at register 0x%02X", reg);
    return false;
  }
  return true;
}

void HUSB238Component::publish_voltage_select_(uint8_t voltage) {
  if (this->voltage_select_ == nullptr)
    return;

  char text[8];
  std::snprintf(text, sizeof(text), "%uV", voltage);
  this->voltage_select_->publish_state(text);
}

std::string HUSB238Component::build_available_pdos_string_(const uint8_t *pdo_regs) const {
  static const uint8_t voltages[] = {5, 9, 12, 15, 18, 20};
  std::string out;

  for (uint8_t i = 0; i < 6; i++) {
    const uint8_t reg = pdo_regs[i];
    if ((reg & 0x80) == 0)
      continue;

    char item[24];
    std::snprintf(item, sizeof(item), "%uV %.2fA", voltages[i], this->current_code_to_amps_(reg & 0x0F));

    if (!out.empty())
      out += ", ";
    out += item;
  }

  return out.empty() ? "none" : out;
}

uint8_t HUSB238Component::pdo_select_code_(uint8_t voltage) {
  switch (voltage) {
    case 5:
      return 0x01;
    case 9:
      return 0x02;
    case 12:
      return 0x03;
    case 15:
      return 0x08;
    case 18:
      return 0x09;
    case 20:
      return 0x0A;
    default:
      return 0x00;
  }
}

uint8_t HUSB238Component::status_voltage_to_volts_(uint8_t code) {
  switch (code) {
    case 0x01:
      return 5;
    case 0x02:
      return 9;
    case 0x03:
      return 12;
    case 0x04:
      return 15;
    case 0x05:
      return 18;
    case 0x06:
      return 20;
    default:
      return 0;
  }
}

float HUSB238Component::current_code_to_amps_(uint8_t code) {
  static const float amps[] = {
      0.50f, 0.70f, 1.00f, 1.25f, 1.50f, 1.75f, 2.00f, 2.25f,
      2.50f, 2.75f, 3.00f, 3.25f, 3.50f, 4.00f, 4.50f, 5.00f,
  };
  return amps[code & 0x0F];
}

float HUSB238Component::legacy_5v_current_to_amps_(uint8_t code) {
  switch (code & 0x03) {
    case 0x01:
      return 1.5f;
    case 0x02:
      return 2.4f;
    case 0x03:
      return 3.0f;
    default:
      return 0.5f;
  }
}

const char *HUSB238Component::pd_response_to_string_(uint8_t code) {
  switch (code) {
    case 0x00:
      return "no_response";
    case 0x01:
      return "success";
    case 0x03:
      return "invalid_command_or_argument";
    case 0x04:
      return "command_not_supported";
    case 0x05:
      return "transaction_failed";
    default:
      return "reserved";
  }
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
