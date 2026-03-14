#include "mcf8329a.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8329a {

static const char *const TAG = "mcf8329a";

void MCF8329ABrakeSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    this->publish_state(!state);
    return;
  }
  if (!this->parent_->set_brake_override(state)) {
    ESP_LOGW(TAG, "Failed to apply brake state");
    this->publish_state(!state);
    return;
  }
  this->publish_state(state);
}

void MCF8329ADirectionSelect::control(const std::string &value) {
  if (this->parent_ == nullptr || !this->parent_->set_direction_mode(value)) {
    ESP_LOGW(TAG, "Failed to set direction mode to %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void MCF8329ASpeedNumber::control(float value) {
  if (this->parent_ == nullptr || !this->parent_->set_speed_percent(value)) {
    ESP_LOGW(TAG, "Failed to set speed command");
    return;
  }
  this->publish_state(value);
}

void MCF8329AClearFaultsButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pulse_clear_faults();
  }
}

void MCF8329AWatchdogTickleButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pulse_watchdog_tickle();
  }
}

void MCF8329AComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up mcf8329a");
  this->normal_operation_ready_ = false;
  this->deferred_comms_last_retry_ms_ = 0u;
  this->deferred_comms_last_scan_ms_ = 0u;

  if (!this->scan_i2c_bus_()) {
    ESP_LOGW(TAG, "I2C scan failed; continuing with communication retries");
    this->status_set_warning();
  }
  if (!this->establish_communications_(STARTUP_COMMS_ATTEMPTS, STARTUP_COMMS_RETRY_DELAY_MS, true)) {
    ESP_LOGW(TAG, "Unable to establish communications with I2C device 0x%02X during setup; deferring normal operation",
             this->address_);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  this->normal_operation_ready_ = true;
  this->apply_post_comms_setup_();
}

void MCF8329AComponent::update() {
  if (!this->normal_operation_ready_) {
    this->process_deferred_startup_();
    return;
  }

  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t controller_fault_status = 0;
  uint32_t vm_voltage_raw = 0;
  bool fault_active = false;
  bool fault_state_valid = false;

  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  if (algo_ok) {
    this->publish_algo_status_(algo_status);
  }

  const bool gate_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_fault_status);
  if (gate_ok) {
    fault_active |= (gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }

  const bool controller_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, controller_fault_status);
  if (controller_ok) {
    fault_active |= (controller_fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }

  if (gate_ok || controller_ok) {
    this->publish_faults_(gate_fault_status, gate_ok, controller_fault_status, controller_ok);
  }
  this->publish_raw_status_hex_(gate_fault_status, gate_ok, controller_fault_status, controller_ok, algo_status, algo_ok);

  if (fault_state_valid) {
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state(fault_active);
    }
    this->handle_fault_shutdown_(fault_active);
  } else {
    this->fault_latched_ = false;
  }

  if (this->algorithm_state_text_sensor_ != nullptr || this->algorithm_state_code_sensor_ != nullptr) {
    uint16_t algorithm_state = 0;
    if (this->read_reg16(REG_ALGORITHM_STATE, algorithm_state)) {
      if (this->algorithm_state_code_sensor_ != nullptr) {
        this->algorithm_state_code_sensor_->publish_state(static_cast<float>(algorithm_state));
      }
      if (this->algorithm_state_text_sensor_ != nullptr) {
        this->algorithm_state_text_sensor_->publish_state(this->algorithm_state_to_string_(algorithm_state));
      }
    }
  }

  if (this->motor_bemf_constant_sensor_ != nullptr) {
    uint32_t mtr_params = 0;
    if (this->read_reg32(REG_MTR_PARAMS, mtr_params)) {
      const uint32_t motor_bemf_const = (mtr_params & MTR_PARAMS_MOTOR_BEMF_CONST_MASK) >> MTR_PARAMS_MOTOR_BEMF_CONST_SHIFT;
      this->motor_bemf_constant_sensor_->publish_state(static_cast<float>(motor_bemf_const));
    }
  }

  if (this->read_reg32(REG_VM_VOLTAGE, vm_voltage_raw)) {
    const float vm_voltage = static_cast<float>(static_cast<double>(vm_voltage_raw) * VM_VOLTAGE_SCALE);
    if (this->vm_voltage_sensor_ != nullptr) {
      this->vm_voltage_sensor_->publish_state(vm_voltage);
    }
    if ((millis() - this->last_vm_diag_log_ms_) >= 5000U) {
      ESP_LOGD(TAG, "VM decode: raw=0x%08X -> %.2fV", vm_voltage_raw, vm_voltage);
      this->last_vm_diag_log_ms_ = millis();
    }
  }

  if (this->auto_tickle_watchdog_ && (millis() - this->last_watchdog_tickle_ms_ >= 500U)) {
    this->pulse_watchdog_tickle();
  }
}

