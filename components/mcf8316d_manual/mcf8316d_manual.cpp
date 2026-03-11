#include "mcf8316d_manual.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8316d_manual {

static const char *const TAG = "mcf8316d_manual";

void MCF8316DBrakeSwitch::write_state(bool state) {
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


void MCF8316DDirectionSelect::control(const std::string &value) {
  if (this->parent_ == nullptr || !this->parent_->set_direction_mode(value)) {
    ESP_LOGW(TAG, "Failed to set direction mode to %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void MCF8316DNumber::control(float value) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Failed to set number value: no parent");
    return;
  }

  bool ok = false;
  if (this->is_speed_command_) {
    ok = this->parent_->set_speed_percent(value);
    if (!ok) {
      ESP_LOGW(TAG, "Failed to set speed command");
    }
  } else {
    if (std::isnan(value)) {
      ESP_LOGW(TAG, "Failed to set motor tuning value: NaN");
      return;
    }
    const uint32_t tune_value = static_cast<uint32_t>(lroundf(value));
    ok = this->parent_->set_motor_tune_parameter(this->tune_parameter_, tune_value);
    if (!ok) {
      ESP_LOGW(TAG, "Failed to set motor tuning value");
    }
  }

  if (ok) {
    this->publish_state(value);
  }
}

void MCF8316DClearFaultsButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pulse_clear_faults();
  }
}

void MCF8316DWatchdogTickleButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pulse_watchdog_tickle();
  }
}

void MCF8316DApplyStartupTuneButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->apply_startup_tune_profile()) {
      ESP_LOGW(TAG, "Startup tune profile failed");
    }
  }
}

void MCF8316DApplyHwLockReportOnlyButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->apply_hw_lock_report_only_profile()) {
      ESP_LOGW(TAG, "HW lock report-only profile failed");
    }
  }
}

void MCF8316DRunStartupSweepButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->start_startup_current_sweep()) {
      ESP_LOGW(TAG, "Startup current sweep could not start");
    }
  }
}

void MCF8316DRunScopeProbeTestButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->start_scope_probe_test()) {
      ESP_LOGW(TAG, "Scope probe test could not start");
    }
  }
}

void MCF8316DCommitEepromButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->commit_shadow_registers_to_eeprom()) {
      ESP_LOGW(TAG, "EEPROM commit failed");
    }
  }
}

void MCF8316DManualComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up mcf8316d_manual");
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

void MCF8316DManualComponent::update() {
  if (!this->normal_operation_ready_) {
    this->process_deferred_startup_();
    return;
  }

  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
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
  const bool controller_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, fault_status);
  if (controller_ok) {
    fault_active |= (fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }
  if (gate_ok || controller_ok) {
    this->publish_faults_(gate_fault_status, gate_ok, fault_status, controller_ok);
  }
  if (fault_state_valid) {
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state(fault_active);
    }
    const bool force_shutdown =
        this->should_force_speed_shutdown_(gate_fault_status, gate_ok, fault_status, controller_ok);
    if (fault_active && !force_shutdown) {
      if (!this->allow_retry_notice_active_) {
        ESP_LOGI(TAG, "Fault active but allowing retry (configured lock mode)");
        this->allow_retry_notice_active_ = true;
      }
    } else {
      this->allow_retry_notice_active_ = false;
    }
    this->handle_fault_shutdown_(force_shutdown);
  } else {
    this->allow_retry_notice_active_ = false;
  }

  const uint16_t duty_raw = (algo_status & ALGO_STATUS_DUTY_CMD_MASK) >> ALGO_STATUS_DUTY_CMD_SHIFT;
  const uint16_t volt_mag_raw = (algo_status & ALGO_STATUS_VOLT_MAG_MASK) >> ALGO_STATUS_VOLT_MAG_SHIFT;
  const bool run_state_diag_active = algo_ok && duty_raw > 0u && volt_mag_raw > 0u && (!fault_state_valid || !fault_active);
  const bool control_diag_active = algo_ok && duty_raw > 8u;
  uint16_t algorithm_state = 0;
  bool algorithm_state_valid = false;
  const bool need_algorithm_state =
      this->algorithm_state_text_sensor_ != nullptr || run_state_diag_active || control_diag_active ||
      this->startup_sweep_active_ || this->scope_probe_test_active_;
  if (need_algorithm_state) {
    if (this->read_reg16(REG_ALGORITHM_STATE, algorithm_state)) {
      algorithm_state_valid = true;
      const char *const state_name = this->algorithm_state_to_string_(algorithm_state);
      if (this->algorithm_state_text_sensor_ != nullptr) {
        this->algorithm_state_text_sensor_->publish_state(state_name);
      }
      if (run_state_diag_active) {
        const uint32_t now = millis();
        const bool should_log = (this->last_run_state_diag_log_ms_ == 0u) ||
                                (algorithm_state != this->last_run_state_diag_value_);
        if (should_log) {
          const float duty_percent = (static_cast<float>(duty_raw) / 4095.0f) * 100.0f;
          const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;
          ESP_LOGI(TAG, "[loop_run_state] state=0x%04X(%s) duty=%.1f%% volt_mag=%.1f%%", algorithm_state, state_name,
                   duty_percent, volt_mag_percent);
          this->last_run_state_diag_log_ms_ = now;
          this->last_run_state_diag_value_ = algorithm_state;
        }
      }
      if (control_diag_active) {
        const uint32_t now = millis();
        const bool should_log =
            (this->last_control_diag_log_ms_ == 0u) || (algorithm_state != this->last_control_diag_state_);
        if (should_log) {
          this->log_control_diagnostics_("loop_control", algorithm_state, duty_raw, volt_mag_raw, fault_active);
          this->last_control_diag_log_ms_ = now;
          this->last_control_diag_state_ = algorithm_state;
        }
      }
    }
  }
  if (!run_state_diag_active) {
    this->last_run_state_diag_log_ms_ = 0u;
    this->last_run_state_diag_value_ = 0xFFFFu;
  }
  if (!control_diag_active) {
    this->last_control_diag_log_ms_ = 0u;
    this->last_control_diag_state_ = 0xFFFFu;
  }
  this->process_startup_sweep_(algorithm_state_valid, algorithm_state, fault_active, fault_state_valid, controller_ok,
                               fault_status, volt_mag_raw);
  this->process_scope_probe_test_(algorithm_state_valid, algorithm_state, fault_active, fault_state_valid, controller_ok,
                                  fault_status, volt_mag_raw);

  const bool mpet_fault_active = controller_ok && ((fault_status & (FAULT_MPET_IPD | FAULT_MPET_BEMF)) != 0);
  if (mpet_fault_active && ((this->last_mpet_diag_log_ms_ == 0U) || (millis() - this->last_mpet_diag_log_ms_ >= 2000U))) {
    this->log_mpet_diagnostics_("loop_mpet_fault");
    this->last_mpet_diag_log_ms_ = millis();
  }
  if (!mpet_fault_active) {
    this->last_mpet_diag_log_ms_ = 0;
  }

  const bool lock_limit_active = controller_ok && ((fault_status & (FAULT_LOCK_LIMIT | FAULT_HW_LOCK_LIMIT)) != 0);
  if (lock_limit_active && (!this->lock_limit_prev_active_ || (millis() - this->last_lock_limit_diag_log_ms_ >= 2000U))) {
    this->log_lock_limit_diagnostics_("loop_lock_limit", fault_status);
    this->last_lock_limit_diag_log_ms_ = millis();
  }
  if (!lock_limit_active) {
    this->last_lock_limit_diag_log_ms_ = 0;
  }
  this->lock_limit_prev_active_ = lock_limit_active;

  const uint32_t motor_lock_fault_mask = FAULT_MTR_LCK | FAULT_ABN_SPEED | FAULT_ABN_BEMF | FAULT_NO_MTR;
  const bool motor_lock_active = controller_ok && ((fault_status & motor_lock_fault_mask) != 0);
  if (motor_lock_active &&
      (!this->motor_lock_prev_active_ || (millis() - this->last_motor_lock_diag_log_ms_ >= 2000U))) {
    this->log_motor_lock_diagnostics_("loop_motor_lock", fault_status);
    this->last_motor_lock_diag_log_ms_ = millis();
  }
  if (!motor_lock_active) {
    this->last_motor_lock_diag_log_ms_ = 0;
  }
  this->motor_lock_prev_active_ = motor_lock_active;

  const bool buck_fault_active = gate_ok && ((gate_fault_status & (GATE_FAULT_BUCK_OCP | GATE_FAULT_BUCK_UV)) != 0);
  if (buck_fault_active &&
      ((this->last_buck_diag_log_ms_ == 0U) || (millis() - this->last_buck_diag_log_ms_ >= 2000U))) {
    this->log_buck_fault_diagnostics_("loop_buck_fault", gate_fault_status);
    this->last_buck_diag_log_ms_ = millis();
  }
  if (!buck_fault_active) {
    this->last_buck_diag_log_ms_ = 0;
  }

  if (this->read_reg32(REG_VM_VOLTAGE, vm_voltage_raw) && this->vm_voltage_sensor_ != nullptr) {
    const uint32_t vm_adc_code_8 = (vm_voltage_raw & VM_VOLTAGE_ADC_MASK) >> VM_VOLTAGE_ADC_SHIFT;
    const uint32_t vm_adc_code_q11 = (vm_voltage_raw & VM_VOLTAGE_Q11_MASK) >> VM_VOLTAGE_Q11_SHIFT;
    const float vm_v = static_cast<float>(vm_adc_code_q11) * (60.0f / 2048.0f);
    this->vm_voltage_sensor_->publish_state(vm_v);
    if ((millis() - this->last_vm_diag_log_ms_) >= 5000U) {
      ESP_LOGD(TAG, "VM decode: raw=0x%08X adc8=%u adc_q11=%u -> %.2fV", vm_voltage_raw,
               static_cast<unsigned>(vm_adc_code_8), static_cast<unsigned>(vm_adc_code_q11), vm_v);
      this->last_vm_diag_log_ms_ = millis();
    }
  }

  if (this->auto_tickle_watchdog_ && (millis() - this->last_watchdog_tickle_ms_ >= 500U)) {
    this->pulse_watchdog_tickle();
  }
}

void MCF8316DManualComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MCF8316D Manual Validation Component:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Inter-byte delay: %u us", static_cast<unsigned>(this->inter_byte_delay_us_));
  if (this->inter_byte_delay_us_ > 0) {
    ESP_LOGCONFIG(TAG, "  Note: inter-byte delay is currently not applied with ESPHome I2C transactions");
  }
  ESP_LOGCONFIG(TAG, "  Auto tickle watchdog: %s", YESNO(this->auto_tickle_watchdog_));
  ESP_LOGCONFIG(TAG, "  Startup comm gate: scan 0x%02X..0x%02X, attempts=%u, retry=%ums, deferred_retry=%ums",
                static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN), static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX),
                static_cast<unsigned>(STARTUP_COMMS_ATTEMPTS), static_cast<unsigned>(STARTUP_COMMS_RETRY_DELAY_MS),
                static_cast<unsigned>(DEFERRED_COMMS_RETRY_INTERVAL_MS));
}

