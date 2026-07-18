#include "mcf8316d.h"

#include "../mcf83xx_common/protocol.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8316d {

using namespace ::mcf8316d_core::regs;

static const char* const TAG = "mcf8316d";

namespace {

constexpr uint32_t LOCK_MODE_AUTO_RECOVERY_MIN = 3u;

constexpr uint8_t STARTUP_COMMS_ATTEMPTS = 20u;
constexpr uint32_t STARTUP_COMMS_RETRY_DELAY_MS = 250u;
constexpr uint32_t DEFERRED_COMMS_RETRY_INTERVAL_MS = 1000u;
constexpr uint32_t DEFERRED_SCAN_INTERVAL_MS = 5000u;
constexpr uint8_t I2C_SCAN_ADDRESS_MIN = 0x00u;
constexpr uint8_t I2C_SCAN_ADDRESS_MAX = 0x7Eu;

}  // namespace

MCF8316DComponent::MCF8316DComponent() : service_(this), tuning_(this) {}

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

void MCF8316DDirectionSelect::control(const std::string& value) {
  if (this->parent_ == nullptr || !this->parent_->set_direction_mode(value)) {
    ESP_LOGW(TAG, "Failed to set direction mode to %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void MCF8316DSpeedNumber::control(float value) {
  if (this->parent_ == nullptr || !this->parent_->set_speed_percent(value)) {
    ESP_LOGW(TAG, "Failed to set speed command");
    return;
  }
  this->publish_state(value);
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

void MCF8316DComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up mcf8316d");
  this->normal_operation_ready_ = false;
  this->deferred_comms_last_retry_ms_ = 0u;
  this->deferred_comms_last_scan_ms_ = 0u;

  if (!this->scan_i2c_bus_()) {
    ESP_LOGW(TAG, "I2C scan failed; continuing with communication retries");
    this->status_set_warning();
  }
  if (!this->establish_communications_(
        STARTUP_COMMS_ATTEMPTS, STARTUP_COMMS_RETRY_DELAY_MS, true
      )) {
    ESP_LOGW(
      TAG,
      "Unable to establish communications with I2C device 0x%02X during setup; deferring normal "
      "operation",
      this->address_
    );
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  this->normal_operation_ready_ = true;
  this->apply_post_comms_setup_();
}

void MCF8316DComponent::update() {
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

  const bool algo_ok = this->read_reg32(RegisterId::ALGO_STATUS, algo_status);
  if (algo_ok) {
    this->publish_algo_status_(algo_status);
  }
  const bool gate_ok = this->read_reg32(RegisterId::GATE_DRIVER_FAULT_STATUS, gate_fault_status);
  if (gate_ok) {
    fault_active |= (gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0;
    fault_state_valid = true;
  }
  const bool controller_ok = this->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, fault_status);
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
  const uint16_t volt_mag_raw =
    (algo_status & ALGO_STATUS_VOLT_MAG_MASK) >> ALGO_STATUS_VOLT_MAG_SHIFT;
  const bool run_state_diag_active =
    algo_ok && duty_raw > 0u && volt_mag_raw > 0u && (!fault_state_valid || !fault_active);
  const bool control_diag_active = algo_ok && duty_raw > 8u;
  uint16_t algorithm_state = 0;
  bool algorithm_state_valid = false;
  const bool need_algorithm_state = this->algorithm_state_text_sensor_ != nullptr ||
                                    run_state_diag_active || control_diag_active ||
                                    this->tuning_.needs_algorithm_state();
  if (need_algorithm_state) {
    if (this->read_reg16(RegisterId::ALGORITHM_STATE, algorithm_state)) {
      algorithm_state_valid = true;
      const char* const state_name = this->algorithm_state_to_string_(algorithm_state);
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
          ESP_LOGI(
            TAG,
            "[loop_run_state] state=0x%04X(%s) duty=%.1f%% volt_mag=%.1f%%",
            algorithm_state,
            state_name,
            duty_percent,
            volt_mag_percent
          );
          this->last_run_state_diag_log_ms_ = now;
          this->last_run_state_diag_value_ = algorithm_state;
        }
      }
      if (control_diag_active) {
        const uint32_t now = millis();
        const bool should_log = (this->last_control_diag_log_ms_ == 0u) ||
                                (algorithm_state != this->last_control_diag_state_);
        if (should_log) {
          this->log_control_diagnostics_(
            "loop_control", algorithm_state, duty_raw, volt_mag_raw, fault_active
          );
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
  this->tuning_.update(
    algorithm_state_valid,
    algorithm_state,
    fault_active,
    fault_state_valid,
    controller_ok,
    fault_status,
    volt_mag_raw
  );

  const bool lock_limit_active =
    controller_ok && ((fault_status & (FAULT_LOCK_LIMIT | FAULT_HW_LOCK_LIMIT)) != 0);
  if (lock_limit_active && (!this->lock_limit_prev_active_ || (millis() - this->last_lock_limit_diag_log_ms_ >= 2000U))) {
    this->log_lock_limit_diagnostics_("loop_lock_limit", fault_status);
    this->last_lock_limit_diag_log_ms_ = millis();
  }
  if (!lock_limit_active) {
    this->last_lock_limit_diag_log_ms_ = 0;
  }
  this->lock_limit_prev_active_ = lock_limit_active;

  const bool buck_fault_active =
    gate_ok && ((gate_fault_status & (GATE_FAULT_BUCK_OCP | GATE_FAULT_BUCK_UV)) != 0);
  if (buck_fault_active && ((this->last_buck_diag_log_ms_ == 0U) || (millis() - this->last_buck_diag_log_ms_ >= 2000U))) {
    this->log_buck_fault_diagnostics_("loop_buck_fault", gate_fault_status);
    this->last_buck_diag_log_ms_ = millis();
  }
  if (!buck_fault_active) {
    this->last_buck_diag_log_ms_ = 0;
  }

  if (this->read_reg32(RegisterId::VM_VOLTAGE, vm_voltage_raw) && this->vm_voltage_sensor_ != nullptr) {
    const uint32_t vm_adc_code_8 = (vm_voltage_raw & VM_VOLTAGE_ADC_MASK) >> VM_VOLTAGE_ADC_SHIFT;
    const uint32_t vm_adc_code_q11 = (vm_voltage_raw & VM_VOLTAGE_Q11_MASK) >> VM_VOLTAGE_Q11_SHIFT;
    const float vm_v = static_cast<float>(vm_adc_code_q11) * (60.0f / 2048.0f);
    this->vm_voltage_sensor_->publish_state(vm_v);
    if ((millis() - this->last_vm_diag_log_ms_) >= 5000U) {
      ESP_LOGD(
        TAG,
        "VM decode: raw=0x%08X adc8=%u adc_q11=%u -> %.2fV",
        vm_voltage_raw,
        static_cast<unsigned>(vm_adc_code_8),
        static_cast<unsigned>(vm_adc_code_q11),
        vm_v
      );
      this->last_vm_diag_log_ms_ = millis();
    }
  }

  if (this->auto_tickle_watchdog_ && (millis() - this->last_watchdog_tickle_ms_ >= 500U)) {
    this->pulse_watchdog_tickle();
  }
}

void MCF8316DComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MCF8316D Manual Validation Component:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(
    TAG, "  Inter-byte delay: %u us", static_cast<unsigned>(this->inter_byte_delay_us_)
  );
  if (this->inter_byte_delay_us_ > 0) {
    ESP_LOGCONFIG(
      TAG, "  Note: inter-byte delay is currently not applied with ESPHome I2C transactions"
    );
  }
  ESP_LOGCONFIG(TAG, "  Auto tickle watchdog: %s", YESNO(this->auto_tickle_watchdog_));
  ESP_LOGCONFIG(
    TAG,
    "  Startup comm gate: scan 0x%02X..0x%02X, attempts=%u, retry=%ums, deferred_retry=%ums",
    static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN),
    static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX),
    static_cast<unsigned>(STARTUP_COMMS_ATTEMPTS),
    static_cast<unsigned>(STARTUP_COMMS_RETRY_DELAY_MS),
    static_cast<unsigned>(DEFERRED_COMMS_RETRY_INTERVAL_MS)
  );
}

const char* MCF8316DComponent::i2c_error_to_string_(i2c::ErrorCode error_code) const {
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

bool MCF8316DComponent::probe_device_ack_(i2c::ErrorCode& error_code) const {
  if (this->bus_ == nullptr) {
    error_code = i2c::ERROR_NOT_INITIALIZED;
    return false;
  }

  error_code = this->bus_->write_readv(this->address_, nullptr, 0, nullptr, 0);
  return error_code == i2c::ERROR_OK;
}

bool MCF8316DComponent::scan_i2c_bus_() {
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
    ESP_LOGW(
      TAG,
      "I2C scan found no ACKing devices in range 0x%02X..0x%02X",
      static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN),
      static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX)
    );
  } else {
    ESP_LOGI(
      TAG,
      "I2C scan found %u device(s): %s",
      static_cast<unsigned>(device_count),
      discovered.c_str()
    );
  }

  if (target_found) {
    ESP_LOGI(TAG, "I2C target 0x%02X was found during scan", this->address_);
  } else {
    ESP_LOGW(TAG, "I2C target 0x%02X was not found during scan", this->address_);
  }

  return device_count > 0u;
}