void MCF8329AComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MCF8329A Manual Component:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Inter-byte delay: %u us", static_cast<unsigned>(this->inter_byte_delay_us_));
  if (this->inter_byte_delay_us_ > 0) {
    ESP_LOGCONFIG(TAG, "  Note: inter-byte delay is currently not applied with ESPHome I2C transactions");
  }
  ESP_LOGCONFIG(TAG, "  Auto tickle watchdog: %s", YESNO(this->auto_tickle_watchdog_));
  ESP_LOGCONFIG(TAG, "  Apply startup config: %s", YESNO(this->apply_startup_config_));
  ESP_LOGCONFIG(TAG, "  Startup brake mode: %s",
                this->startup_brake_mode_set_ ? this->startup_brake_mode_to_string_(this->startup_brake_mode_)
                                              : "(unchanged)");
  ESP_LOGCONFIG(TAG, "  Startup brake time: %s",
                this->startup_brake_time_set_ ? this->startup_brake_time_to_string_(this->startup_brake_time_)
                                              : "(unchanged)");
  ESP_LOGCONFIG(TAG, "  Startup mode: %s",
                this->startup_mode_set_ ? this->startup_mode_to_string_(this->startup_mode_) : "(unchanged)");
  ESP_LOGCONFIG(TAG, "  Startup align time: %s",
                this->startup_align_time_set_ ? this->startup_align_time_to_string_(this->startup_align_time_)
                                              : "(unchanged)");
  ESP_LOGCONFIG(TAG, "  Startup direction: %s",
                this->startup_direction_mode_set_ ? this->startup_direction_mode_.c_str() : "(hardware default)");
  ESP_LOGCONFIG(TAG, "  Startup comm gate: scan 0x%02X..0x%02X, attempts=%u, retry=%ums, deferred_retry=%ums",
                static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN), static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX),
                static_cast<unsigned>(STARTUP_COMMS_ATTEMPTS), static_cast<unsigned>(STARTUP_COMMS_RETRY_DELAY_MS),
                static_cast<unsigned>(DEFERRED_COMMS_RETRY_INTERVAL_MS));
}

const char *MCF8329AComponent::i2c_error_to_string_(i2c::ErrorCode error_code) const {
  switch (error_code) {
    case i2c::ERROR_OK:
      return "ok";
    case i2c::ERROR_INVALID_ARGUMENT:
      return "invalid_argument";
    case i2c::ERROR_NOT_ACKNOWLEDGED:
      return "not_acknowledged";
    case i2c::ERROR_TIMEOUT:
      return "timeout";
    case i2c::ERROR_NOT_INITIALIZED:
      return "not_initialized";
    case i2c::ERROR_TOO_LARGE:
      return "too_large";
    case i2c::ERROR_UNKNOWN:
      return "unknown";
    case i2c::ERROR_CRC:
      return "crc";
    default:
      return "other";
  }
}

bool MCF8329AComponent::probe_device_ack_(i2c::ErrorCode &error_code) const {
  if (this->bus_ == nullptr) {
    error_code = i2c::ERROR_NOT_INITIALIZED;
    return false;
  }

  error_code = this->bus_->write_readv(this->address_, nullptr, 0, nullptr, 0);
  return error_code == i2c::ERROR_OK;
}

bool MCF8329AComponent::scan_i2c_bus_() {
  if (this->bus_ == nullptr) {
    ESP_LOGW(TAG, "I2C scan skipped: bus is not initialized");
    return false;
  }

  std::string discovered;
  uint8_t device_count = 0;
  bool target_found = false;
  for (uint8_t address = I2C_SCAN_ADDRESS_MIN; address <= I2C_SCAN_ADDRESS_MAX; address++) {
    const i2c::ErrorCode err = this->bus_->write_readv(address, nullptr, 0, nullptr, 0);
    if (err == i2c::ERROR_OK) {
      char addr_text[8];
      std::snprintf(addr_text, sizeof(addr_text), "0x%02X", address);
      if (!discovered.empty()) {
        discovered += ", ";
      }
      discovered += addr_text;
      device_count++;
      if (address == this->address_) {
        target_found = true;
      }
    }
    arch_feed_wdt();
  }

  if (device_count == 0u) {
    ESP_LOGW(TAG, "I2C scan found no ACKing devices in range 0x%02X..0x%02X",
             static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN), static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX));
  } else {
    ESP_LOGI(TAG, "I2C scan found %u device(s): %s", static_cast<unsigned>(device_count), discovered.c_str());
  }

  if (target_found) {
    ESP_LOGI(TAG, "I2C target 0x%02X was found during scan", this->address_);
  } else {
    ESP_LOGW(TAG, "I2C target 0x%02X was not found during scan", this->address_);
  }

  return device_count > 0u;
}