const char *MCF8316DManualComponent::i2c_error_to_string_(i2c::ErrorCode error_code) const {
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

bool MCF8316DManualComponent::probe_device_ack_(i2c::ErrorCode &error_code) const {
  if (this->bus_ == nullptr) {
    error_code = i2c::ERROR_NOT_INITIALIZED;
    return false;
  }

  error_code = this->bus_->write_readv(this->address_, nullptr, 0, nullptr, 0);
  return error_code == i2c::ERROR_OK;
}

bool MCF8316DManualComponent::scan_i2c_bus_() {
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

bool MCF8316DManualComponent::establish_communications_(uint8_t attempts, uint32_t retry_delay_ms, bool log_retry_delays) {
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

void MCF8316DManualComponent::process_deferred_startup_() {
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

void MCF8316DManualComponent::apply_post_comms_setup_() {
  uint32_t ctrl_fault = 0;

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

  if (this->direction_select_ != nullptr) {
    this->direction_select_->publish_state("hardware");
  }
  if (!this->set_direction_mode("hardware")) {
    ESP_LOGW(TAG, "Failed to force direction to hardware during setup");
  }

  if (!this->ensure_buck_current_limit_for_manual_()) {
    ESP_LOGW(TAG, "Failed to ensure buck current limit for manual validation");
  }

  if (!this->seed_closed_loop_params_if_zero_()) {
    ESP_LOGW(TAG, "Failed to seed one or more zero CLOSED_LOOP motor parameters");
  }

  // Force manual mode by disabling any MPET command/config bits carried over at boot.
  if (!this->update_bits32(REG_ALGO_DEBUG2, ALGO_DEBUG2_MPET_ALL_MASK, 0)) {
    ESP_LOGW(TAG, "Failed to disable MPET control bits in ALGO_DEBUG2");
  } else {
    uint32_t algo_debug2 = 0;
    if (this->read_reg32(REG_ALGO_DEBUG2, algo_debug2)) {
      ESP_LOGI(TAG, "ALGO_DEBUG2 after MPET disable: 0x%08X", algo_debug2);
    }
  }

  if (this->read_reg32(REG_CONTROLLER_FAULT_STATUS, ctrl_fault) && ((ctrl_fault & (FAULT_MPET_IPD | FAULT_MPET_BEMF)) != 0)) {
    ESP_LOGW(TAG, "MPET fault latched at startup (0x%08X), attempting clear", ctrl_fault);
    this->pulse_clear_faults();
  }

  this->log_mpet_diagnostics_("setup");
}

bool MCF8316DManualComponent::read_reg32(uint16_t offset, uint32_t &value) { return this->perform_read_(offset, value); }

bool MCF8316DManualComponent::read_reg16(uint16_t offset, uint16_t &value) { return this->perform_read16_(offset, value); }

bool MCF8316DManualComponent::write_reg32(uint16_t offset, uint32_t value) { return this->perform_write_(offset, value); }

bool MCF8316DManualComponent::update_bits32(uint16_t offset, uint32_t mask, uint32_t value) {
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

bool MCF8316DManualComponent::set_brake_override(bool brake_on) {
  const uint32_t value = brake_on ? PIN_CONFIG_BRAKE_INPUT_BRAKE : PIN_CONFIG_BRAKE_INPUT_NO_BRAKE;
  if (!this->update_bits32(REG_PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value)) {
    return false;
  }

  uint32_t pin_config = 0;
  if (this->read_reg32(REG_PIN_CONFIG, pin_config)) {
    const uint32_t brake_input_value = (pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 10;
    ESP_LOGI(TAG, "Brake override write: request=%s pin_cfg=0x%08X brake_input=%u(%s)", brake_on ? "ON" : "OFF",
             pin_config, static_cast<unsigned>(brake_input_value), this->brake_input_to_string_(brake_input_value));
  }
  return true;
}

bool MCF8316DManualComponent::set_direction_mode(const std::string &direction_mode) {
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
    const uint32_t direction_input_value = (peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK);
    ESP_LOGI(TAG, "Direction write: request=%s peri_cfg1=0x%08X dir_input=%u(%s)", direction_mode.c_str(), peri_config1,
             static_cast<unsigned>(direction_input_value), this->direction_input_to_string_(direction_input_value));
  }
  return true;
}

bool MCF8316DManualComponent::set_motor_tune_parameter(MotorTuneParameter parameter, uint32_t value) {
  uint16_t reg = REG_CLOSED_LOOP2;
  uint32_t mask = 0;
  uint32_t shifted_value = 0;
  const char *label = "";

  switch (parameter) {
    case MotorTuneParameter::MOTOR_RES:
      reg = REG_CLOSED_LOOP2;
      mask = CLOSED_LOOP2_MOTOR_RES_MASK;
      shifted_value = (value << CLOSED_LOOP2_MOTOR_RES_SHIFT);
      label = "MOTOR_RES";
      break;
    case MotorTuneParameter::MOTOR_IND:
      reg = REG_CLOSED_LOOP2;
      mask = CLOSED_LOOP2_MOTOR_IND_MASK;
      shifted_value = (value << CLOSED_LOOP2_MOTOR_IND_SHIFT);
      label = "MOTOR_IND";
      break;
    case MotorTuneParameter::MOTOR_BEMF_CONST:
      reg = REG_CLOSED_LOOP3;
      mask = CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK;
      shifted_value = (value << CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT);
      label = "MOTOR_BEMF_CONST";
      break;
    case MotorTuneParameter::SPEED_LOOP_KP: {
      const uint32_t kp = value & 0x3FFu;
      uint32_t cl3 = 0;
      uint32_t cl4 = 0;
      if (!this->read_reg32(REG_CLOSED_LOOP3, cl3) || !this->read_reg32(REG_CLOSED_LOOP4, cl4)) {
        ESP_LOGW(TAG, "Failed to read CLOSED_LOOP3/4 for SPEED_LOOP_KP");
        return false;
      }
      const uint32_t cl3_next = (cl3 & ~CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) |
                                ((kp >> 7) << CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT);
      const uint32_t cl4_next = (cl4 & ~CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) |
                                ((kp & 0x7Fu) << CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT);
      if ((cl3_next != cl3 && !this->write_reg32(REG_CLOSED_LOOP3, cl3_next)) ||
          (cl4_next != cl4 && !this->write_reg32(REG_CLOSED_LOOP4, cl4_next))) {
        ESP_LOGW(TAG, "Failed writing SPEED_LOOP_KP shadow fields");
        return false;
      }
      ESP_LOGI(TAG, "Updated SPEED_LOOP_KP to %u", static_cast<unsigned>(kp));
      return true;
    }
    case MotorTuneParameter::SPEED_LOOP_KI:
      reg = REG_CLOSED_LOOP4;
      mask = CLOSED_LOOP4_SPD_LOOP_KI_MASK;
      shifted_value = (value << CLOSED_LOOP4_SPD_LOOP_KI_SHIFT);
      label = "SPEED_LOOP_KI";
      break;
    case MotorTuneParameter::MAX_SPEED:
      reg = REG_CLOSED_LOOP4;
      mask = CLOSED_LOOP4_MAX_SPEED_MASK;
      shifted_value = (value << CLOSED_LOOP4_MAX_SPEED_SHIFT);
      label = "MAX_SPEED";
      break;
  }

  const uint32_t clipped = shifted_value & mask;
  if (!this->update_bits32(reg, mask, clipped)) {
    ESP_LOGW(TAG, "Failed to set %s", label);
    return false;
  }
  ESP_LOGI(TAG, "Updated %s to %u", label, static_cast<unsigned>(value));
  return true;
}

bool MCF8316DManualComponent::commit_shadow_registers_to_eeprom() {
  ESP_LOGW(TAG, "Committing shadow/RAM motor parameters to EEPROM");
  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Failed to force speed 0%% before EEPROM commit");
    return false;
  }

  if (!this->write_reg32(REG_ALGO_CTRL1, ALGO_CTRL1_EEPROM_WRITE_TRIGGER)) {
    ESP_LOGW(TAG, "Failed to trigger EEPROM write");
    return false;
  }

  delay_microseconds_safe(750000);
  uint32_t algo_ctrl1 = 0;
  if (!this->read_reg32(REG_ALGO_CTRL1, algo_ctrl1)) {
    ESP_LOGW(TAG, "Failed to read ALGO_CTRL1 after EEPROM write");
    return false;
  }
  if ((algo_ctrl1 & ALGO_CTRL1_EEPROM_WRITE_BUSY_MASK) != 0u) {
    ESP_LOGW(TAG, "EEPROM write still busy (ALGO_CTRL1=0x%08X)", algo_ctrl1);
    return false;
  }
  ESP_LOGI(TAG, "EEPROM commit complete (ALGO_CTRL1=0x%08X)", algo_ctrl1);
  return true;
}

bool MCF8316DManualComponent::set_speed_percent(float speed_percent) {
  if (std::isnan(speed_percent)) {
    return false;
  }

  const float clamped = clamp(speed_percent, 0.0f, 100.0f);
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

void MCF8316DManualComponent::pulse_clear_faults() {
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
    const bool force_shutdown =
        this->should_force_speed_shutdown_(gate_after, gate_after_ok, ctrl_after, ctrl_after_ok);
    this->handle_fault_shutdown_(force_shutdown);
  }

  if (gate_after_ok && ((gate_after & (GATE_FAULT_BUCK_OCP | GATE_FAULT_BUCK_UV)) != 0)) {
    ESP_LOGW(TAG, "CLR_FLT did not clear buck faults; condition likely still active");
    this->log_buck_fault_diagnostics_("clear_faults", gate_after);
  }
}

void MCF8316DManualComponent::pulse_watchdog_tickle() {
  ESP_LOGD(TAG, "Pulsing watchdog tickle");
  if (this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK)) {
    (void) this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0);
  }
  this->last_watchdog_tickle_ms_ = millis();
}

bool MCF8316DManualComponent::apply_startup_tune_profile() {
  ESP_LOGW(TAG, "Applying startup tune profile (double-align + higher max speed + startup current tuning)");
  bool ok = true;
  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Failed to set speed to 0%% before applying startup tune");
    ok = false;
  }
  if (!this->set_direction_mode("cw")) {
    ESP_LOGW(TAG, "Failed to force direction to cw for startup tune");
    ok = false;
  } else if (this->direction_select_ != nullptr) {
    this->direction_select_->publish_state("cw");
  }
  if (!this->set_brake_override(false)) {
    ESP_LOGW(TAG, "Failed to force brake OFF for startup tune");
    ok = false;
  } else if (this->brake_switch_ != nullptr) {
    this->brake_switch_->publish_state(false);
  }

  auto apply_masked_bits = [this](const char *label, uint16_t reg, uint32_t mask, uint32_t value) {
    uint32_t before = 0;
    if (!this->read_reg32(reg, before)) {
      ESP_LOGW(TAG, "%s read failed (reg=0x%04X)", label, reg);
      return false;
    }

    const uint32_t next = (before & ~mask) | (value & mask);
    if (next != before && !this->write_reg32(reg, next)) {
      ESP_LOGW(TAG, "%s write failed (reg=0x%04X): 0x%08X -> 0x%08X", label, reg, before, next);
      return false;
    }

    uint32_t after = 0;
    if (!this->read_reg32(reg, after)) {
      ESP_LOGW(TAG, "%s verify read failed (reg=0x%04X)", label, reg);
      return false;
    }
    ESP_LOGI(TAG, "%s: 0x%08X -> 0x%08X", label, before, after);

    const bool fields_match = (after & mask) == (value & mask);
    if (!fields_match) {
      ESP_LOGW(TAG, "%s verify mismatch (reg=0x%04X): expected mask=0x%08X actual mask=0x%08X", label, reg,
               (value & mask), (after & mask));
    }
    return fields_match;
  };

  ok &= apply_masked_bits(
      "FAULT_CONFIG1 tuning", REG_FAULT_CONFIG1,
      FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK | FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK | FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK |
          FAULT_CONFIG1_LCK_RETRY_MASK | FAULT_CONFIG1_MTR_LCK_MODE_MASK,
      (STARTUP_TUNE_HW_LOCK_ILIMIT << FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT) |
          (STARTUP_TUNE_LOCK_ILIMIT_MODE << FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT) |
          (STARTUP_TUNE_LOCK_ILIMIT_DEG << FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT) |
          (STARTUP_TUNE_LCK_RETRY << FAULT_CONFIG1_LCK_RETRY_SHIFT) |
          (STARTUP_TUNE_MTR_LCK_MODE << FAULT_CONFIG1_MTR_LCK_MODE_SHIFT));
  ok &= apply_masked_bits(
      "FAULT_CONFIG2 tuning", REG_FAULT_CONFIG2,
      FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK | FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK | FAULT_CONFIG2_LOCK2_EN_MASK |
          FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK,
      (STARTUP_TUNE_HW_LOCK_ILIMIT_DEG << FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT) |
          (STARTUP_TUNE_HW_LOCK_ILIMIT_MODE << FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT) |
          (STARTUP_TUNE_LOCK2_EN ? FAULT_CONFIG2_LOCK2_EN_MASK : 0u) |
          (STARTUP_TUNE_ABNORMAL_BEMF_THR << FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT));
  ok &= apply_masked_bits(
      "MOTOR_STARTUP1 tuning", REG_MOTOR_STARTUP1,
      MOTOR_STARTUP1_MTR_STARTUP_MASK | MOTOR_STARTUP1_ALIGN_TIME_MASK | MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK,
      (STARTUP_TUNE_MTR_STARTUP << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) |
      (STARTUP_TUNE_ALIGN_TIME << MOTOR_STARTUP1_ALIGN_TIME_SHIFT) |
      (STARTUP_TUNE_ALIGN_OR_SLOW_CURRENT_ILIMIT << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT));
  ok &= apply_masked_bits(
      "MOTOR_STARTUP2 tuning", REG_MOTOR_STARTUP2,
      MOTOR_STARTUP2_OL_ILIMIT_MASK | MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK | MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK |
          MOTOR_STARTUP2_ALIGN_ANGLE_MASK | MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK |
          MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK,
      (STARTUP_TUNE_OL_ILIMIT << MOTOR_STARTUP2_OL_ILIMIT_SHIFT) |
          (STARTUP_TUNE_AUTO_HANDOFF_EN << MOTOR_STARTUP2_AUTO_HANDOFF_EN_SHIFT) |
          (STARTUP_TUNE_OPN_CL_HANDOFF_THR << MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT) |
          (STARTUP_TUNE_ALIGN_ANGLE << MOTOR_STARTUP2_ALIGN_ANGLE_SHIFT) |
          (STARTUP_TUNE_SLOW_FIRST_CYC_FREQ << MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT) |
          (STARTUP_TUNE_FIRST_CYCLE_FREQ_SEL ? MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK : 0u));
  ok &= apply_masked_bits("CLOSED_LOOP1 tuning", REG_CLOSED_LOOP1, CLOSED_LOOP1_PWM_FREQ_OUT_MASK,
                          (STARTUP_TUNE_PWM_FREQ_OUT << CLOSED_LOOP1_PWM_FREQ_OUT_SHIFT));
  ok &= apply_masked_bits("DEVICE_CONFIG2 tuning", REG_DEVICE_CONFIG2, DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK,
                          STARTUP_TUNE_DYNAMIC_CSA_GAIN_EN ? DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK : 0u);
  ok &= apply_masked_bits("GD_CONFIG1 tuning", REG_GD_CONFIG1, GD_CONFIG1_CSA_GAIN_MASK,
                          (STARTUP_TUNE_CSA_GAIN << GD_CONFIG1_CSA_GAIN_SHIFT));
  ok &= apply_masked_bits(
      "ISD_CONFIG tuning", REG_ISD_CONFIG,
      ISD_CONFIG_ISD_EN_MASK | ISD_CONFIG_BRAKE_EN_MASK | ISD_CONFIG_RESYNC_EN_MASK | ISD_CONFIG_BRK_CONFIG_MASK |
          ISD_CONFIG_BRK_TIME_MASK,
      (STARTUP_TUNE_ISD_EN ? ISD_CONFIG_ISD_EN_MASK : 0u) | (STARTUP_TUNE_BRAKE_EN ? ISD_CONFIG_BRAKE_EN_MASK : 0u) |
          (STARTUP_TUNE_RESYNC_EN ? ISD_CONFIG_RESYNC_EN_MASK : 0u) |
          (STARTUP_TUNE_BRK_CONFIG ? ISD_CONFIG_BRK_CONFIG_MASK : 0u) |
          (STARTUP_TUNE_BRK_TIME << ISD_CONFIG_BRK_TIME_SHIFT));
  ok &= apply_masked_bits("CLOSED_LOOP4 tuning", REG_CLOSED_LOOP4, CLOSED_LOOP4_MAX_SPEED_MASK,
                          (STARTUP_TUNE_MAX_SPEED << CLOSED_LOOP4_MAX_SPEED_SHIFT));

  if (ok) {
    ESP_LOGI(TAG, "Startup tune profile applied; pulsing CLR_FLT");
  } else {
    ESP_LOGW(TAG, "Startup tune profile partially applied; pulsing CLR_FLT");
  }
  if (!STARTUP_TUNE_LOCK2_EN) {
    ESP_LOGW(TAG, "Startup tune disables ABN_BEMF lock (LOCK2_EN=0) for manual bring-up stability");
  }
  this->pulse_clear_faults();
  return ok;
}

bool MCF8316DManualComponent::apply_hw_lock_report_only_profile() {
  ESP_LOGW(TAG, "Applying locks-disabled + short-align debug profile");
  ESP_LOGW(TAG, "WARNING: LOCK/HW_LOCK/MTR_LCK protective actions are disabled for this debug mode");

  bool ok = true;
  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Failed to set speed to 0%% before applying HW lock report-only mode");
    ok = false;
  }
  if (!this->set_direction_mode("cw")) {
    ESP_LOGW(TAG, "Failed to force direction to cw for locks-disabled debug mode");
    ok = false;
  } else if (this->direction_select_ != nullptr) {
    this->direction_select_->publish_state("cw");
  }
  if (!this->set_brake_override(false)) {
    ESP_LOGW(TAG, "Failed to force brake OFF for locks-disabled debug mode");
    ok = false;
  } else if (this->brake_switch_ != nullptr) {
    this->brake_switch_->publish_state(false);
  }

  uint32_t before = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG2, before)) {
    ESP_LOGW(TAG, "FAULT_CONFIG2 read failed");
    return false;
  }

  const uint32_t mask = FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK;
  const uint32_t value = (LOCK_DISABLED_MODE << FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT);
  const uint32_t next = (before & ~mask) | (value & mask);
  if (next != before && !this->write_reg32(REG_FAULT_CONFIG2, next)) {
    ESP_LOGW(TAG, "FAULT_CONFIG2 write failed: 0x%08X -> 0x%08X", before, next);
    return false;
  }

  uint32_t after = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG2, after)) {
    ESP_LOGW(TAG, "FAULT_CONFIG2 verify read failed");
    return false;
  }

  ESP_LOGI(TAG, "FAULT_CONFIG2 HW_LOCK_ILIMIT_MODE: 0x%08X -> 0x%08X", before, after);
  const bool mode_ok = (after & mask) == (value & mask);
  if (!mode_ok) {
    ESP_LOGW(TAG, "FAULT_CONFIG2 verify mismatch: expected mask=0x%08X actual mask=0x%08X", (value & mask),
             (after & mask));
  }
  ok &= mode_ok;

  uint32_t fc1_before = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG1, fc1_before)) {
    ESP_LOGW(TAG, "FAULT_CONFIG1 read failed");
    return false;
  }

  const uint32_t fc1_mask = FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK | FAULT_CONFIG1_MTR_LCK_MODE_MASK;
  const uint32_t fc1_value = (LOCK_DISABLED_MODE << FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT) |
                             (LOCK_DISABLED_MODE << FAULT_CONFIG1_MTR_LCK_MODE_SHIFT);
  const uint32_t fc1_next = (fc1_before & ~fc1_mask) | (fc1_value & fc1_mask);
  if (fc1_next != fc1_before && !this->write_reg32(REG_FAULT_CONFIG1, fc1_next)) {
    ESP_LOGW(TAG, "FAULT_CONFIG1 write failed: 0x%08X -> 0x%08X", fc1_before, fc1_next);
    return false;
  }

  uint32_t fc1_after = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG1, fc1_after)) {
    ESP_LOGW(TAG, "FAULT_CONFIG1 verify read failed");
    return false;
  }

  ESP_LOGI(TAG, "FAULT_CONFIG1 LOCK/MTR modes: 0x%08X -> 0x%08X", fc1_before, fc1_after);
  const bool fc1_mode_ok = (fc1_after & fc1_mask) == (fc1_value & fc1_mask);
  if (!fc1_mode_ok) {
    ESP_LOGW(TAG, "FAULT_CONFIG1 verify mismatch: expected mask=0x%08X actual mask=0x%08X", (fc1_value & fc1_mask),
             (fc1_after & fc1_mask));
  }
  ok &= fc1_mode_ok;

  uint32_t s1_before = 0;
  if (!this->read_reg32(REG_MOTOR_STARTUP1, s1_before)) {
    ESP_LOGW(TAG, "MOTOR_STARTUP1 read failed");
    return false;
  }
  const uint32_t s1_mask = MOTOR_STARTUP1_MTR_STARTUP_MASK | MOTOR_STARTUP1_ALIGN_TIME_MASK;
  const uint32_t s1_value = (DEBUG_ALIGN_MTR_STARTUP << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) |
                            (DEBUG_ALIGN_TIME << MOTOR_STARTUP1_ALIGN_TIME_SHIFT);
  const uint32_t s1_next = (s1_before & ~s1_mask) | (s1_value & s1_mask);
  if (s1_next != s1_before && !this->write_reg32(REG_MOTOR_STARTUP1, s1_next)) {
    ESP_LOGW(TAG, "MOTOR_STARTUP1 write failed: 0x%08X -> 0x%08X", s1_before, s1_next);
    return false;
  }

  uint32_t s1_after = 0;
  if (!this->read_reg32(REG_MOTOR_STARTUP1, s1_after)) {
    ESP_LOGW(TAG, "MOTOR_STARTUP1 verify read failed");
    return false;
  }
  ESP_LOGI(TAG, "MOTOR_STARTUP1 mode/align-time: 0x%08X -> 0x%08X", s1_before, s1_after);
  const bool s1_ok = (s1_after & s1_mask) == (s1_value & s1_mask);
  if (!s1_ok) {
    ESP_LOGW(TAG, "MOTOR_STARTUP1 verify mismatch: expected mask=0x%08X actual mask=0x%08X", (s1_value & s1_mask),
             (s1_after & s1_mask));
  }
  ok &= s1_ok;

  if (ok) {
    ESP_LOGI(TAG, "Locks-disabled debug profile applied; pulsing CLR_FLT");
  } else {
    ESP_LOGW(TAG, "Locks-disabled debug profile partially applied; pulsing CLR_FLT");
  }
  this->pulse_clear_faults();
  return ok;
}