bool MCF8316DComponent::establish_communications_(
  uint8_t attempts, uint32_t retry_delay_ms, bool log_retry_delays
) {
  if (attempts == 0u) {
    return false;
  }

  for (uint8_t attempt = 1u; attempt <= attempts; attempt++) {
    i2c::ErrorCode ack_error = i2c::ERROR_UNKNOWN;
    if (!this->probe_device_ack_(ack_error)) {
      ESP_LOGW(
        TAG,
        "Comms attempt %u/%u: address 0x%02X probe failed: %s (%d)",
        static_cast<unsigned>(attempt),
        static_cast<unsigned>(attempts),
        this->address_,
        this->i2c_error_to_string_(ack_error),
        static_cast<int>(ack_error)
      );
    } else if (this->read_probe_and_publish_()) {
      ESP_LOGI(
        TAG,
        "I2C communications established with 0x%02X (attempt %u/%u)",
        this->address_,
        static_cast<unsigned>(attempt),
        static_cast<unsigned>(attempts)
      );
      return true;
    } else {
      ESP_LOGW(
        TAG,
        "Comms attempt %u/%u: address 0x%02X ACKed but register probe failed",
        static_cast<unsigned>(attempt),
        static_cast<unsigned>(attempts),
        this->address_
      );
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

void MCF8316DComponent::process_deferred_startup_() {
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

  if (this->deferred_comms_last_retry_ms_ != 0u && (now - this->deferred_comms_last_retry_ms_) < DEFERRED_COMMS_RETRY_INTERVAL_MS) {
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

void MCF8316DComponent::apply_post_comms_setup_() {
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

  this->tuning_.apply_post_comms_setup();
}

bool MCF8316DComponent::read_reg32(RegisterId id, uint32_t& value) {
  return this->service_.read_reg32(id, value);
}

bool MCF8316DComponent::read_reg16(RegisterId id, uint16_t& value) {
  return this->service_.read_reg16(id, value);
}

bool MCF8316DComponent::write_reg32(RegisterId id, uint32_t value) {
  return this->service_.write_reg32(id, value);
}

bool MCF8316DComponent::update_bits32(RegisterId id, uint32_t mask, uint32_t value) {
  return this->service_.update_bits32(id, mask, value);
}

bool MCF8316DComponent::read_register32(uint16_t offset, uint32_t *value) {
  if (value == nullptr) {
    return false;
  }
  const auto command = ::mcf83xx_common::make_control_frame(
      true, offset, ::mcf83xx_common::RegisterWidth::BITS_32);
  uint8_t rx[4] = {0, 0, 0, 0};
  const i2c::ErrorCode err = this->write_read(command.data(), command.size(), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "read_reg32(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }
  *value = ::mcf83xx_common::decode_read32(rx);
  return true;
}

bool MCF8316DComponent::read_register16(uint16_t offset, uint16_t *value) {
  if (value == nullptr) {
    return false;
  }
  const auto command = ::mcf83xx_common::make_control_frame(
      true, offset, ::mcf83xx_common::RegisterWidth::BITS_16);
  uint8_t rx[2] = {0, 0};
  const i2c::ErrorCode err = this->write_read(command.data(), command.size(), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "read_reg16(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }
  *value = ::mcf83xx_common::decode_read16(rx);
  return true;
}

bool MCF8316DComponent::write_register32(uint16_t offset, uint32_t value) {
  const auto frame = ::mcf83xx_common::make_write32_frame(offset, value);
  const i2c::ErrorCode err = this->write(frame.data(), frame.size());
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(
      TAG, "write_reg32(0x%04X, 0x%08X) failed: i2c error %d", offset, value, static_cast<int>(err)
    );
    return false;
  }
  return true;
}

void MCF8316DComponent::delay_microseconds(uint32_t delay_us) {
  delay_microseconds_safe(delay_us);
}

bool MCF8316DComponent::set_brake_override(bool brake_on) {
  if (!this->service_.set_brake_input(brake_on)) {
    return false;
  }

  uint8_t brake_input_value = 0u;
  uint32_t pin_config = 0;
  if (this->service_.read_brake_input(brake_input_value) && this->read_reg32(RegisterId::PIN_CONFIG, pin_config)) {
    ESP_LOGI(
      TAG,
      "Brake override write: request=%s pin_cfg=0x%08X brake_input=%u(%s)",
      brake_on ? "ON" : "OFF",
      pin_config,
      static_cast<unsigned>(brake_input_value),
      this->brake_input_to_string_(brake_input_value)
    );
  }
  return true;
}

bool MCF8316DComponent::set_direction_mode(const std::string& direction_mode) {
  ::mcf8316d_core::DirectionInputMode mode = ::mcf8316d_core::DirectionInputMode::HARDWARE;
  if (direction_mode == "cw") {
    mode = ::mcf8316d_core::DirectionInputMode::CW;
  } else if (direction_mode == "ccw") {
    mode = ::mcf8316d_core::DirectionInputMode::CCW;
  }
  if (!this->service_.set_direction_input(mode)) {
    return false;
  }

  uint8_t direction_input_value = 0u;
  uint32_t peri_config1 = 0;
  if (this->service_.read_direction_input(direction_input_value) && this->read_reg32(RegisterId::PERI_CONFIG1, peri_config1)) {
    ESP_LOGI(
      TAG,
      "Direction write: request=%s peri_cfg1=0x%08X dir_input=%u(%s)",
      direction_mode.c_str(),
      peri_config1,
      static_cast<unsigned>(direction_input_value),
      this->direction_input_to_string_(direction_input_value)
    );
  }
  return true;
}

bool MCF8316DComponent::set_speed_percent(float speed_percent) {
  if (!this->service_.write_speed_command_percent(speed_percent)) {
    return false;
  }

  const float clamped = clamp(speed_percent, 0.0f, 100.0f);
  if (this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(clamped);
  }
  return true;
}

void MCF8316DComponent::pulse_clear_faults() {
  ESP_LOGI(TAG, "Pulsing clear faults");
  uint32_t gate_before = 0;
  uint32_t ctrl_before = 0;
  uint32_t gate_after = 0;
  uint32_t ctrl_after = 0;
  const bool gate_before_ok = this->read_reg32(RegisterId::GATE_DRIVER_FAULT_STATUS, gate_before);
  const bool ctrl_before_ok = this->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, ctrl_before);

  if (!this->service_.pulse_clear_faults()) {
    ESP_LOGW(TAG, "Failed to pulse CLR_FLT");
    return;
  }

  const bool gate_after_ok = this->read_reg32(RegisterId::GATE_DRIVER_FAULT_STATUS, gate_after);
  const bool ctrl_after_ok = this->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, ctrl_after);

  if (gate_before_ok || ctrl_before_ok || gate_after_ok || ctrl_after_ok) {
    ESP_LOGI(
      TAG,
      "CLR_FLT status: gate 0x%08X -> 0x%08X, ctrl 0x%08X -> 0x%08X",
      gate_before,
      gate_after,
      ctrl_before,
      ctrl_after
    );
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

void MCF8316DComponent::pulse_watchdog_tickle() {
  ESP_LOGD(TAG, "Pulsing watchdog tickle");
  if (!this->service_.pulse_watchdog_tickle()) {
    ESP_LOGW(TAG, "Failed to pulse watchdog tickle");
  }
  this->last_watchdog_tickle_ms_ = millis();
}

bool MCF8316DComponent::apply_startup_tune_profile() { return this->tuning_.apply_startup_tune_profile(); }

bool MCF8316DComponent::apply_hw_lock_report_only_profile() {
  return this->tuning_.apply_hw_lock_report_only_profile();
}

bool MCF8316DComponent::start_startup_current_sweep() {
  return this->tuning_.start_startup_current_sweep();
}

bool MCF8316DComponent::start_scope_probe_test() { return this->tuning_.start_scope_probe_test(); }

bool MCF8316DComponent::read_probe_and_publish_() {
  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
  bool fault_active = false;
  bool fault_state_valid = false;
  bool ok = true;

  const bool gate_ok = this->read_reg32(RegisterId::GATE_DRIVER_FAULT_STATUS, gate_fault_status);
  const bool algo_ok = this->read_reg32(RegisterId::ALGO_STATUS, algo_status);
  const bool controller_ok = this->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, fault_status);
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


void MCF8316DComponent::publish_faults_(
  uint32_t gate_fault_status,
  bool gate_fault_valid,
  uint32_t fault_status,
  bool controller_fault_valid
) {
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

void MCF8316DComponent::log_buck_fault_diagnostics_(
  const char* context, uint32_t gate_fault_status
) {
  uint32_t gd_config2 = 0;
  const bool gd_ok = this->read_reg32(RegisterId::GD_CONFIG2, gd_config2);

  const bool buck_ocp = (gate_fault_status & GATE_FAULT_BUCK_OCP) != 0;
  const bool buck_uv = (gate_fault_status & GATE_FAULT_BUCK_UV) != 0;
  const bool vcp_uv = (gate_fault_status & GATE_FAULT_VCP_UV) != 0;

  if (!gd_ok) {
    ESP_LOGW(
      TAG,
      "[%s] BUCK fault diag: gate=0x%08X buck_ocp=%s buck_uv=%s vcp_uv=%s gd_config2=READ_FAIL",
      context,
      gate_fault_status,
      YESNO(buck_ocp),
      YESNO(buck_uv),
      YESNO(vcp_uv)
    );
    return;
  }

  const bool buck_disabled = (gd_config2 & GD_CONFIG2_BUCK_DIS_MASK) != 0;
  const bool buck_ps_disabled = (gd_config2 & GD_CONFIG2_BUCK_PS_DIS_MASK) != 0;
  const bool buck_cl_150ma = (gd_config2 & GD_CONFIG2_BUCK_CL_MASK) != 0;
  const uint32_t buck_sel = (gd_config2 & GD_CONFIG2_BUCK_SEL_MASK) >> GD_CONFIG2_BUCK_SEL_SHIFT;



  const char* buck_sel_label = "unknown";
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
    "[%s] BUCK fault diag: gate=0x%08X buck_ocp=%s buck_uv=%s vcp_uv=%s gd2=0x%08X buck_dis=%s "
    "buck_ps_dis=%s "
    "buck_cl=%s buck_sel=%u(%s)",
    context,
    gate_fault_status,
    YESNO(buck_ocp),
    YESNO(buck_uv),
    YESNO(vcp_uv),
    gd_config2,
    YESNO(buck_disabled),
    YESNO(buck_ps_disabled),
    buck_cl_150ma ? "150mA" : "600mA",
    static_cast<unsigned>(buck_sel),
    buck_sel_label
  );

  if (buck_disabled) {
    ESP_LOGW(
      TAG,
      "[%s] BUCK is disabled while BUCK fault bits are set; check HW config and FB_BK wiring",
      context
    );
  } else if (buck_cl_150ma) {
    ESP_LOGW(
      TAG,
      "[%s] BUCK current limit is 150mA; external load may exceed limit and cause BUCK_OCP",
      context
    );
  }
}

void MCF8316DComponent::log_lock_limit_diagnostics_(
  const char* context, uint32_t controller_fault_status
) {
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

  const bool state_ok = this->read_reg16(RegisterId::ALGORITHM_STATE, algorithm_state);
  const bool csa_fb_ok = this->read_reg16(RegisterId::CSA_GAIN_FEEDBACK, csa_gain_feedback);
  const bool algo_ok = this->read_reg32(RegisterId::ALGO_STATUS, algo_status);
  const bool dbg1_ok = this->read_reg32(RegisterId::ALGO_DEBUG1, algo_debug1);
  const bool dbg2_ok = this->read_reg32(RegisterId::ALGO_DEBUG2, algo_debug2);
  const bool cl1_ok = this->read_reg32(RegisterId::CLOSED_LOOP1, closed_loop1);
  const bool dev2_ok = this->read_reg32(RegisterId::DEVICE_CONFIG2, device_config2);
  const bool fault_cfg_ok = this->read_reg32(RegisterId::FAULT_CONFIG1, fault_config1);
  const bool fault_cfg2_ok = this->read_reg32(RegisterId::FAULT_CONFIG2, fault_config2);
  const bool gd1_ok = this->read_reg32(RegisterId::GD_CONFIG1, gd_config1);
  const bool startup1_ok = this->read_reg32(RegisterId::MOTOR_STARTUP1, startup1);
  const bool startup2_ok = this->read_reg32(RegisterId::MOTOR_STARTUP2, startup2);
  const bool isd_ok = this->read_reg32(RegisterId::ISD_CONFIG, isd_config);
  const bool rev_ok = this->read_reg32(RegisterId::REV_DRIVE_CONFIG, rev_drive_config);

  ESP_LOGW(
    TAG,
    "[%s] LOCK_LIMIT diag: ctrl=0x%08X state=0x%04X(%s) algo=0x%08X dbg1=0x%08X dbg2=0x%08X "
    "fcfg1=0x%08X "
    "fcfg2=0x%08X s1=0x%08X s2=0x%08X isd=0x%08X rev=0x%08X",
    context,
    controller_fault_status,
    static_cast<unsigned>(algorithm_state),
    this->algorithm_state_to_string_(algorithm_state),
    algo_status,
    algo_debug1,
    algo_debug2,
    fault_config1,
    fault_config2,
    startup1,
    startup2,
    isd_config,
    rev_drive_config
  );

  this->tuning_.log_mpet_entry_conditions(context, algo_debug2);

  if (cl1_ok && dev2_ok && gd1_ok && csa_fb_ok) {
    const uint32_t pwm_freq_code = static_cast<uint32_t>(
      (closed_loop1 & CLOSED_LOOP1_PWM_FREQ_OUT_MASK) >> CLOSED_LOOP1_PWM_FREQ_OUT_SHIFT
    );
    const bool dynamic_csa_gain = (device_config2 & DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK) != 0;
    const bool dynamic_voltage_gain =
      (device_config2 & DEVICE_CONFIG2_DYNAMIC_VOLTAGE_GAIN_EN_MASK) != 0;
    const uint32_t csa_gain_cfg =
      static_cast<uint32_t>((gd_config1 & GD_CONFIG1_CSA_GAIN_MASK) >> GD_CONFIG1_CSA_GAIN_SHIFT);
    // TODO: these shouldbt be in here
    const char* pwm_freq_label = "n/a";
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

    const char* csa_gain_cfg_label = "unknown";
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

    const char* csa_gain_fb_label = "unknown";
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

    ESP_LOGI(
      TAG,
      "[%s] DRIVE cfg: cl1=0x%08X pwm_freq=%u(%s) dev2=0x%08X dyn_csa=%s dyn_vgain=%s gd1=0x%08X "
      "csa_gain_cfg=%u(%s) csa_gain_fb=0x%04X(%s)",
      context,
      closed_loop1,
      static_cast<unsigned>(pwm_freq_code),
      pwm_freq_label,
      device_config2,
      YESNO(dynamic_csa_gain),
      YESNO(dynamic_voltage_gain),
      gd_config1,
      static_cast<unsigned>(csa_gain_cfg),
      csa_gain_cfg_label,
      static_cast<unsigned>(csa_gain_feedback),
      csa_gain_fb_label
    );
  }

  if (fault_cfg_ok) {
    const uint32_t ilimit =
      (fault_config1 & FAULT_CONFIG1_ILIMIT_MASK) >> FAULT_CONFIG1_ILIMIT_SHIFT;
    const uint32_t hw_lock_ilimit =
      (fault_config1 & FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT;
    const uint32_t lock_ilimit =
      (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_SHIFT;
    const uint32_t lock_mode =
      (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT;
    const uint32_t lock_deg =
      (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT;
    const uint32_t lck_retry =
      (fault_config1 & FAULT_CONFIG1_LCK_RETRY_MASK) >> FAULT_CONFIG1_LCK_RETRY_SHIFT;
    const bool lock_limit = (controller_fault_status & FAULT_LOCK_LIMIT) != 0;
    const bool hw_lock_limit = (controller_fault_status & FAULT_HW_LOCK_LIMIT) != 0;

    // TODO: these shouldbt be in here
    static const float kCurrentThresholdA[16] = {
      0.125f,
      0.25f,
      0.5f,
      1.0f,
      1.5f,
      2.0f,
      2.5f,
      3.0f,
      3.5f,
      4.0f,
      4.5f,
      5.0f,
      5.5f,
      6.0f,
      7.0f,
      8.0f};
    static const float kLockDegMs[16] = {
      0.0f,
      0.1f,
      0.2f,
      0.5f,
      1.0f,
      2.5f,
      5.0f,
      7.5f,
      10.0f,
      25.0f,
      50.0f,
      75.0f,
      100.0f,
      200.0f,
      500.0f,
      1000.0f};
    static const uint16_t kLckRetryMs[16] = {
      300,
      500,
      1000,
      2000,
      3000,
      4000,
      5000,
      6000,
      7000,
      8000,
      9000,
      10000,
      11000,
      12000,
      13000,
      14000};
    static const char* const kLockModeName[8] = {
      "latched_hiz",
      "latched_ls_brake",
      "latched_hs_brake",
      "retry_hiz",
      "retry_ls_brake",
      "retry_hs_brake",
      "report_only",
      "disabled"};

    const float ilimit_a = kCurrentThresholdA[ilimit & 0xFu];
    const float lock_ilimit_a = kCurrentThresholdA[lock_ilimit & 0xFu];
    const float hw_lock_ilimit_a = kCurrentThresholdA[hw_lock_ilimit & 0xFu];
    const float lock_deg_ms = kLockDegMs[lock_deg & 0xFu];
    const float lck_retry_s = static_cast<float>(kLckRetryMs[lck_retry & 0xFu]) / 1000.0f;
    const char* lock_mode_name = kLockModeName[lock_mode & 0x7u];

    uint32_t hw_lock_mode = 0;
    uint32_t hw_lock_deg = 0;
    const char* hw_lock_mode_name = "unknown";
    float hw_lock_deg_us = 0.0f;
    if (fault_cfg2_ok) {
      hw_lock_mode = (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >>
                     FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT;
      hw_lock_deg = (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK) >>
                    FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT;
      static const float kHwLockDegUs[8] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
      hw_lock_mode_name = kLockModeName[hw_lock_mode & 0x7u];
      hw_lock_deg_us = kHwLockDegUs[hw_lock_deg & 0x7u];
    }

    ESP_LOGW(
      TAG,
      "[%s] LOCK_LIMIT fields: lock=%s hw_lock=%s ILIMIT=%u(%.3gA) LOCK_ILIMIT=%u(%.3gA) "
      "HW_LOCK_ILIMIT=%u(%.3gA) LOCK_MODE=%u(%s) LOCK_DEG=%u(%.1fms) LCK_RETRY=%u(%.1fs) "
      "HW_LOCK_MODE=%u(%s) HW_LOCK_DEG=%u(%.1fus)",
      context,
      YESNO(lock_limit),
      YESNO(hw_lock_limit),
      static_cast<unsigned>(ilimit),
      ilimit_a,
      static_cast<unsigned>(lock_ilimit),
      lock_ilimit_a,
      static_cast<unsigned>(hw_lock_ilimit),
      hw_lock_ilimit_a,
      static_cast<unsigned>(lock_mode),
      lock_mode_name,
      static_cast<unsigned>(lock_deg),
      lock_deg_ms,
      static_cast<unsigned>(lck_retry),
      lck_retry_s,
      static_cast<unsigned>(hw_lock_mode),
      hw_lock_mode_name,
      static_cast<unsigned>(hw_lock_deg),
      hw_lock_deg_us
    );
  }

  if (!(state_ok && csa_fb_ok && algo_ok && dbg1_ok && dbg2_ok && cl1_ok && dev2_ok &&
        fault_cfg_ok && fault_cfg2_ok && gd1_ok && startup1_ok && startup2_ok && isd_ok &&
        rev_ok)) {
    ESP_LOGW(
      TAG,
      "[%s] LOCK_LIMIT diag read warning: state=%s csa_fb=%s algo=%s dbg1=%s dbg2=%s cl1=%s "
      "dev2=%s fcfg1=%s "
      "fcfg2=%s gd1=%s s1=%s s2=%s isd=%s rev=%s",
      context,
      YESNO(state_ok),
      YESNO(csa_fb_ok),
      YESNO(algo_ok),
      YESNO(dbg1_ok),
      YESNO(dbg2_ok),
      YESNO(cl1_ok),
      YESNO(dev2_ok),
      YESNO(fault_cfg_ok),
      YESNO(fault_cfg2_ok),
      YESNO(gd1_ok),
      YESNO(startup1_ok),
      YESNO(startup2_ok),
      YESNO(isd_ok),
      YESNO(rev_ok)
    );
  }
}

bool MCF8316DComponent::should_force_speed_shutdown_(
  uint32_t gate_fault_status,
  bool gate_fault_valid,
  uint32_t controller_fault_status,
  bool controller_fault_valid
) {
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

  const uint32_t motor_lock_faults =
    FAULT_MTR_LCK | FAULT_ABN_SPEED | FAULT_ABN_BEMF | FAULT_NO_MTR;
  uint32_t fault_config1 = 0;
  bool fault_config1_valid = false;
  if ((remaining_faults & (FAULT_LOCK_LIMIT | motor_lock_faults)) != 0u) {
    fault_config1_valid = this->read_reg32(RegisterId::FAULT_CONFIG1, fault_config1);
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
    if (!this->read_reg32(RegisterId::FAULT_CONFIG2, fault_config2)) {
      return true;
    }
    const uint32_t hw_lock_mode = (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >>
                                  FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT;
    if (hw_lock_mode >= LOCK_MODE_AUTO_RECOVERY_MIN) {
      remaining_faults &= ~FAULT_HW_LOCK_LIMIT;
    }
  }

  return remaining_faults != 0u;
}

const char* MCF8316DComponent::algorithm_state_to_string_(uint16_t state) const {
  return ::mcf8316d_core::algorithm_state_to_string(state);
}

const char* MCF8316DComponent::brake_input_to_string_(uint32_t brake_input_value) const {
  return ::mcf8316d_core::brake_input_to_string(brake_input_value);
}

const char* MCF8316DComponent::direction_input_to_string_(uint32_t direction_input_value) const {
  return ::mcf8316d_core::direction_input_to_string(direction_input_value);
}

void MCF8316DComponent::log_control_diagnostics_(
  const char* context,
  uint16_t algorithm_state,
  uint16_t duty_raw,
  uint16_t volt_mag_raw,
  bool fault_active
) {
  uint32_t pin_config = 0;
  uint32_t peri_config1 = 0;
  uint32_t algo_debug1 = 0;
  uint32_t isd_config = 0;
  const bool pin_ok = this->read_reg32(RegisterId::PIN_CONFIG, pin_config);
  const bool peri_ok = this->read_reg32(RegisterId::PERI_CONFIG1, peri_config1);
  const bool dbg1_ok = this->read_reg32(RegisterId::ALGO_DEBUG1, algo_debug1);
  const bool isd_ok = this->read_reg32(RegisterId::ISD_CONFIG, isd_config);

  const uint32_t brake_input_value =
    pin_ok ? ((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 10) : 0u;
  const uint32_t direction_input_value =
    peri_ok ? (peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK) : 0u;
  const bool digital_override = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_OVERRIDE_MASK) != 0u);
  const uint16_t digital_speed_raw =
    dbg1_ok ? static_cast<uint16_t>((algo_debug1 & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16) : 0u;
  const bool closed_loop_disabled =
    dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_CLOSED_LOOP_DIS_MASK) != 0u);
  const bool force_align_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ALIGN_EN_MASK) != 0u);
  const bool force_slow_first_cycle_en =
    dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_SLOW_FIRST_CYCLE_EN_MASK) != 0u);
  const bool force_ipd_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_IPD_EN_MASK) != 0u);
  const bool force_isd_en = dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ISD_EN_MASK) != 0u);
  const bool force_align_angle_src =
    dbg1_ok && ((algo_debug1 & ALGO_DEBUG1_FORCE_ALIGN_ANGLE_SRC_SEL_MASK) != 0u);
  const uint32_t isd_en = isd_ok && ((isd_config & ISD_CONFIG_ISD_EN_MASK) != 0u);
  const uint32_t isd_brake_en = isd_ok && ((isd_config & ISD_CONFIG_BRAKE_EN_MASK) != 0u);
  const uint32_t isd_brk_cfg = isd_ok && ((isd_config & ISD_CONFIG_BRK_CONFIG_MASK) != 0u);
  const uint32_t isd_brk_time =
    isd_ok ? ((isd_config & ISD_CONFIG_BRK_TIME_MASK) >> ISD_CONFIG_BRK_TIME_SHIFT) : 0u;

  const float duty_percent = (static_cast<float>(duty_raw) / 4095.0f) * 100.0f;
  const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;
  const float digital_speed_percent = (static_cast<float>(digital_speed_raw) / 32767.0f) * 100.0f;

  ESP_LOGI(
    TAG,
    "[%s] CTRL diag: state=0x%04X(%s) fault=%s duty=%.1f%% volt_mag=%.1f%% pin=0x%08X "
    "brake_sel=%u(%s) "
    "peri=0x%08X dir_sel=%u(%s) dbg1=0x%08X ovrd=%s speed_cmd=%.1f%% cl_dis=%s "
    "force[align=%s slow=%s ipd=%s isd=%s ang_src=%s] isd=0x%08X isd_en=%s isd_brk=%s "
    "isd_brk_cfg=%s "
    "isd_brk_time=%u",
    context,
    static_cast<unsigned>(algorithm_state),
    this->algorithm_state_to_string_(algorithm_state),
    YESNO(fault_active),
    duty_percent,
    volt_mag_percent,
    pin_config,
    static_cast<unsigned>(brake_input_value),
    this->brake_input_to_string_(brake_input_value),
    peri_config1,
    static_cast<unsigned>(direction_input_value),
    this->direction_input_to_string_(direction_input_value),
    algo_debug1,
    YESNO(digital_override),
    digital_speed_percent,
    YESNO(closed_loop_disabled),
    YESNO(force_align_en),
    YESNO(force_slow_first_cycle_en),
    YESNO(force_ipd_en),
    YESNO(force_isd_en),
    force_align_angle_src ? "forced_align_angle" : "align_angle",
    isd_config,
    YESNO(isd_en),
    YESNO(isd_brake_en),
    YESNO(isd_brk_cfg),
    static_cast<unsigned>(isd_brk_time)
  );

  if (!(pin_ok && peri_ok && dbg1_ok && isd_ok)) {
    ESP_LOGW(
      TAG,
      "[%s] CTRL diag read warning: pin=%s peri=%s dbg1=%s isd=%s",
      context,
      YESNO(pin_ok),
      YESNO(peri_ok),
      YESNO(dbg1_ok),
      YESNO(isd_ok)
    );
  }
}

void MCF8316DComponent::publish_algo_status_(uint32_t algo_status) {
  const uint16_t duty_raw = (algo_status & ALGO_STATUS_DUTY_CMD_MASK) >> ALGO_STATUS_DUTY_CMD_SHIFT;
  const uint16_t volt_mag_raw =
    (algo_status & ALGO_STATUS_VOLT_MAG_MASK) >> ALGO_STATUS_VOLT_MAG_SHIFT;

  if (this->duty_cmd_percent_sensor_ != nullptr) {
    this->duty_cmd_percent_sensor_->publish_state(
      (static_cast<float>(duty_raw) / 4095.0f) * 100.0f
    );
  }
  if (this->volt_mag_percent_sensor_ != nullptr) {
    this->volt_mag_percent_sensor_->publish_state(
      (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f
    );
  }
  if (this->sys_enable_binary_sensor_ != nullptr) {
    this->sys_enable_binary_sensor_->publish_state(
      (algo_status & ALGO_STATUS_SYS_ENABLE_FLAG_MASK) != 0
    );
  }
}

void MCF8316DComponent::handle_fault_shutdown_(bool fault_active) {
  if (!fault_active) {
    this->fault_latched_ = false;
    return;
  }
  if (this->fault_latched_) {
    return;
  }

  this->fault_latched_ = true;
  ESP_LOGW(TAG, "Fault detected, forcing speed command to 0%%");
  (void)this->set_speed_percent(0.0f);
}

}  // namespace mcf8316d
}  // namespace esphome