bool MCF8329AComponent::establish_communications_(uint8_t attempts, uint32_t retry_delay_ms, bool log_retry_delays) {
  if (attempts == 0u) {
    return false;
  }

  for (uint8_t attempt = 1u; attempt <= attempts; attempt++) {
    i2c::ErrorCode ack_error = i2c::ERROR_UNKNOWN;
    if (!this->probe_device_ack_(ack_error)) {
      ESP_LOGW(TAG, "Comms attempt %u/%u: address 0x%02X probe failed: %s (%d)", static_cast<unsigned>(attempt),
               static_cast<unsigned>(attempts), this->address_, this->i2c_error_to_string_(ack_error),
               static_cast<int>(ack_error));
    } else if (this->read_probe_and_publish_()) {
      ESP_LOGI(TAG, "I2C communications established with 0x%02X (attempt %u/%u)", this->address_,
               static_cast<unsigned>(attempt), static_cast<unsigned>(attempts));
      return true;
    } else {
      ESP_LOGW(TAG, "Comms attempt %u/%u: address 0x%02X ACKed but register probe failed", static_cast<unsigned>(attempt),
               static_cast<unsigned>(attempts), this->address_);
    }

    if (attempt < attempts && retry_delay_ms > 0u) {
      if (log_retry_delays) {
        ESP_LOGW(TAG, "Retrying communications in %ums", static_cast<unsigned>(retry_delay_ms));
      }
      delay(retry_delay_ms);
    }
  }

  return false;
}

void MCF8329AComponent::process_deferred_startup_() {
  const uint32_t now = millis();
  const bool should_scan = this->deferred_comms_last_scan_ms_ == 0u ||
                           (now - this->deferred_comms_last_scan_ms_) >= DEFERRED_SCAN_INTERVAL_MS;
  if (should_scan) {
    if (!this->scan_i2c_bus_()) {
      ESP_LOGW(TAG, "Deferred I2C scan failed; keeping deferred retry mode");
      this->status_set_warning();
    }
    this->deferred_comms_last_scan_ms_ = now;
  }

  if (this->deferred_comms_last_retry_ms_ != 0u &&
      (now - this->deferred_comms_last_retry_ms_) < DEFERRED_COMMS_RETRY_INTERVAL_MS) {
    return;
  }
  this->deferred_comms_last_retry_ms_ = now;

  if (!this->establish_communications_(1u, 0u, false)) {
    this->status_set_warning();
    return;
  }

  ESP_LOGI(TAG, "I2C communications recovered; entering normal operation");
  this->status_clear_warning();
  this->normal_operation_ready_ = true;
  this->apply_post_comms_setup_();
}

void MCF8329AComponent::apply_post_comms_setup_() {
  if (this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(0.0f);
  }
  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Failed to force speed to 0%% during setup");
  }

  if (this->brake_switch_ != nullptr) {
    this->brake_switch_->publish_state(true);
  }
  if (!this->set_brake_override(true)) {
    ESP_LOGW(TAG, "Failed to force brake ON during setup");
  }

  const std::string direction_mode =
      (this->apply_startup_config_ && this->startup_direction_mode_set_) ? this->startup_direction_mode_ : "hardware";
  if (this->direction_select_ != nullptr) {
    this->direction_select_->publish_state(direction_mode);
  }
  if (!this->set_direction_mode(direction_mode)) {
    ESP_LOGW(TAG, "Failed to set startup direction mode to %s", direction_mode.c_str());
  }

  if (!this->apply_startup_motor_config_()) {
    ESP_LOGW(TAG, "Failed to apply one or more configured startup motor settings");
  }
  if (this->startup_config_text_sensor_ != nullptr) {
    this->startup_config_text_sensor_->publish_state(this->startup_config_summary_);
  }
}