bool MCF8316DManualComponent::start_startup_current_sweep() {
  if (this->scope_probe_test_active_) {
    ESP_LOGW(TAG, "Scope probe test active; stopping probe sequence before startup sweep");
    this->scope_probe_test_active_ = false;
    this->scope_probe_stage_pending_ = false;
  }
  if (this->startup_sweep_active_) {
    ESP_LOGW(TAG, "Startup current sweep already active; restarting from step 1");
  } else {
    ESP_LOGI(TAG, "Starting startup current sweep (%u steps)", static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT));
  }

  if (!this->apply_startup_tune_profile()) {
    ESP_LOGW(TAG, "Startup tune baseline was partial; continuing with current sweep");
  }

  this->startup_sweep_active_ = true;
  this->startup_sweep_step_pending_ = false;
  this->startup_sweep_step_index_ = 0u;
  this->startup_sweep_pass_count_ = 0u;
  this->startup_sweep_step_start_ms_ = 0u;
  this->startup_sweep_next_step_due_ms_ = 0u;
  return this->begin_startup_sweep_step_();
}

bool MCF8316DManualComponent::start_scope_probe_test() {
  if (this->startup_sweep_active_) {
    ESP_LOGW(TAG, "Startup sweep is active; stopping sweep before scope probe test");
    this->startup_sweep_active_ = false;
    this->startup_sweep_step_pending_ = false;
  }
  if (this->scope_probe_test_active_) {
    ESP_LOGW(TAG, "Scope probe test already active; restarting from stage 1");
  } else {
    ESP_LOGI(TAG, "Starting scope probe test (%u stages)", static_cast<unsigned>(SCOPE_PROBE_STAGE_COUNT));
  }

  if (!this->apply_startup_tune_profile()) {
    ESP_LOGW(TAG, "Scope probe baseline startup tune was partial; continuing");
  }

  this->scope_probe_test_active_ = true;
  this->scope_probe_stage_pending_ = false;
  this->scope_probe_stage_index_ = 0u;
  this->scope_probe_stage_start_ms_ = 0u;
  this->scope_probe_next_stage_due_ms_ = 0u;
  return this->begin_scope_probe_stage_();
}

float MCF8316DManualComponent::scope_probe_stage_speed_percent_(uint8_t stage_index) const {
  switch (stage_index) {
    case 0:
      return 5.0f;
    case 1:
      return 8.0f;
    case 2:
    default:
      return 12.0f;
  }
}

uint32_t MCF8316DManualComponent::scope_probe_stage_hold_ms_(uint8_t stage_index) const {
  (void) stage_index;
  return 7000u;
}