bool MCF8329AComponent::apply_startup_motor_config_() {
  if (!this->apply_startup_config_) {
    this->startup_config_summary_ = "disabled";
    ESP_LOGI(TAG, "Startup motor config application disabled");
    return true;
  }

  const bool has_startup_settings =
      this->startup_brake_mode_set_ || this->startup_brake_time_set_ || this->startup_mode_set_ || this->startup_align_time_set_;
  if (!has_startup_settings) {
    this->startup_config_summary_ = "default";
    return true;
  }

  bool ok = true;

  uint32_t closed_loop2 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP2, closed_loop2)) {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP2 for startup config");
    return false;
  }
  uint32_t closed_loop2_next = closed_loop2;
  if (this->startup_brake_mode_set_) {
    closed_loop2_next =
        (closed_loop2_next & ~CLOSED_LOOP2_MTR_STOP_MASK) |
        ((static_cast<uint32_t>(this->startup_brake_mode_) << CLOSED_LOOP2_MTR_STOP_SHIFT) & CLOSED_LOOP2_MTR_STOP_MASK);
  }
  if (this->startup_brake_time_set_) {
    closed_loop2_next = (closed_loop2_next & ~CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK) |
                        ((static_cast<uint32_t>(this->startup_brake_time_) << CLOSED_LOOP2_MTR_STOP_BRK_TIME_SHIFT) &
                         CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK);
  }
  if (closed_loop2_next != closed_loop2) {
    ok &= this->write_reg32(REG_CLOSED_LOOP2, closed_loop2_next);
    ESP_LOGI(TAG, "CLOSED_LOOP2 startup cfg: 0x%08X -> 0x%08X", closed_loop2, closed_loop2_next);
  }

  uint32_t motor_startup1 = 0;
  if (!this->read_reg32(REG_MOTOR_STARTUP1, motor_startup1)) {
    ESP_LOGW(TAG, "Failed to read MOTOR_STARTUP1 for startup config");
    return false;
  }
  uint32_t motor_startup1_next = motor_startup1;
  if (this->startup_mode_set_) {
    motor_startup1_next =
        (motor_startup1_next & ~MOTOR_STARTUP1_MTR_STARTUP_MASK) |
        ((static_cast<uint32_t>(this->startup_mode_) << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) & MOTOR_STARTUP1_MTR_STARTUP_MASK);
  }
  if (this->startup_align_time_set_) {
    motor_startup1_next =
        (motor_startup1_next & ~MOTOR_STARTUP1_ALIGN_TIME_MASK) |
        ((static_cast<uint32_t>(this->startup_align_time_) << MOTOR_STARTUP1_ALIGN_TIME_SHIFT) & MOTOR_STARTUP1_ALIGN_TIME_MASK);
  }
  if (motor_startup1_next != motor_startup1) {
    ok &= this->write_reg32(REG_MOTOR_STARTUP1, motor_startup1_next);
    ESP_LOGI(TAG, "MOTOR_STARTUP1 startup cfg: 0x%08X -> 0x%08X", motor_startup1, motor_startup1_next);
  }

  std::string summary;
  if (this->startup_brake_mode_set_) {
    summary += "stop=";
    summary += this->startup_brake_mode_to_string_(this->startup_brake_mode_);
  }
  if (this->startup_brake_time_set_) {
    if (!summary.empty()) {
      summary += ",";
    }
    summary += "stop_brake_time=";
    summary += this->startup_brake_time_to_string_(this->startup_brake_time_);
  }
  if (this->startup_mode_set_) {
    if (!summary.empty()) {
      summary += ",";
    }
    summary += "startup=";
    summary += this->startup_mode_to_string_(this->startup_mode_);
  }
  if (this->startup_align_time_set_) {
    if (!summary.empty()) {
      summary += ",";
    }
    summary += "align_time=";
    summary += this->startup_align_time_to_string_(this->startup_align_time_);
  }
  if (this->startup_direction_mode_set_) {
    if (!summary.empty()) {
      summary += ",";
    }
    summary += "dir=";
    summary += this->startup_direction_mode_;
  }
  if (summary.empty()) {
    summary = "default";
  }
  this->startup_config_summary_ = summary;
  ESP_LOGI(TAG, "Startup motor config: %s", this->startup_config_summary_.c_str());
  return ok;
}

void MCF8329AComponent::publish_raw_status_hex_(uint32_t gate_fault_status, bool gate_ok, uint32_t controller_fault_status,
                                                bool controller_ok, uint32_t algo_status, bool algo_ok) {
  char hex_buf[11];

  if (gate_ok && this->gate_fault_status_text_sensor_ != nullptr) {
    std::snprintf(hex_buf, sizeof(hex_buf), "0x%08X", gate_fault_status);
    this->gate_fault_status_text_sensor_->publish_state(hex_buf);
  }
  if (controller_ok && this->controller_fault_status_text_sensor_ != nullptr) {
    std::snprintf(hex_buf, sizeof(hex_buf), "0x%08X", controller_fault_status);
    this->controller_fault_status_text_sensor_->publish_state(hex_buf);
  }
  if (algo_ok && this->algo_status_text_sensor_ != nullptr) {
    std::snprintf(hex_buf, sizeof(hex_buf), "0x%08X", algo_status);
    this->algo_status_text_sensor_->publish_state(hex_buf);
  }
}