bool MCF8316DManualComponent::begin_scope_probe_stage_() {
  if (!this->scope_probe_test_active_) {
    return false;
  }

  if (this->scope_probe_stage_index_ >= SCOPE_PROBE_STAGE_COUNT) {
    ESP_LOGI(TAG, "Scope probe test finished");
    this->scope_probe_test_active_ = false;
    this->scope_probe_stage_pending_ = false;
    (void) this->set_speed_percent(0.0f);
    return true;
  }

  const float speed_percent = this->scope_probe_stage_speed_percent_(this->scope_probe_stage_index_);
  const uint32_t hold_ms = this->scope_probe_stage_hold_ms_(this->scope_probe_stage_index_);

  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Scope probe stage %u failed to set speed to 0%% before configure",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u));
  }
  if (!this->set_direction_mode("cw")) {
    ESP_LOGW(TAG, "Scope probe stage %u failed to force direction cw",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u));
  }
  if (!this->set_brake_override(false)) {
    ESP_LOGW(TAG, "Scope probe stage %u failed to force brake OFF",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u));
  }
  this->pulse_clear_faults();
  if (!this->set_speed_percent(speed_percent)) {
    ESP_LOGW(TAG, "Scope probe stage %u failed to set speed to %.1f%%",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u), speed_percent);
    this->scope_probe_test_active_ = false;
    return false;
  }

  this->scope_probe_stage_start_ms_ = millis();
  ESP_LOGI(TAG, "[scope_probe] Stage %u/%u start: speed=%.1f%% hold=%ums",
           static_cast<unsigned>(this->scope_probe_stage_index_ + 1u), static_cast<unsigned>(SCOPE_PROBE_STAGE_COUNT),
           speed_percent, static_cast<unsigned>(hold_ms));
  return true;
}

void MCF8316DManualComponent::process_scope_probe_test_(bool algorithm_state_valid, uint16_t algorithm_state, bool fault_active,
                                                         bool fault_state_valid, bool controller_valid,
                                                         uint32_t controller_fault_status, uint16_t volt_mag_raw) {
  if (!this->scope_probe_test_active_) {
    return;
  }

  const uint32_t now = millis();
  if (this->scope_probe_stage_pending_) {
    if (now < this->scope_probe_next_stage_due_ms_) {
      return;
    }
    if (fault_state_valid && fault_active) {
      ESP_LOGI(TAG, "[scope_probe] waiting for fault clear before stage %u",
               static_cast<unsigned>(this->scope_probe_stage_index_ + 1u));
      this->pulse_clear_faults();
      this->scope_probe_next_stage_due_ms_ = now + SCOPE_PROBE_CLEAR_RETRY_MS;
      return;
    }
    this->scope_probe_stage_pending_ = false;
    (void) this->begin_scope_probe_stage_();
    return;
  }

  const uint32_t elapsed_ms = now - this->scope_probe_stage_start_ms_;
  const uint32_t hold_ms = this->scope_probe_stage_hold_ms_(this->scope_probe_stage_index_);
  const float speed_percent = this->scope_probe_stage_speed_percent_(this->scope_probe_stage_index_);
  const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;

  if (fault_state_valid && fault_active) {
    const uint32_t ctrl_fault_word = controller_valid ? controller_fault_status : 0u;
    ESP_LOGW(TAG,
             "[scope_probe] Stage %u FAULT: speed=%.1f%% elapsed=%ums ctrl_fault=0x%08X ctrl_valid=%s state=%s volt_mag=%.1f%%",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u), speed_percent, static_cast<unsigned>(elapsed_ms),
             ctrl_fault_word, YESNO(controller_valid),
             algorithm_state_valid ? this->algorithm_state_to_string_(algorithm_state) : "UNKNOWN", volt_mag_percent);
    (void) this->set_speed_percent(0.0f);
    this->scope_probe_stage_index_++;
    this->scope_probe_stage_pending_ = true;
    this->scope_probe_next_stage_due_ms_ = now + SCOPE_PROBE_INTER_STAGE_DELAY_MS;
    return;
  }

  if (elapsed_ms >= hold_ms) {
    ESP_LOGI(TAG, "[scope_probe] Stage %u complete: speed=%.1f%% elapsed=%ums state=%s volt_mag=%.1f%%",
             static_cast<unsigned>(this->scope_probe_stage_index_ + 1u), speed_percent, static_cast<unsigned>(elapsed_ms),
             algorithm_state_valid ? this->algorithm_state_to_string_(algorithm_state) : "UNKNOWN", volt_mag_percent);
    (void) this->set_speed_percent(0.0f);
    this->scope_probe_stage_index_++;
    this->scope_probe_stage_pending_ = true;
    this->scope_probe_next_stage_due_ms_ = now + SCOPE_PROBE_INTER_STAGE_DELAY_MS;
  }
}

uint32_t MCF8316DManualComponent::startup_sweep_current_code_(uint8_t step_index) const {
  switch (step_index) {
    case 0:
      return 3u;  // 1.0A
    case 1:
      return 4u;  // 1.5A
    case 2:
      return 5u;  // 2.0A
    case 3:
    default:
      return 6u;  // 2.5A
  }
}

float MCF8316DManualComponent::current_limit_code_to_amps_(uint32_t current_limit_code) const {
  static const float kCurrentThresholdA[16] = {0.125f, 0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f,
                                               3.5f,   4.0f,  4.5f, 5.0f, 5.5f, 6.0f, 7.0f, 8.0f};
  return kCurrentThresholdA[current_limit_code & 0xFu];
}

bool MCF8316DManualComponent::apply_startup_sweep_current_limits_(uint32_t current_limit_code) {
  const uint32_t s1_value = (current_limit_code << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT);
  const uint32_t s2_value = (current_limit_code << MOTOR_STARTUP2_OL_ILIMIT_SHIFT);
  if (!this->update_bits32(REG_MOTOR_STARTUP1, MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK, s1_value)) {
    ESP_LOGW(TAG, "Startup sweep MOTOR_STARTUP1 write failed");
    return false;
  }
  if (!this->update_bits32(REG_MOTOR_STARTUP2, MOTOR_STARTUP2_OL_ILIMIT_MASK, s2_value)) {
    ESP_LOGW(TAG, "Startup sweep MOTOR_STARTUP2 write failed");
    return false;
  }

  uint32_t startup1 = 0;
  uint32_t startup2 = 0;
  const bool startup1_ok = this->read_reg32(REG_MOTOR_STARTUP1, startup1);
  const bool startup2_ok = this->read_reg32(REG_MOTOR_STARTUP2, startup2);
  if (startup1_ok && startup2_ok) {
    const uint32_t align_ilimit =
        (startup1 & MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK) >> MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT;
    const uint32_t ol_ilimit = (startup2 & MOTOR_STARTUP2_OL_ILIMIT_MASK) >> MOTOR_STARTUP2_OL_ILIMIT_SHIFT;
    ESP_LOGI(TAG, "Startup sweep current limits: startup1=0x%08X startup2=0x%08X align_ilimit=%u(%.3gA) ol_ilimit=%u(%.3gA)",
             startup1, startup2, static_cast<unsigned>(align_ilimit), this->current_limit_code_to_amps_(align_ilimit),
             static_cast<unsigned>(ol_ilimit), this->current_limit_code_to_amps_(ol_ilimit));
  }
  return true;
}

bool MCF8316DManualComponent::begin_startup_sweep_step_() {
  if (!this->startup_sweep_active_) {
    return false;
  }

  if (this->startup_sweep_step_index_ >= STARTUP_SWEEP_STEP_COUNT) {
    ESP_LOGI(TAG, "Startup current sweep finished: %u/%u steps reached open/closed loop",
             static_cast<unsigned>(this->startup_sweep_pass_count_), static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT));
    this->startup_sweep_active_ = false;
    this->startup_sweep_step_pending_ = false;
    (void) this->set_speed_percent(0.0f);
    return true;
  }

  const uint32_t current_limit_code = this->startup_sweep_current_code_(this->startup_sweep_step_index_);
  const float current_limit_a = this->current_limit_code_to_amps_(current_limit_code);

  if (!this->set_speed_percent(0.0f)) {
    ESP_LOGW(TAG, "Startup sweep step %u failed to set speed to 0%% before configuring",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u));
  }
  if (!this->set_direction_mode("cw")) {
    ESP_LOGW(TAG, "Startup sweep step %u failed to force direction cw",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u));
  }
  if (!this->set_brake_override(false)) {
    ESP_LOGW(TAG, "Startup sweep step %u failed to force brake OFF",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u));
  }

  if (!this->apply_startup_sweep_current_limits_(current_limit_code)) {
    ESP_LOGW(TAG, "Startup sweep step %u failed to apply current limits",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u));
    this->startup_sweep_active_ = false;
    return false;
  }

  this->pulse_clear_faults();
  if (!this->set_speed_percent(STARTUP_SWEEP_SPEED_PERCENT)) {
    ESP_LOGW(TAG, "Startup sweep step %u failed to set speed to %.1f%%",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u), STARTUP_SWEEP_SPEED_PERCENT);
    this->startup_sweep_active_ = false;
    return false;
  }
  this->startup_sweep_step_start_ms_ = millis();

  ESP_LOGI(TAG, "[startup_sweep] Step %u/%u start: current_limit=%u (%.3gA), speed=%.1f%%",
           static_cast<unsigned>(this->startup_sweep_step_index_ + 1u), static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT),
           static_cast<unsigned>(current_limit_code), current_limit_a, STARTUP_SWEEP_SPEED_PERCENT);
  return true;
}

void MCF8316DManualComponent::schedule_startup_sweep_step_(uint32_t delay_ms) {
  (void) this->set_speed_percent(0.0f);
  this->startup_sweep_step_pending_ = true;
  this->startup_sweep_next_step_due_ms_ = millis() + delay_ms;
}

void MCF8316DManualComponent::process_startup_sweep_(bool algorithm_state_valid, uint16_t algorithm_state, bool fault_active,
                                                      bool fault_state_valid, bool controller_valid,
                                                      uint32_t controller_fault_status, uint16_t volt_mag_raw) {
  if (!this->startup_sweep_active_) {
    return;
  }

  const uint32_t now = millis();
  if (this->startup_sweep_step_pending_) {
    if (now < this->startup_sweep_next_step_due_ms_) {
      return;
    }
    if (fault_state_valid && fault_active) {
      ESP_LOGI(TAG, "[startup_sweep] waiting for fault clear before step %u", static_cast<unsigned>(this->startup_sweep_step_index_ + 1u));
      this->pulse_clear_faults();
      this->startup_sweep_next_step_due_ms_ = now + STARTUP_SWEEP_CLEAR_RETRY_MS;
      return;
    }
    this->startup_sweep_step_pending_ = false;
    (void) this->begin_startup_sweep_step_();
    return;
  }

  const uint32_t elapsed_ms = now - this->startup_sweep_step_start_ms_;
  const uint32_t current_limit_code = this->startup_sweep_current_code_(this->startup_sweep_step_index_);
  const float current_limit_a = this->current_limit_code_to_amps_(current_limit_code);
  const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;
  const bool reached_drive_state =
      algorithm_state_valid &&
      ((algorithm_state == ALGORITHM_STATE_OPEN_LOOP) || (algorithm_state == ALGORITHM_STATE_CLOSED_LOOP_UNALIGNED) ||
       (algorithm_state == ALGORITHM_STATE_CLOSED_LOOP_ALIGNED));

  if (reached_drive_state && !fault_active && (volt_mag_raw > 0u)) {
    ESP_LOGI(TAG, "[startup_sweep] Step %u PASS: current_limit=%u (%.3gA), elapsed=%ums, state=%s, volt_mag=%.1f%%",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u), static_cast<unsigned>(current_limit_code),
             current_limit_a, static_cast<unsigned>(elapsed_ms), this->algorithm_state_to_string_(algorithm_state),
             volt_mag_percent);
    this->startup_sweep_pass_count_++;
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
    return;
  }

  if (fault_state_valid && fault_active) {
    const uint32_t ctrl_fault_word = controller_valid ? controller_fault_status : 0u;
    ESP_LOGW(TAG,
             "[startup_sweep] Step %u FAIL: current_limit=%u (%.3gA), elapsed=%ums, ctrl_fault=0x%08X ctrl_valid=%s state=%s volt_mag=%.1f%%",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u), static_cast<unsigned>(current_limit_code),
             current_limit_a, static_cast<unsigned>(elapsed_ms), ctrl_fault_word, YESNO(controller_valid),
             algorithm_state_valid ? this->algorithm_state_to_string_(algorithm_state) : "UNKNOWN", volt_mag_percent);
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
    return;
  }

  if (elapsed_ms >= STARTUP_SWEEP_STEP_TIMEOUT_MS) {
    ESP_LOGW(TAG,
             "[startup_sweep] Step %u TIMEOUT: current_limit=%u (%.3gA), state=%s, volt_mag=%.1f%%, no drive-state transition",
             static_cast<unsigned>(this->startup_sweep_step_index_ + 1u), static_cast<unsigned>(current_limit_code),
             current_limit_a, algorithm_state_valid ? this->algorithm_state_to_string_(algorithm_state) : "UNKNOWN",
             volt_mag_percent);
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
  }
}