bool MCF8329AComponent::read_reg32(uint16_t offset, uint32_t &value) { return this->perform_read_(offset, value); }

bool MCF8329AComponent::read_reg16(uint16_t offset, uint16_t &value) { return this->perform_read16_(offset, value); }

bool MCF8329AComponent::write_reg32(uint16_t offset, uint32_t value) { return this->perform_write_(offset, value); }

bool MCF8329AComponent::update_bits32(uint16_t offset, uint32_t mask, uint32_t value) {
  uint32_t current = 0;
  if (!this->read_reg32(offset, current)) {
    return false;
  }
  const uint32_t next = (current & ~mask) | (value & mask);
  if (next == current) {
    return true;
  }
  return this->write_reg32(offset, next);
}

bool MCF8329AComponent::set_brake_override(bool brake_on) {
  const uint32_t value = brake_on ? PIN_CONFIG_BRAKE_INPUT_BRAKE : PIN_CONFIG_BRAKE_INPUT_NO_BRAKE;
  if (!this->update_bits32(REG_PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value)) {
    return false;
  }

  uint32_t pin_config = 0;
  if (this->read_reg32(REG_PIN_CONFIG, pin_config)) {
    const uint32_t brake_input_value = (pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 2;
    ESP_LOGI(TAG, "Brake override write: request=%s pin_cfg=0x%08X brake_input=%u(%s)", brake_on ? "ON" : "OFF",
             pin_config, static_cast<unsigned>(brake_input_value), this->brake_input_to_string_(brake_input_value));
  }
  return true;
}

bool MCF8329AComponent::set_direction_mode(const std::string &direction_mode) {
  uint32_t value = PERI_CONFIG1_DIR_INPUT_HARDWARE;
  if (direction_mode == "cw") {
    value = PERI_CONFIG1_DIR_INPUT_CW;
  } else if (direction_mode == "ccw") {
    value = PERI_CONFIG1_DIR_INPUT_CCW;
  }

  if (!this->update_bits32(REG_PERI_CONFIG1, PERI_CONFIG1_DIR_INPUT_MASK, value)) {
    return false;
  }

  uint32_t peri_config1 = 0;
  if (this->read_reg32(REG_PERI_CONFIG1, peri_config1)) {
    const uint32_t direction_input_value = (peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK) >> 19;
    ESP_LOGI(TAG, "Direction write: request=%s peri_cfg1=0x%08X dir_input=%u(%s)", direction_mode.c_str(), peri_config1,
             static_cast<unsigned>(direction_input_value), this->direction_input_to_string_(direction_input_value));
  }
  return true;
}

bool MCF8329AComponent::set_speed_percent(float speed_percent) {
  if (std::isnan(speed_percent)) {
    return false;
  }

  float clamped = speed_percent;
  if (clamped < 0.0f) {
    clamped = 0.0f;
  } else if (clamped > 100.0f) {
    clamped = 100.0f;
  }

  const uint16_t digital_speed_ctrl = static_cast<uint16_t>(lroundf((clamped / 100.0f) * 32767.0f));

  uint32_t value = ALGO_DEBUG1_OVERRIDE_MASK;
  value |= (static_cast<uint32_t>(digital_speed_ctrl) << 16) & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK;

  if (!this->update_bits32(REG_ALGO_DEBUG1, ALGO_DEBUG1_OVERRIDE_MASK | ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK, value)) {
    return false;
  }

  if (this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(clamped);
  }
  return true;
}

void MCF8329AComponent::pulse_clear_faults() {
  ESP_LOGI(TAG, "Pulsing clear faults");

  uint32_t gate_before = 0;
  uint32_t ctrl_before = 0;
  uint32_t gate_after = 0;
  uint32_t ctrl_after = 0;
  const bool gate_before_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_before);
  const bool ctrl_before_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, ctrl_before);

  if (!this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, ALGO_CTRL1_CLR_FLT_MASK)) {
    ESP_LOGW(TAG, "Failed to assert CLR_FLT");
    return;
  }
  delay_microseconds_safe(2000);
  if (!this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, 0)) {
    ESP_LOGW(TAG, "Failed to deassert CLR_FLT");
  }
  delay_microseconds_safe(2000);

  const bool gate_after_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_after);
  const bool ctrl_after_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, ctrl_after);

  if (gate_before_ok || ctrl_before_ok || gate_after_ok || ctrl_after_ok) {
    ESP_LOGI(TAG, "CLR_FLT status: gate 0x%08X -> 0x%08X, ctrl 0x%08X -> 0x%08X", gate_before, gate_after, ctrl_before,
             ctrl_after);
  }

  if (gate_after_ok || ctrl_after_ok) {
    this->publish_faults_(gate_after, gate_after_ok, ctrl_after, ctrl_after_ok);
    bool fault_active = false;
    if (gate_after_ok) {
      fault_active |= (gate_after & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0;
    }
    if (ctrl_after_ok) {
      fault_active |= (ctrl_after & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    }
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state(fault_active);
    }
    this->handle_fault_shutdown_(fault_active);
  }
}

void MCF8329AComponent::pulse_watchdog_tickle() {
  ESP_LOGD(TAG, "Pulsing watchdog tickle");
  if (this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK)) {
    (void) this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0);
  }
  this->last_watchdog_tickle_ms_ = millis();
}

bool MCF8329AComponent::read_probe_and_publish_() {
  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t controller_fault_status = 0;
  bool fault_active = false;
  bool fault_state_valid = false;
  bool ok = true;

  const bool gate_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_fault_status);
  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  const bool controller_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, controller_fault_status);
  ok &= algo_ok;
  ok &= gate_ok;
  ok &= controller_ok;

  if (gate_ok) {
    fault_active |= (gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }
  if (controller_ok) {
    fault_active |= (controller_fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }

  if (algo_ok) {
    this->publish_algo_status_(algo_status);
  }
  if (gate_ok || controller_ok) {
    this->publish_faults_(gate_fault_status, gate_ok, controller_fault_status, controller_ok);
  }
  if (fault_state_valid && this->fault_active_binary_sensor_ != nullptr) {
    this->fault_active_binary_sensor_->publish_state(fault_active);
  }

  return ok;
}

bool MCF8329AComponent::perform_read_(uint16_t offset, uint32_t &value) {
  const uint32_t control_word = this->build_control_word_(true, offset, true);
  const uint8_t cw[3] = {
      static_cast<uint8_t>((control_word >> 16) & 0xFF),
      static_cast<uint8_t>((control_word >> 8) & 0xFF),
      static_cast<uint8_t>(control_word & 0xFF),
  };

  uint8_t rx[4] = {0, 0, 0, 0};

  const i2c::ErrorCode err = this->write_read(cw, sizeof(cw), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "read_reg32(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }

  value = static_cast<uint32_t>(rx[0]) | (static_cast<uint32_t>(rx[1]) << 8) | (static_cast<uint32_t>(rx[2]) << 16) |
          (static_cast<uint32_t>(rx[3]) << 24);
  return true;
}

bool MCF8329AComponent::perform_read16_(uint16_t offset, uint16_t &value) {
  const uint32_t control_word = this->build_control_word_(true, offset, false);
  const uint8_t cw[3] = {
      static_cast<uint8_t>((control_word >> 16) & 0xFF),
      static_cast<uint8_t>((control_word >> 8) & 0xFF),
      static_cast<uint8_t>(control_word & 0xFF),
  };

  uint8_t rx[2] = {0, 0};

  const i2c::ErrorCode err = this->write_read(cw, sizeof(cw), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "read_reg16(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }

  value = static_cast<uint16_t>(rx[0]) | (static_cast<uint16_t>(rx[1]) << 8);
  return true;
}

bool MCF8329AComponent::perform_write_(uint16_t offset, uint32_t value) {
  const uint32_t control_word = this->build_control_word_(false, offset, true);
  const uint8_t cw[3] = {
      static_cast<uint8_t>((control_word >> 16) & 0xFF),
      static_cast<uint8_t>((control_word >> 8) & 0xFF),
      static_cast<uint8_t>(control_word & 0xFF),
  };
  const uint8_t payload[4] = {
      static_cast<uint8_t>(value & 0xFF),
      static_cast<uint8_t>((value >> 8) & 0xFF),
      static_cast<uint8_t>((value >> 16) & 0xFF),
      static_cast<uint8_t>((value >> 24) & 0xFF),
  };

  const uint8_t tx[7] = {cw[0], cw[1], cw[2], payload[0], payload[1], payload[2], payload[3]};
  const i2c::ErrorCode err = this->write(tx, sizeof(tx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "write_reg32(0x%04X, 0x%08X) failed: i2c error %d", offset, value, static_cast<int>(err));
    return false;
  }
  return true;
}

uint32_t MCF8329AComponent::build_control_word_(bool is_read, uint16_t offset, bool is_32bit) const {
  const uint32_t dlen = is_32bit ? 0x1u : 0x0u;
  return ((is_read ? 1u : 0u) << 23) | (dlen << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

void MCF8329AComponent::publish_faults_(uint32_t gate_fault_status, bool gate_fault_valid,
                                        uint32_t controller_fault_status, bool controller_fault_valid) {
  std::vector<std::string> faults;
  bool gate_detail_found = false;
  bool controller_detail_found = false;

  if (gate_fault_valid) {
    if (gate_fault_status & GATE_FAULT_OTS)
      faults.emplace_back("DRV_OTS"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_VDS)
      faults.emplace_back("DRV_OCP_VDS"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_SNS)
      faults.emplace_back("DRV_OCP_SNS"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_BST_UV)
      faults.emplace_back("DRV_BST_UV"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_GVDD_UV)
      faults.emplace_back("DRV_GVDD_UV"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_DRV_OFF)
      faults.emplace_back("DRV_OFF"), gate_detail_found = true;
    if ((gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0 && !gate_detail_found) {
      faults.emplace_back("DRV_FAULT_ACTIVE");
    }
  }

  if (controller_fault_valid) {
    if (controller_fault_status & FAULT_IPD_FREQ)
      faults.emplace_back("IPD_FREQ_FAULT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_IPD_T1)
      faults.emplace_back("IPD_T1_FAULT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_BUS_CURRENT_LIMIT)
      faults.emplace_back("BUS_CURRENT_LIMIT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_MPET_BEMF)
      faults.emplace_back("MPET_BEMF_FAULT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_ABN_SPEED)
      faults.emplace_back("ABN_SPEED"), controller_detail_found = true;
    if (controller_fault_status & FAULT_ABN_BEMF)
      faults.emplace_back("ABN_BEMF"), controller_detail_found = true;
    if (controller_fault_status & FAULT_NO_MTR)
      faults.emplace_back("NO_MTR"), controller_detail_found = true;
    if (controller_fault_status & FAULT_MTR_LCK)
      faults.emplace_back("MTR_LCK"), controller_detail_found = true;
    if (controller_fault_status & FAULT_LOCK_LIMIT)
      faults.emplace_back("LOCK_LIMIT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_HW_LOCK_LIMIT)
      faults.emplace_back("HW_LOCK_LIMIT"), controller_detail_found = true;
    if (controller_fault_status & FAULT_DCBUS_UNDER_VOLTAGE)
      faults.emplace_back("DCBUS_UNDER_VOLTAGE"), controller_detail_found = true;
    if (controller_fault_status & FAULT_DCBUS_OVER_VOLTAGE)
      faults.emplace_back("DCBUS_OVER_VOLTAGE"), controller_detail_found = true;
    if (controller_fault_status & FAULT_SPEED_LOOP_SATURATION)
      faults.emplace_back("SPEED_LOOP_SATURATION"), controller_detail_found = true;
    if (controller_fault_status & FAULT_CURRENT_LOOP_SATURATION)
      faults.emplace_back("CURRENT_LOOP_SATURATION"), controller_detail_found = true;
    if (controller_fault_status & FAULT_WATCHDOG)
      faults.emplace_back("WATCHDOG_FAULT"), controller_detail_found = true;
    if ((controller_fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0 && !controller_detail_found) {
      faults.emplace_back("CTRL_FAULT_ACTIVE");
    }
  }

  std::string summary = "none";
  if (!faults.empty()) {
    summary.clear();
    for (size_t i = 0; i < faults.size(); i++) {
      if (i != 0) {
        summary += ",";
      }
      summary += faults[i];
    }
  }

  if (this->fault_summary_text_sensor_ != nullptr) {
    this->fault_summary_text_sensor_->publish_state(summary);
  }

  if (summary != this->last_fault_summary_) {
    if (summary == "none") {
      ESP_LOGI(TAG, "Faults cleared");
    } else {
      ESP_LOGW(TAG, "Active faults: %s", summary.c_str());
    }
    this->last_fault_summary_ = summary;
  }
}

const char *MCF8329AComponent::startup_mode_to_string_(uint8_t mode) const {
  switch (mode) {
    case 0:
      return "align";
    case 1:
      return "double_align";
    case 2:
      return "ipd";
    case 3:
      return "slow_first_cycle";
    default:
      return "unknown";
  }
}

const char *MCF8329AComponent::startup_align_time_to_string_(uint8_t code) const {
  static const char *const kAlignTimeLabels[16] = {
      "10ms",  "50ms",  "100ms", "200ms", "300ms", "400ms", "500ms", "750ms",
      "1000ms", "1500ms", "2000ms", "3000ms", "4000ms", "5000ms", "7500ms", "10000ms",
  };
  return kAlignTimeLabels[code & 0x0Fu];
}

const char *MCF8329AComponent::startup_brake_mode_to_string_(uint8_t code) const {
  switch (code) {
    case 0:
      return "hiz";
    case 1:
      return "recirculation";
    case 2:
      return "low_side_brake";
    case 3:
      return "low_side_brake_alt";
    case 4:
      return "active_spin_down";
    default:
      return "reserved";
  }
}

const char *MCF8329AComponent::startup_brake_time_to_string_(uint8_t code) const {
  static const char *const kBrakeTimeLabels[16] = {
      "1ms",   "1ms",    "1ms",    "1ms",    "1ms",    "5ms",    "10ms",   "50ms",
      "100ms", "250ms",  "500ms",  "1000ms", "2500ms", "5000ms", "10000ms", "15000ms",
  };
  return kBrakeTimeLabels[code & 0x0Fu];
}

const char *MCF8329AComponent::algorithm_state_to_string_(uint16_t state) const {
  switch (state) {
    case 0x00:
      return "MOTOR_IDLE";
    case 0x01:
      return "MOTOR_ISD";
    case 0x02:
      return "MOTOR_TRISTATE";
    case 0x03:
      return "MOTOR_BRAKE_ON_START";
    case 0x04:
      return "MOTOR_IPD";
    case 0x05:
      return "MOTOR_SLOW_FIRST_CYCLE";
    case 0x06:
      return "MOTOR_ALIGN";
    case 0x07:
      return "MOTOR_OPEN_LOOP";
    case 0x08:
      return "MOTOR_CLOSED_LOOP_UNALIGNED";
    case 0x09:
      return "MOTOR_CLOSED_LOOP_ALIGNED";
    case 0x0A:
      return "MOTOR_CLOSED_LOOP_ACTIVE_BRAKING";
    case 0x0B:
      return "MOTOR_SOFT_STOP";
    case 0x0C:
      return "MOTOR_RECIRCULATE_STOP";
    case 0x0D:
      return "MOTOR_BRAKE_ON_STOP";
    case 0x0E:
      return "MOTOR_FAULT";
    case 0x0F:
      return "MOTOR_MPET_MOTOR_STOP_CHECK";
    case 0x10:
      return "MOTOR_MPET_MOTOR_STOP_WAIT";
    case 0x11:
      return "MOTOR_MPET_MOTOR_BRAKE";
    case 0x12:
      return "MOTOR_MPET_ALGORITHM_PARAMETERS_INIT";
    case 0x13:
      return "MOTOR_MPET_RL_MEASURE";
    case 0x14:
      return "MOTOR_MPET_KE_MEASURE";
    case 0x15:
      return "MOTOR_MPET_STALL_CURRENT_MEASURE";
    case 0x16:
      return "MOTOR_MPET_TORQUE_MODE";
    case 0x17:
      return "MOTOR_MPET_DONE";
    case 0x18:
      return "MOTOR_MPET_FAULT";
    default:
      return "UNKNOWN";
  }
}

const char *MCF8329AComponent::brake_input_to_string_(uint32_t brake_input_value) const {
  switch (brake_input_value & 0x3u) {
    case 0x0:
      return "hardware";
    case 0x1:
      return "brake_on";
    case 0x2:
      return "brake_off";
    default:
      return "reserved";
  }
}

const char *MCF8329AComponent::direction_input_to_string_(uint32_t direction_input_value) const {
  switch (direction_input_value & 0x3u) {
    case 0x0:
      return "hardware";
    case 0x1:
      return "cw";
    case 0x2:
      return "ccw";
    default:
      return "reserved";
  }
}

void MCF8329AComponent::publish_algo_status_(uint32_t algo_status) {
  const uint16_t duty_raw = (algo_status & ALGO_STATUS_DUTY_CMD_MASK) >> ALGO_STATUS_DUTY_CMD_SHIFT;
  const uint16_t volt_mag_raw = (algo_status & ALGO_STATUS_VOLT_MAG_MASK) >> ALGO_STATUS_VOLT_MAG_SHIFT;

  if (this->duty_cmd_percent_sensor_ != nullptr) {
    this->duty_cmd_percent_sensor_->publish_state((static_cast<float>(duty_raw) / 4095.0f) * 100.0f);
  }
  if (this->volt_mag_percent_sensor_ != nullptr) {
    this->volt_mag_percent_sensor_->publish_state((static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f);
  }
  if (this->sys_enable_binary_sensor_ != nullptr) {
    this->sys_enable_binary_sensor_->publish_state((algo_status & ALGO_STATUS_SYS_ENABLE_FLAG_MASK) != 0);
  }
}

void MCF8329AComponent::handle_fault_shutdown_(bool fault_active) {
  if (!fault_active) {
    this->fault_latched_ = false;
    return;
  }
  if (this->fault_latched_) {
    return;
  }

  this->fault_latched_ = true;
  ESP_LOGW(TAG, "Fault detected, forcing speed command to 0%%");
  (void) this->set_speed_percent(0.0f);
}

}  // namespace mcf8329a
}  // namespace esphome