bool MCF8316DManualComponent::read_probe_and_publish_() {
  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
  bool fault_active = false;
  bool fault_state_valid = false;
  bool ok = true;

  const bool gate_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_fault_status);
  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  const bool controller_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, fault_status);
  ok &= algo_ok;
  ok &= gate_ok;
  ok &= controller_ok;

  if (gate_ok) {
    fault_active |= (gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }
  if (controller_ok) {
    fault_active |= (fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }

  if (algo_ok) {
    this->publish_algo_status_(algo_status);
  }
  if (gate_ok || controller_ok) {
    this->publish_faults_(gate_fault_status, gate_ok, fault_status, controller_ok);
  }
  if (fault_state_valid && this->fault_active_binary_sensor_ != nullptr) {
    this->fault_active_binary_sensor_->publish_state(fault_active);
  }

  return ok;
}

bool MCF8316DManualComponent::ensure_buck_current_limit_for_manual_() {
  uint32_t gd_config2 = 0;
  if (!this->read_reg32(REG_GD_CONFIG2, gd_config2)) {
    ESP_LOGW(TAG, "Failed to read GD_CONFIG2 for BUCK_CL check");
    return false;
  }

  const bool buck_cl_150ma = (gd_config2 & GD_CONFIG2_BUCK_CL_MASK) != 0;
  if (!buck_cl_150ma) {
    ESP_LOGI(TAG, "GD_CONFIG2 BUCK_CL already 600mA (gd2=0x%08X)", gd_config2);
    return true;
  }

  ESP_LOGW(TAG, "GD_CONFIG2 has BUCK_CL=150mA (gd2=0x%08X); setting to 600mA for manual validation", gd_config2);
  if (!this->update_bits32(REG_GD_CONFIG2, GD_CONFIG2_BUCK_CL_MASK, 0)) {
    ESP_LOGW(TAG, "Failed to write GD_CONFIG2 BUCK_CL to 600mA");
    return false;
  }

  uint32_t gd_verify = 0;
  if (!this->read_reg32(REG_GD_CONFIG2, gd_verify)) {
    ESP_LOGW(TAG, "Failed to verify GD_CONFIG2 after BUCK_CL update");
    return false;
  }
  ESP_LOGI(TAG, "GD_CONFIG2 after BUCK_CL update: 0x%08X (BUCK_CL=%s)", gd_verify,
           ((gd_verify & GD_CONFIG2_BUCK_CL_MASK) != 0) ? "150mA" : "600mA");
  return true;
}

bool MCF8316DManualComponent::seed_closed_loop_params_if_zero_() {
  uint32_t closed_loop2 = 0;
  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP2, closed_loop2) || !this->read_reg32(REG_CLOSED_LOOP3, closed_loop3) ||
      !this->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP2/3/4 for MPET seed check");
    return false;
  }

  const uint32_t motor_res =
      static_cast<uint32_t>((closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT);
  const uint32_t motor_ind =
      static_cast<uint32_t>((closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT);
  const uint32_t motor_bemf =
      static_cast<uint32_t>((closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT);
  const uint32_t spd_loop_kp_msb =
      static_cast<uint32_t>((closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT);
  const uint32_t spd_loop_kp_lsb =
      static_cast<uint32_t>((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT);
  const uint32_t spd_loop_kp = (spd_loop_kp_msb << 7) | spd_loop_kp_lsb;
  const uint32_t spd_loop_ki =
      static_cast<uint32_t>((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT);

  uint32_t closed_loop2_next = closed_loop2;
  uint32_t closed_loop3_next = closed_loop3;
  uint32_t closed_loop4_next = closed_loop4;
  bool needs_seed = false;

  if (motor_res == 0U) {
    closed_loop2_next = (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_RES_MASK) |
                        ((CLOSED_LOOP_SEED_MOTOR_RES << CLOSED_LOOP2_MOTOR_RES_SHIFT) & CLOSED_LOOP2_MOTOR_RES_MASK);
    needs_seed = true;
  }
  if (motor_ind == 0U) {
    closed_loop2_next = (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_IND_MASK) |
                        ((CLOSED_LOOP_SEED_MOTOR_IND << CLOSED_LOOP2_MOTOR_IND_SHIFT) & CLOSED_LOOP2_MOTOR_IND_MASK);
    needs_seed = true;
  }
  if (motor_bemf == 0U) {
    closed_loop3_next = (closed_loop3_next & ~CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) |
                        ((CLOSED_LOOP_SEED_MOTOR_BEMF << CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT) &
                         CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK);
    needs_seed = true;
  }
  if (spd_loop_kp == 0U) {
    closed_loop3_next = (closed_loop3_next & ~CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) |
                        (((CLOSED_LOOP_SEED_SPD_KP >> 7) << CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT) &
                         CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK);
    closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) |
                        (((CLOSED_LOOP_SEED_SPD_KP & 0x7Fu) << CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT) &
                         CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK);
    needs_seed = true;
  }
  if (spd_loop_ki == 0U) {
    closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KI_MASK) |
                        ((CLOSED_LOOP_SEED_SPD_KI << CLOSED_LOOP4_SPD_LOOP_KI_SHIFT) & CLOSED_LOOP4_SPD_LOOP_KI_MASK);
    needs_seed = true;
  }

  if (!needs_seed) {
    ESP_LOGI(TAG, "CLOSED_LOOP params already non-zero; not seeding manual startup defaults");
    return true;
  }

  ESP_LOGW(TAG,
           "Seeding zero CLOSED_LOOP params to avoid forced MPET: cl2 0x%08X->0x%08X cl3 0x%08X->0x%08X "
           "cl4 0x%08X->0x%08X",
           closed_loop2, closed_loop2_next, closed_loop3, closed_loop3_next, closed_loop4, closed_loop4_next);
  ESP_LOGW(TAG, "Seed codes: MOTOR_RES=0x%02X MOTOR_IND=0x%02X MOTOR_BEMF=0x%02X SPD_KP=%u SPD_KI=%u",
           static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_RES), static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_IND),
           static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_BEMF), static_cast<unsigned>(CLOSED_LOOP_SEED_SPD_KP),
           static_cast<unsigned>(CLOSED_LOOP_SEED_SPD_KI));

  bool ok = true;
  if (closed_loop2_next != closed_loop2) {
    ok &= this->write_reg32(REG_CLOSED_LOOP2, closed_loop2_next);
  }
  if (closed_loop3_next != closed_loop3) {
    ok &= this->write_reg32(REG_CLOSED_LOOP3, closed_loop3_next);
  }
  if (closed_loop4_next != closed_loop4) {
    ok &= this->write_reg32(REG_CLOSED_LOOP4, closed_loop4_next);
  }
  if (!ok) {
    ESP_LOGW(TAG, "Failed writing one or more seeded CLOSED_LOOP registers");
    return false;
  }

  uint32_t verify2 = 0;
  uint32_t verify3 = 0;
  uint32_t verify4 = 0;
  if (this->read_reg32(REG_CLOSED_LOOP2, verify2) && this->read_reg32(REG_CLOSED_LOOP3, verify3) &&
      this->read_reg32(REG_CLOSED_LOOP4, verify4)) {
    ESP_LOGI(TAG, "CLOSED_LOOP after seed: cl2=0x%08X cl3=0x%08X cl4=0x%08X", verify2, verify3, verify4);
  }

  return true;
}

bool MCF8316DManualComponent::perform_read_(uint16_t offset, uint32_t &value) {
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

bool MCF8316DManualComponent::perform_read16_(uint16_t offset, uint16_t &value) {
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

bool MCF8316DManualComponent::perform_write_(uint16_t offset, uint32_t value) {
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

uint32_t MCF8316DManualComponent::build_control_word_(bool is_read, uint16_t offset, bool is_32bit) const {
  const uint32_t dlen = is_32bit ? 0x1u : 0x0u;
  return ((is_read ? 1u : 0u) << 23) | (dlen << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

void MCF8316DManualComponent::delay_between_bytes_() const {
  if (this->inter_byte_delay_us_ > 0) {
    delay_microseconds_safe(this->inter_byte_delay_us_);
  }
}

void MCF8316DManualComponent::publish_faults_(uint32_t gate_fault_status, bool gate_fault_valid, uint32_t fault_status,
                                              bool controller_fault_valid) {
  std::vector<std::string> faults;
  bool gate_detail_found = false;
  bool controller_detail_found = false;
  if (gate_fault_valid) {
    if (gate_fault_status & GATE_FAULT_OCP)
      faults.emplace_back("DRV_OCP"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OVP)
      faults.emplace_back("DRV_OVP"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OTW)
      faults.emplace_back("DRV_OTW"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OTS)
      faults.emplace_back("DRV_OTS"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_HA)
      faults.emplace_back("DRV_OCP_HA"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_LA)
      faults.emplace_back("DRV_OCP_LA"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_HB)
      faults.emplace_back("DRV_OCP_HB"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_LB)
      faults.emplace_back("DRV_OCP_LB"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_HC)
      faults.emplace_back("DRV_OCP_HC"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_OCP_LC)
      faults.emplace_back("DRV_OCP_LC"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_BUCK_OCP)
      faults.emplace_back("DRV_BUCK_OCP"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_BUCK_UV)
      faults.emplace_back("DRV_BUCK_UV"), gate_detail_found = true;
    if (gate_fault_status & GATE_FAULT_VCP_UV)
      faults.emplace_back("DRV_VCP_UV"), gate_detail_found = true;
    if ((gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0 && !gate_detail_found) {
      faults.emplace_back("DRV_FAULT_ACTIVE");
    }
  }
  if (controller_fault_valid) {
    if (fault_status & FAULT_IPD_FREQ)
      faults.emplace_back("IPD_FREQ_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_IPD_T1)
      faults.emplace_back("IPD_T1_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_IPD_T2)
      faults.emplace_back("IPD_T2_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_MPET_IPD)
      faults.emplace_back("MPET_IPD_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_MPET_BEMF)
      faults.emplace_back("MPET_BEMF_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_WATCHDOG)
      faults.emplace_back("WATCHDOG_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_NO_MTR)
      faults.emplace_back("NO_MTR"), controller_detail_found = true;
    if (fault_status & FAULT_MTR_LCK)
      faults.emplace_back("MTR_LCK"), controller_detail_found = true;
    if (fault_status & FAULT_LOCK_LIMIT)
      faults.emplace_back("LOCK_LIMIT"), controller_detail_found = true;
    if (fault_status & FAULT_HW_LOCK_LIMIT)
      faults.emplace_back("HW_LOCK_LIMIT"), controller_detail_found = true;
    if (fault_status & FAULT_ABN_SPEED)
      faults.emplace_back("ABN_SPEED"), controller_detail_found = true;
    if (fault_status & FAULT_ABN_BEMF)
      faults.emplace_back("ABN_BEMF"), controller_detail_found = true;
    if (fault_status & FAULT_MTR_UNDER_VOLTAGE)
      faults.emplace_back("MTR_UNDER_VOLTAGE"), controller_detail_found = true;
    if (fault_status & FAULT_MTR_OVER_VOLTAGE)
      faults.emplace_back("MTR_OVER_VOLTAGE"), controller_detail_found = true;
    if (fault_status & FAULT_SPEED_LOOP_SATURATION)
      faults.emplace_back("SPEED_LOOP_SATURATION"), controller_detail_found = true;
    if (fault_status & FAULT_CURRENT_LOOP_SATURATION)
      faults.emplace_back("CURRENT_LOOP_SATURATION"), controller_detail_found = true;
    if (fault_status & FAULT_MAX_SPEED_SATURATION)
      faults.emplace_back("MAX_SPEED_SATURATION"), controller_detail_found = true;
    if (fault_status & FAULT_BUS_POWER_LIMIT_SATURATION)
      faults.emplace_back("BUS_POWER_LIMIT_SATURATION"), controller_detail_found = true;
    if (fault_status & FAULT_EEPROM_WRITE_LOCK_SET)
      faults.emplace_back("EEPROM_WRITE_LOCK_SET"), controller_detail_found = true;
    if (fault_status & FAULT_EEPROM_READ_LOCK_SET)
      faults.emplace_back("EEPROM_READ_LOCK_SET"), controller_detail_found = true;
    if (fault_status & FAULT_I2C_CRC)
      faults.emplace_back("I2C_CRC_FAULT_STATUS"), controller_detail_found = true;
    if (fault_status & FAULT_EEPROM_ERR)
      faults.emplace_back("EEPROM_ERR_STATUS"), controller_detail_found = true;
    if (fault_status & FAULT_BOOT_STL)
      faults.emplace_back("BOOT_STL_FAULT"), controller_detail_found = true;
    if (fault_status & FAULT_CPU_RESET)
      faults.emplace_back("CPU_RESET_FAULT_STATUS"), controller_detail_found = true;
    if (fault_status & FAULT_WWDT)
      faults.emplace_back("WWDT_FAULT_STATUS"), controller_detail_found = true;
    if ((fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0 && !controller_detail_found) {
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

void MCF8316DManualComponent::log_buck_fault_diagnostics_(const char *context, uint32_t gate_fault_status) {
  uint32_t gd_config2 = 0;
  const bool gd_ok = this->read_reg32(REG_GD_CONFIG2, gd_config2);

  const bool buck_ocp = (gate_fault_status & GATE_FAULT_BUCK_OCP) != 0;
  const bool buck_uv = (gate_fault_status & GATE_FAULT_BUCK_UV) != 0;
  const bool vcp_uv = (gate_fault_status & GATE_FAULT_VCP_UV) != 0;

  if (!gd_ok) {
    ESP_LOGW(TAG, "[%s] BUCK fault diag: gate=0x%08X buck_ocp=%s buck_uv=%s vcp_uv=%s gd_config2=READ_FAIL", context,
             gate_fault_status, YESNO(buck_ocp), YESNO(buck_uv), YESNO(vcp_uv));
    return;
  }

  const bool buck_disabled = (gd_config2 & GD_CONFIG2_BUCK_DIS_MASK) != 0;
  const bool buck_ps_disabled = (gd_config2 & GD_CONFIG2_BUCK_PS_DIS_MASK) != 0;
  const bool buck_cl_150ma = (gd_config2 & GD_CONFIG2_BUCK_CL_MASK) != 0;
  const uint32_t buck_sel = (gd_config2 & GD_CONFIG2_BUCK_SEL_MASK) >> GD_CONFIG2_BUCK_SEL_SHIFT;

  const char *buck_sel_label = "unknown";
  switch (buck_sel) {
    case 0:
      buck_sel_label = "3.3V";
      break;
    case 1:
      buck_sel_label = "5.0V";
      break;
    case 2:
      buck_sel_label = "4.0V";
      break;
    case 3:
      buck_sel_label = "5.7V";
      break;
    default:
      break;
  }

  ESP_LOGW(
      TAG,
      "[%s] BUCK fault diag: gate=0x%08X buck_ocp=%s buck_uv=%s vcp_uv=%s gd2=0x%08X buck_dis=%s buck_ps_dis=%s "
      "buck_cl=%s buck_sel=%u(%s)",
      context, gate_fault_status, YESNO(buck_ocp), YESNO(buck_uv), YESNO(vcp_uv), gd_config2, YESNO(buck_disabled),
      YESNO(buck_ps_disabled), buck_cl_150ma ? "150mA" : "600mA", static_cast<unsigned>(buck_sel), buck_sel_label);

  if (buck_disabled) {
    ESP_LOGW(TAG, "[%s] BUCK is disabled while BUCK fault bits are set; check HW config and FB_BK wiring", context);
  } else if (buck_cl_150ma) {
    ESP_LOGW(TAG, "[%s] BUCK current limit is 150mA; external load may exceed limit and cause BUCK_OCP", context);
  }
}

void MCF8316DManualComponent::log_mpet_diagnostics_(const char *context) {
  uint32_t ctrl_fault = 0;
  uint32_t algo_debug2 = 0;
  uint32_t algo_status_mpet = 0;
  uint32_t mtr_params = 0;
  uint16_t algorithm_state = 0;

  const bool ctrl_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, ctrl_fault);
  const bool dbg2_ok = this->read_reg32(REG_ALGO_DEBUG2, algo_debug2);
  const bool mpet_ok = this->read_reg32(REG_ALGO_STATUS_MPET, algo_status_mpet);
  const bool mtr_ok = this->read_reg32(REG_MTR_PARAMS, mtr_params);
  const bool state_ok = this->read_reg16(REG_ALGORITHM_STATE, algorithm_state);

  ESP_LOGI(TAG, "[%s] MPET diag: state=0x%04X(%s) ctrl=0x%08X dbg2=0x%08X mpet=0x%08X mtr=0x%08X", context,
           static_cast<unsigned>(algorithm_state), this->algorithm_state_to_string_(algorithm_state), ctrl_fault,
           algo_debug2, algo_status_mpet, mtr_params);

  if (dbg2_ok || mpet_ok || mtr_ok) {
    const bool mpet_cmd = (algo_debug2 & ALGO_DEBUG2_MPET_CMD_MASK) != 0;
    const bool mpet_r = (algo_debug2 & ALGO_DEBUG2_MPET_R_MASK) != 0;
    const bool mpet_l = (algo_debug2 & ALGO_DEBUG2_MPET_L_MASK) != 0;
    const bool mpet_ke = (algo_debug2 & ALGO_DEBUG2_MPET_KE_MASK) != 0;
    const bool mpet_mech = (algo_debug2 & ALGO_DEBUG2_MPET_MECH_MASK) != 0;
    const bool mpet_write_shadow = (algo_debug2 & ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK) != 0;
    const bool mpet_r_done = (algo_status_mpet & ALGO_STATUS_MPET_R_DONE_MASK) != 0;
    const bool mpet_l_done = (algo_status_mpet & ALGO_STATUS_MPET_L_DONE_MASK) != 0;
    const bool mpet_ke_done = (algo_status_mpet & ALGO_STATUS_MPET_KE_DONE_MASK) != 0;
    const bool mpet_mech_done = (algo_status_mpet & ALGO_STATUS_MPET_MECH_DONE_MASK) != 0;
    const uint32_t mpet_pwm_freq =
        static_cast<uint32_t>((algo_status_mpet & ALGO_STATUS_MPET_PWM_FREQ_MASK) >> ALGO_STATUS_MPET_PWM_FREQ_SHIFT);
    const uint32_t motor_r = static_cast<uint32_t>((mtr_params & MTR_PARAMS_R_MASK) >> MTR_PARAMS_R_SHIFT);
    const uint32_t motor_l = static_cast<uint32_t>((mtr_params & MTR_PARAMS_L_MASK) >> MTR_PARAMS_L_SHIFT);
    const uint32_t motor_ke = static_cast<uint32_t>((mtr_params & MTR_PARAMS_KE_MASK) >> MTR_PARAMS_KE_SHIFT);

    ESP_LOGI(TAG,
             "[%s] MPET fields: cmd=%s r=%s l=%s ke=%s mech=%s wr_shadow=%s done[r=%s l=%s ke=%s mech=%s] pwm=%u "
             "params[R=%u L=%u Ke=%u]",
             context, YESNO(mpet_cmd), YESNO(mpet_r), YESNO(mpet_l), YESNO(mpet_ke), YESNO(mpet_mech),
             YESNO(mpet_write_shadow), YESNO(mpet_r_done), YESNO(mpet_l_done), YESNO(mpet_ke_done),
             YESNO(mpet_mech_done), static_cast<unsigned>(mpet_pwm_freq), static_cast<unsigned>(motor_r),
             static_cast<unsigned>(motor_l), static_cast<unsigned>(motor_ke));

    this->log_mpet_entry_conditions_(context, algo_debug2);
  }

  if (!(ctrl_ok && dbg2_ok && mpet_ok && mtr_ok && state_ok)) {
    ESP_LOGW(TAG, "[%s] MPET diag read warning: ctrl=%s dbg2=%s mpet=%s mtr=%s state=%s", context, YESNO(ctrl_ok),
             YESNO(dbg2_ok), YESNO(mpet_ok), YESNO(mtr_ok), YESNO(state_ok));
  }
}

void MCF8316DManualComponent::log_mpet_entry_conditions_(const char *context, uint32_t algo_debug2) {
  uint32_t closed_loop2 = 0;
  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;

  const bool cl2_ok = this->read_reg32(REG_CLOSED_LOOP2, closed_loop2);
  const bool cl3_ok = this->read_reg32(REG_CLOSED_LOOP3, closed_loop3);
  const bool cl4_ok = this->read_reg32(REG_CLOSED_LOOP4, closed_loop4);

  if (cl2_ok && cl3_ok && cl4_ok) {
    const uint32_t motor_res =
        static_cast<uint32_t>((closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT);
    const uint32_t motor_ind =
        static_cast<uint32_t>((closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT);
    const uint32_t motor_bemf = static_cast<uint32_t>(
        (closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT);
    const uint32_t curr_loop_kp =
        static_cast<uint32_t>((closed_loop3 & CLOSED_LOOP3_CURR_LOOP_KP_MASK) >> CLOSED_LOOP3_CURR_LOOP_KP_SHIFT);
    const uint32_t curr_loop_ki =
        static_cast<uint32_t>((closed_loop3 & CLOSED_LOOP3_CURR_LOOP_KI_MASK) >> CLOSED_LOOP3_CURR_LOOP_KI_SHIFT);
    const uint32_t spd_loop_kp_msb = static_cast<uint32_t>(
        (closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT);
    const uint32_t spd_loop_kp_lsb = static_cast<uint32_t>(
        (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT);
    const uint32_t spd_loop_kp = (spd_loop_kp_msb << 7) | spd_loop_kp_lsb;
    const uint32_t spd_loop_ki =
        static_cast<uint32_t>((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT);
    const uint32_t max_speed =
        static_cast<uint32_t>((closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT);

    const bool mpet_cmd = (algo_debug2 & ALGO_DEBUG2_MPET_CMD_MASK) != 0;
    const bool mpet_r = (algo_debug2 & ALGO_DEBUG2_MPET_R_MASK) != 0;
    const bool mpet_l = (algo_debug2 & ALGO_DEBUG2_MPET_L_MASK) != 0;
    const bool mpet_ke = (algo_debug2 & ALGO_DEBUG2_MPET_KE_MASK) != 0;
    const bool mpet_mech = (algo_debug2 & ALGO_DEBUG2_MPET_MECH_MASK) != 0;

    const bool rl_forced_by_zero = (motor_res == 0U) || (motor_ind == 0U);
    const bool ke_forced_by_zero = (motor_bemf == 0U);
    const bool mech_forced_by_zero = (spd_loop_kp == 0U) || (spd_loop_ki == 0U);
    const bool mpet_on_nonzero_speed = mpet_r || mpet_l || mpet_ke || mpet_mech || rl_forced_by_zero ||
                                       ke_forced_by_zero || mech_forced_by_zero;

    ESP_LOGI(TAG,
             "[%s] MPET cfg: cl2=0x%08X cl3=0x%08X cl4=0x%08X motor_res=0x%02X motor_ind=0x%02X motor_bemf=0x%02X "
             "curr_kp=%u curr_ki=%u spd_kp=%u spd_ki=%u max_speed=%u",
             context, closed_loop2, closed_loop3, closed_loop4, static_cast<unsigned>(motor_res),
             static_cast<unsigned>(motor_ind), static_cast<unsigned>(motor_bemf), static_cast<unsigned>(curr_loop_kp),
             static_cast<unsigned>(curr_loop_ki), static_cast<unsigned>(spd_loop_kp), static_cast<unsigned>(spd_loop_ki),
             static_cast<unsigned>(max_speed));
    ESP_LOGI(
        TAG,
        "[%s] MPET triggers: cmd=%s bits[r=%s l=%s ke=%s mech=%s] zero_forced[rl=%s ke=%s mech=%s] enter_on_speed=%s",
        context, YESNO(mpet_cmd), YESNO(mpet_r), YESNO(mpet_l), YESNO(mpet_ke), YESNO(mpet_mech), YESNO(rl_forced_by_zero),
        YESNO(ke_forced_by_zero), YESNO(mech_forced_by_zero), YESNO(mpet_on_nonzero_speed));
  } else {
    ESP_LOGW(TAG, "[%s] MPET cfg read warning: cl2=%s cl3=%s cl4=%s", context, YESNO(cl2_ok), YESNO(cl3_ok),
             YESNO(cl4_ok));
  }
}

void MCF8316DManualComponent::log_lock_limit_diagnostics_(const char *context, uint32_t controller_fault_status) {
  uint16_t algorithm_state = 0;
  uint16_t csa_gain_feedback = 0;
  uint32_t algo_status = 0;
  uint32_t algo_debug1 = 0;
  uint32_t algo_debug2 = 0;
  uint32_t closed_loop1 = 0;
  uint32_t device_config2 = 0;
  uint32_t fault_config1 = 0;
  uint32_t fault_config2 = 0;
  uint32_t gd_config1 = 0;
  uint32_t startup1 = 0;
  uint32_t startup2 = 0;
  uint32_t isd_config = 0;
  uint32_t rev_drive_config = 0;

  const bool state_ok = this->read_reg16(REG_ALGORITHM_STATE, algorithm_state);
  const bool csa_fb_ok = this->read_reg16(REG_CSA_GAIN_FEEDBACK, csa_gain_feedback);
  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  const bool dbg1_ok = this->read_reg32(REG_ALGO_DEBUG1, algo_debug1);
  const bool dbg2_ok = this->read_reg32(REG_ALGO_DEBUG2, algo_debug2);
  const bool cl1_ok = this->read_reg32(REG_CLOSED_LOOP1, closed_loop1);
  const bool dev2_ok = this->read_reg32(REG_DEVICE_CONFIG2, device_config2);
  const bool fault_cfg_ok = this->read_reg32(REG_FAULT_CONFIG1, fault_config1);
  const bool fault_cfg2_ok = this->read_reg32(REG_FAULT_CONFIG2, fault_config2);
  const bool gd1_ok = this->read_reg32(REG_GD_CONFIG1, gd_config1);
  const bool startup1_ok = this->read_reg32(REG_MOTOR_STARTUP1, startup1);
  const bool startup2_ok = this->read_reg32(REG_MOTOR_STARTUP2, startup2);
  const bool isd_ok = this->read_reg32(REG_ISD_CONFIG, isd_config);
  const bool rev_ok = this->read_reg32(REG_REV_DRIVE_CONFIG, rev_drive_config);

  ESP_LOGW(
      TAG,
      "[%s] LOCK_LIMIT diag: ctrl=0x%08X state=0x%04X(%s) algo=0x%08X dbg1=0x%08X dbg2=0x%08X fcfg1=0x%08X "
      "fcfg2=0x%08X s1=0x%08X s2=0x%08X isd=0x%08X rev=0x%08X",
      context, controller_fault_status, static_cast<unsigned>(algorithm_state), this->algorithm_state_to_string_(algorithm_state),
      algo_status, algo_debug1, algo_debug2, fault_config1, fault_config2, startup1, startup2, isd_config,
      rev_drive_config);

  this->log_mpet_entry_conditions_(context, algo_debug2);

  if (cl1_ok && dev2_ok && gd1_ok && csa_fb_ok) {
    const uint32_t pwm_freq_code =
        static_cast<uint32_t>((closed_loop1 & CLOSED_LOOP1_PWM_FREQ_OUT_MASK) >> CLOSED_LOOP1_PWM_FREQ_OUT_SHIFT);
    const bool dynamic_csa_gain = (device_config2 & DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK) != 0;
    const bool dynamic_voltage_gain = (device_config2 & DEVICE_CONFIG2_DYNAMIC_VOLTAGE_GAIN_EN_MASK) != 0;
    const uint32_t csa_gain_cfg =
        static_cast<uint32_t>((gd_config1 & GD_CONFIG1_CSA_GAIN_MASK) >> GD_CONFIG1_CSA_GAIN_SHIFT);

    const char *pwm_freq_label = "n/a";
    switch (pwm_freq_code) {
      case 0:
        pwm_freq_label = "10kHz";
        break;
      case 1:
        pwm_freq_label = "15kHz";
        break;
      case 2:
        pwm_freq_label = "20kHz";
        break;
      case 3:
        pwm_freq_label = "25kHz";
        break;
      case 4:
        pwm_freq_label = "30kHz";
        break;
      case 5:
        pwm_freq_label = "35kHz";
        break;
      case 6:
        pwm_freq_label = "40kHz";
        break;
      case 7:
        pwm_freq_label = "45kHz";
        break;
      case 8:
        pwm_freq_label = "50kHz";
        break;
      case 9:
        pwm_freq_label = "55kHz";
        break;
      case 10:
        pwm_freq_label = "60kHz";
        break;
      default:
        break;
    }

    const char *csa_gain_cfg_label = "unknown";
    switch (csa_gain_cfg) {
      case 0:
        csa_gain_cfg_label = "0.15V/A";
        break;
      case 1:
        csa_gain_cfg_label = "0.3V/A";
        break;
      case 2:
        csa_gain_cfg_label = "0.6V/A";
        break;
      case 3:
        csa_gain_cfg_label = "1.2V/A";
        break;
      default:
        break;
    }

    const char *csa_gain_fb_label = "unknown";
    switch (csa_gain_feedback) {
      case 0:
        csa_gain_fb_label = "1.2V/A (min*8)";
        break;
      case 1:
        csa_gain_fb_label = "0.6V/A (min*4)";
        break;
      case 2:
        csa_gain_fb_label = "0.3V/A (min*2)";
        break;
      case 3:
        csa_gain_fb_label = "0.15V/A (min*1)";
        break;
      default:
        break;
    }

    ESP_LOGI(TAG,
             "[%s] DRIVE cfg: cl1=0x%08X pwm_freq=%u(%s) dev2=0x%08X dyn_csa=%s dyn_vgain=%s gd1=0x%08X "
             "csa_gain_cfg=%u(%s) csa_gain_fb=0x%04X(%s)",
             context, closed_loop1, static_cast<unsigned>(pwm_freq_code), pwm_freq_label, device_config2,
             YESNO(dynamic_csa_gain), YESNO(dynamic_voltage_gain), gd_config1, static_cast<unsigned>(csa_gain_cfg),
             csa_gain_cfg_label, static_cast<unsigned>(csa_gain_feedback), csa_gain_fb_label);
  }

  if (fault_cfg_ok) {
    const uint32_t ilimit = (fault_config1 & FAULT_CONFIG1_ILIMIT_MASK) >> FAULT_CONFIG1_ILIMIT_SHIFT;
    const uint32_t hw_lock_ilimit =
        (fault_config1 & FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT;
    const uint32_t lock_ilimit = (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_SHIFT;
    const uint32_t lock_mode =
        (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT;
    const uint32_t lock_deg =
        (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT;
    const uint32_t lck_retry = (fault_config1 & FAULT_CONFIG1_LCK_RETRY_MASK) >> FAULT_CONFIG1_LCK_RETRY_SHIFT;
    const bool lock_limit = (controller_fault_status & FAULT_LOCK_LIMIT) != 0;
    const bool hw_lock_limit = (controller_fault_status & FAULT_HW_LOCK_LIMIT) != 0;

    static const float kCurrentThresholdA[16] = {0.125f, 0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f,
                                                 3.5f,   4.0f,  4.5f, 5.0f, 5.5f, 6.0f, 7.0f, 8.0f};
    static const float kLockDegMs[16] = {0.0f,   0.1f,  0.2f, 0.5f, 1.0f, 2.5f, 5.0f, 7.5f,
                                         10.0f, 25.0f, 50.0f, 75.0f, 100.0f, 200.0f, 500.0f, 1000.0f};
    static const uint16_t kLckRetryMs[16] = {300,   500,   1000, 2000, 3000, 4000, 5000, 6000,
                                             7000,  8000,  9000, 10000, 11000, 12000, 13000, 14000};
    static const char *const kLockModeName[8] = {"latched_hiz",      "latched_ls_brake", "latched_hs_brake",
                                                 "retry_hiz",         "retry_ls_brake",   "retry_hs_brake",
                                                 "report_only",       "disabled"};

    const float ilimit_a = kCurrentThresholdA[ilimit & 0xFu];
    const float lock_ilimit_a = kCurrentThresholdA[lock_ilimit & 0xFu];
    const float hw_lock_ilimit_a = kCurrentThresholdA[hw_lock_ilimit & 0xFu];
    const float lock_deg_ms = kLockDegMs[lock_deg & 0xFu];
    const float lck_retry_s = static_cast<float>(kLckRetryMs[lck_retry & 0xFu]) / 1000.0f;
    const char *lock_mode_name = kLockModeName[lock_mode & 0x7u];

    uint32_t hw_lock_mode = 0;
    uint32_t hw_lock_deg = 0;
    const char *hw_lock_mode_name = "unknown";
    float hw_lock_deg_us = 0.0f;
    if (fault_cfg2_ok) {
      hw_lock_mode =
          (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT;
      hw_lock_deg = (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK) >> FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT;
      static const float kHwLockDegUs[8] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
      hw_lock_mode_name = kLockModeName[hw_lock_mode & 0x7u];
      hw_lock_deg_us = kHwLockDegUs[hw_lock_deg & 0x7u];
    }

    ESP_LOGW(TAG,
             "[%s] LOCK_LIMIT fields: lock=%s hw_lock=%s ILIMIT=%u(%.3gA) LOCK_ILIMIT=%u(%.3gA) "
             "HW_LOCK_ILIMIT=%u(%.3gA) LOCK_MODE=%u(%s) LOCK_DEG=%u(%.1fms) LCK_RETRY=%u(%.1fs) "
             "HW_LOCK_MODE=%u(%s) HW_LOCK_DEG=%u(%.1fus)",
             context, YESNO(lock_limit), YESNO(hw_lock_limit), static_cast<unsigned>(ilimit), ilimit_a,
             static_cast<unsigned>(lock_ilimit), lock_ilimit_a, static_cast<unsigned>(hw_lock_ilimit), hw_lock_ilimit_a,
             static_cast<unsigned>(lock_mode), lock_mode_name, static_cast<unsigned>(lock_deg), lock_deg_ms,
             static_cast<unsigned>(lck_retry), lck_retry_s, static_cast<unsigned>(hw_lock_mode), hw_lock_mode_name,
             static_cast<unsigned>(hw_lock_deg), hw_lock_deg_us);
  }

  if (!(state_ok && csa_fb_ok && algo_ok && dbg1_ok && dbg2_ok && cl1_ok && dev2_ok && fault_cfg_ok && fault_cfg2_ok &&
        gd1_ok && startup1_ok && startup2_ok && isd_ok && rev_ok)) {
    ESP_LOGW(TAG,
             "[%s] LOCK_LIMIT diag read warning: state=%s csa_fb=%s algo=%s dbg1=%s dbg2=%s cl1=%s dev2=%s fcfg1=%s "
             "fcfg2=%s gd1=%s s1=%s s2=%s isd=%s rev=%s",
             context, YESNO(state_ok), YESNO(csa_fb_ok), YESNO(algo_ok), YESNO(dbg1_ok), YESNO(dbg2_ok), YESNO(cl1_ok),
             YESNO(dev2_ok), YESNO(fault_cfg_ok), YESNO(fault_cfg2_ok), YESNO(gd1_ok), YESNO(startup1_ok),
             YESNO(startup2_ok), YESNO(isd_ok), YESNO(rev_ok));
  }
}

void MCF8316DManualComponent::log_motor_lock_diagnostics_(const char *context, uint32_t controller_fault_status) {
  uint16_t algorithm_state = 0;
  uint32_t algo_status = 0;
  uint32_t fault_config1 = 0;
  uint32_t fault_config2 = 0;
  uint32_t startup2 = 0;
  uint32_t closed_loop4 = 0;

  const bool state_ok = this->read_reg16(REG_ALGORITHM_STATE, algorithm_state);
  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  const bool fault_cfg1_ok = this->read_reg32(REG_FAULT_CONFIG1, fault_config1);
  const bool fault_cfg2_ok = this->read_reg32(REG_FAULT_CONFIG2, fault_config2);
  const bool startup2_ok = this->read_reg32(REG_MOTOR_STARTUP2, startup2);
  const bool cl4_ok = this->read_reg32(REG_CLOSED_LOOP4, closed_loop4);

  ESP_LOGW(TAG,
           "[%s] MTR_LCK diag: ctrl=0x%08X state=0x%04X(%s) algo=0x%08X fcfg1=0x%08X fcfg2=0x%08X s2=0x%08X "
           "cl4=0x%08X",
           context, controller_fault_status, static_cast<unsigned>(algorithm_state),
           this->algorithm_state_to_string_(algorithm_state), algo_status, fault_config1, fault_config2, startup2,
           closed_loop4);

  if (fault_cfg1_ok && fault_cfg2_ok && startup2_ok && cl4_ok) {
    static const char *const kLockModeName[8] = {"latched_hiz",      "latched_ls_brake", "latched_hs_brake",
                                                 "retry_hiz",         "retry_ls_brake",   "retry_hs_brake",
                                                 "report_only",       "disabled"};
    static const uint16_t kLckRetryMs[16] = {300,   500,   1000, 2000, 3000, 4000, 5000, 6000,
                                             7000,  8000,  9000, 10000, 11000, 12000, 13000, 14000};
    static const uint16_t kLockAbnSpeedPct[8] = {130, 140, 150, 160, 170, 180, 190, 200};
    static const float kAbnormalBemfPct[8] = {40.0f, 45.0f, 50.0f, 55.0f, 60.0f, 65.0f, 67.5f, 70.0f};
    static const float kSlowFirstCyclePct[16] = {0.1f, 0.3f, 0.5f, 0.7f, 1.0f, 1.5f, 2.0f, 2.5f,
                                                 3.0f, 4.0f, 5.0f, 7.5f, 10.0f, 15.0f, 20.0f, 25.0f};

    const bool mtr_lck = (controller_fault_status & FAULT_MTR_LCK) != 0;
    const bool abn_speed = (controller_fault_status & FAULT_ABN_SPEED) != 0;
    const bool abn_bemf = (controller_fault_status & FAULT_ABN_BEMF) != 0;
    const bool no_mtr = (controller_fault_status & FAULT_NO_MTR) != 0;

    const bool lock1_en = (fault_config2 & FAULT_CONFIG2_LOCK1_EN_MASK) != 0;
    const bool lock2_en = (fault_config2 & FAULT_CONFIG2_LOCK2_EN_MASK) != 0;
    const bool lock3_en = (fault_config2 & FAULT_CONFIG2_LOCK3_EN_MASK) != 0;
    const uint32_t lock_abn_speed =
        (fault_config2 & FAULT_CONFIG2_LOCK_ABN_SPEED_MASK) >> FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT;
    const uint32_t abn_bemf_thr =
        (fault_config2 & FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK) >> FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT;

    const uint32_t mtr_lck_mode =
        (fault_config1 & FAULT_CONFIG1_MTR_LCK_MODE_MASK) >> FAULT_CONFIG1_MTR_LCK_MODE_SHIFT;
    const uint32_t lck_retry = (fault_config1 & FAULT_CONFIG1_LCK_RETRY_MASK) >> FAULT_CONFIG1_LCK_RETRY_SHIFT;
    const char *mtr_lck_mode_name = kLockModeName[mtr_lck_mode & 0x7u];
    const float lck_retry_s = static_cast<float>(kLckRetryMs[lck_retry & 0xFu]) / 1000.0f;

    const bool auto_handoff = (startup2 & MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK) != 0;
    const uint32_t handoff_code =
        (startup2 & MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK) >> MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT;
    const uint32_t slow_first_code =
        (startup2 & MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK) >> MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT;
    const uint32_t max_speed = (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT;

    float handoff_pct = 0.0f;
    if (handoff_code <= 0x13u) {
      handoff_pct = static_cast<float>(handoff_code + 1u);
    } else {
      handoff_pct = 22.5f + (static_cast<float>(handoff_code) - 20.0f) * 2.5f;
    }
    const float max_speed_hz = static_cast<float>(max_speed) / 6.0f;
    const float handoff_hz = max_speed_hz * (handoff_pct / 100.0f);
    const float slow_first_pct = kSlowFirstCyclePct[slow_first_code & 0xFu];
    const float slow_first_hz = max_speed_hz * (slow_first_pct / 100.0f);

    ESP_LOGI(TAG,
             "[%s] MTR_LCK fields: mtr_lck=%s abn_speed=%s abn_bemf=%s no_mtr=%s lock_en[speed=%s bemf=%s no_mtr=%s] "
             "MTR_LCK_MODE=%u(%s) LCK_RETRY=%u(%.1fs) LOCK_ABN_SPEED=%u(%u%%) ABN_BEMF_THR=%u(%.1f%%) "
             "AUTO_HANDOFF=%s OPN_CL_HANDOFF_THR=%u(%.1f%%/%.1fHz) SLOW_FIRST_CYC_FREQ=%u(%.1f%%/%.1fHz) "
             "MAX_SPEED=%u(%.1fHz)",
             context, YESNO(mtr_lck), YESNO(abn_speed), YESNO(abn_bemf), YESNO(no_mtr), YESNO(lock1_en), YESNO(lock2_en),
             YESNO(lock3_en), static_cast<unsigned>(mtr_lck_mode), mtr_lck_mode_name, static_cast<unsigned>(lck_retry),
             lck_retry_s, static_cast<unsigned>(lock_abn_speed), static_cast<unsigned>(kLockAbnSpeedPct[lock_abn_speed & 0x7u]),
             static_cast<unsigned>(abn_bemf_thr), kAbnormalBemfPct[abn_bemf_thr & 0x7u], YESNO(auto_handoff),
             static_cast<unsigned>(handoff_code), handoff_pct, handoff_hz, static_cast<unsigned>(slow_first_code),
             slow_first_pct, slow_first_hz, static_cast<unsigned>(max_speed), max_speed_hz);
  }

  if (!(state_ok && algo_ok && fault_cfg1_ok && fault_cfg2_ok && startup2_ok && cl4_ok)) {
    ESP_LOGW(TAG, "[%s] MTR_LCK diag read warning: state=%s algo=%s fcfg1=%s fcfg2=%s s2=%s cl4=%s", context,
             YESNO(state_ok), YESNO(algo_ok), YESNO(fault_cfg1_ok), YESNO(fault_cfg2_ok), YESNO(startup2_ok),
             YESNO(cl4_ok));
  }
}

bool MCF8316DManualComponent::should_force_speed_shutdown_(uint32_t gate_fault_status, bool gate_fault_valid,
                                                            uint32_t controller_fault_status,
                                                            bool controller_fault_valid) {
  if (gate_fault_valid && ((gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0)) {
    return true;
  }

  if (!controller_fault_valid || ((controller_fault_status & CONTROLLER_FAULT_ACTIVE_MASK) == 0)) {
    return false;
  }

  uint32_t remaining_faults = controller_fault_status & ~CONTROLLER_FAULT_ACTIVE_MASK;
  if (remaining_faults == 0u) {
    return false;
  }

  const uint32_t motor_lock_faults = FAULT_MTR_LCK | FAULT_ABN_SPEED | FAULT_ABN_BEMF | FAULT_NO_MTR;
  uint32_t fault_config1 = 0;
  bool fault_config1_valid = false;
  if ((remaining_faults & (FAULT_LOCK_LIMIT | motor_lock_faults)) != 0u) {
    fault_config1_valid = this->read_reg32(REG_FAULT_CONFIG1, fault_config1);
    if (!fault_config1_valid) {
      return true;
    }
  }

  if ((remaining_faults & FAULT_LOCK_LIMIT) != 0u) {
    const uint32_t lock_mode =
        (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT;
    if (lock_mode >= LOCK_MODE_AUTO_RECOVERY_MIN) {
      remaining_faults &= ~FAULT_LOCK_LIMIT;
    }
  }

  if ((remaining_faults & motor_lock_faults) != 0u) {
    const uint32_t mtr_lock_mode =
        (fault_config1 & FAULT_CONFIG1_MTR_LCK_MODE_MASK) >> FAULT_CONFIG1_MTR_LCK_MODE_SHIFT;
    if (mtr_lock_mode >= LOCK_MODE_AUTO_RECOVERY_MIN) {
      remaining_faults &= ~motor_lock_faults;
    }
  }

  if ((remaining_faults & FAULT_HW_LOCK_LIMIT) != 0u) {
    uint32_t fault_config2 = 0;
    if (!this->read_reg32(REG_FAULT_CONFIG2, fault_config2)) {
      return true;
    }
    const uint32_t hw_lock_mode =
        (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT;
    if (hw_lock_mode >= LOCK_MODE_AUTO_RECOVERY_MIN) {
      remaining_faults &= ~FAULT_HW_LOCK_LIMIT;
    }
  }

  return remaining_faults != 0u;
}

const char *MCF8316DManualComponent::algorithm_state_to_string_(uint16_t state) const {
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

const char *MCF8316DManualComponent::brake_input_to_string_(uint32_t brake_input_value) const {
  switch (brake_input_value & 0x3u) {
    case 0x0:
      return "hardware_or_hiz";
    case 0x1:
      return "brake_on";
    case 0x2:
      return "brake_off";
    default:
      return "reserved";
  }
}

const char *MCF8316DManualComponent::direction_input_to_string_(uint32_t direction_input_value) const {
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

void MCF8316DManualComponent::log_control_diagnostics_(const char *context, uint16_t algorithm_state, uint16_t duty_raw,
                                                       uint16_t volt_mag_raw, bool fault_active) {
  uint32_t pin_config = 0;
  uint32_t peri_config1 = 0;
  uint32_t algo_debug1 = 0;
  uint32_t isd_config = 0;
  const bool pin_ok = this->read_reg32(REG_PIN_CONFIG, pin_config);
  const bool peri_ok = this->read_reg32(REG_PERI_CONFIG1, peri_config1);
  const bool dbg1_ok = this->read_reg32(REG_ALGO_DEBUG1, algo_debug1);
  const bool isd_ok = this->read_reg32(REG_ISD_CONFIG, isd_config);

  const uint32_t brake_input_value = pin_ok ? ((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 10) : 0u;
  const uint32_t direction_input_value = peri_ok ? (peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK) : 0u;
  const bool digital_override = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_OVERRIDE_MASK) != 0u);
  const uint16_t digital_speed_raw =
      dbg1_ok ? static_cast<uint16_t>((algo_debug1 & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16) : 0u;
  const bool closed_loop_disabled = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_CLOSED_LOOP_DIS_MASK) != 0u);
  const bool force_align_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ALIGN_EN_MASK) != 0u);
  const bool force_slow_first_cycle_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_SLOW_FIRST_CYCLE_EN_MASK) != 0u);
  const bool force_ipd_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_IPD_EN_MASK) != 0u);
  const bool force_isd_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ISD_EN_MASK) != 0u);
  const bool force_align_angle_src = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ALIGN_ANGLE_SRC_SEL_MASK) != 0u);
  const uint32_t isd_en = isd_ok && ((isd_config & ISD_CONFIG_ISD_EN_MASK) != 0u);
  const uint32_t isd_brake_en = isd_ok && ((isd_config & ISD_CONFIG_BRAKE_EN_MASK) != 0u);
  const uint32_t isd_brk_cfg = isd_ok && ((isd_config & ISD_CONFIG_BRK_CONFIG_MASK) != 0u);
  const uint32_t isd_brk_time = isd_ok ? ((isd_config & ISD_CONFIG_BRK_TIME_MASK) >> ISD_CONFIG_BRK_TIME_SHIFT) : 0u;

  const float duty_percent = (static_cast<float>(duty_raw) / 4095.0f) * 100.0f;
  const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;
  const float digital_speed_percent = (static_cast<float>(digital_speed_raw) / 32767.0f) * 100.0f;

  ESP_LOGI(TAG,
           "[%s] CTRL diag: state=0x%04X(%s) fault=%s duty=%.1f%% volt_mag=%.1f%% pin=0x%08X brake_sel=%u(%s) "
           "peri=0x%08X dir_sel=%u(%s) dbg1=0x%08X ovrd=%s speed_cmd=%.1f%% cl_dis=%s "
           "force[align=%s slow=%s ipd=%s isd=%s ang_src=%s] isd=0x%08X isd_en=%s isd_brk=%s isd_brk_cfg=%s "
           "isd_brk_time=%u",
           context, static_cast<unsigned>(algorithm_state), this->algorithm_state_to_string_(algorithm_state),
           YESNO(fault_active), duty_percent, volt_mag_percent, pin_config, static_cast<unsigned>(brake_input_value),
           this->brake_input_to_string_(brake_input_value), peri_config1, static_cast<unsigned>(direction_input_value),
           this->direction_input_to_string_(direction_input_value), algo_debug1, YESNO(digital_override),
           digital_speed_percent, YESNO(closed_loop_disabled), YESNO(force_align_en), YESNO(force_slow_first_cycle_en),
           YESNO(force_ipd_en), YESNO(force_isd_en), force_align_angle_src ? "forced_align_angle" : "align_angle",
           isd_config, YESNO(isd_en), YESNO(isd_brake_en), YESNO(isd_brk_cfg), static_cast<unsigned>(isd_brk_time));

  if (!(pin_ok && peri_ok && dbg1_ok && isd_ok)) {
    ESP_LOGW(TAG, "[%s] CTRL diag read warning: pin=%s peri=%s dbg1=%s isd=%s", context, YESNO(pin_ok), YESNO(peri_ok),
             YESNO(dbg1_ok), YESNO(isd_ok));
  }
}

void MCF8316DManualComponent::publish_algo_status_(uint32_t algo_status) {
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

void MCF8316DManualComponent::handle_fault_shutdown_(bool fault_active) {
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

}  // namespace mcf8316d_manual
}  // namespace esphome
