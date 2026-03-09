#include "mcf8316d_manual.h"

#include <cmath>
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

void MCF8316DManualComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up mcf8316d_manual");

  if (!this->read_probe_and_publish_()) {
    ESP_LOGW(TAG, "Read-only probe failed");
    this->status_set_warning();
  }

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
}

void MCF8316DManualComponent::update() {
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
  uint32_t vm_voltage_raw = 0;

  if (this->read_reg32(REG_ALGO_STATUS, algo_status)) {
    this->publish_algo_status_(algo_status);
  }
  if (this->read_reg32(REG_CONTROLLER_FAULT_STATUS, fault_status)) {
    const bool fault_active = (fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0;
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state(fault_active);
    }
    this->publish_faults_(fault_status);
    this->handle_fault_shutdown_(fault_active);
  }
  if (this->read_reg32(REG_VM_VOLTAGE, vm_voltage_raw) && this->vm_voltage_sensor_ != nullptr) {
    const float vm_v = static_cast<float>(vm_voltage_raw) * (60.0f / 227.0f);
    this->vm_voltage_sensor_->publish_state(vm_v);
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
}

bool MCF8316DManualComponent::read_reg32(uint16_t offset, uint32_t &value) { return this->perform_read_(offset, value); }

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
  return this->update_bits32(REG_PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value);
}

bool MCF8316DManualComponent::set_direction_mode(const std::string &direction_mode) {
  uint32_t value = PERI_CONFIG1_DIR_INPUT_HARDWARE;
  if (direction_mode == "cw") {
    value = PERI_CONFIG1_DIR_INPUT_CW;
  } else if (direction_mode == "ccw") {
    value = PERI_CONFIG1_DIR_INPUT_CCW;
  }
  return this->update_bits32(REG_PERI_CONFIG1, PERI_CONFIG1_DIR_INPUT_MASK, value);
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
  if (this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, ALGO_CTRL1_CLR_FLT_MASK)) {
    (void) this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, 0);
  }
}

void MCF8316DManualComponent::pulse_watchdog_tickle() {
  ESP_LOGD(TAG, "Pulsing watchdog tickle");
  if (this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK)) {
    (void) this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0);
  }
  this->last_watchdog_tickle_ms_ = millis();
}

bool MCF8316DManualComponent::read_probe_and_publish_() {
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
  bool ok = true;

  ok &= this->read_reg32(REG_ALGO_STATUS, algo_status);
  ok &= this->read_reg32(REG_CONTROLLER_FAULT_STATUS, fault_status);

  if (ok) {
    this->publish_algo_status_(algo_status);
    this->publish_faults_(fault_status);
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state((fault_status & CONTROLLER_FAULT_ACTIVE_MASK) != 0);
    }
  }

  return ok;
}

bool MCF8316DManualComponent::perform_read_(uint16_t offset, uint32_t &value) {
  const uint32_t control_word = this->build_control_word_(true, offset);
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

bool MCF8316DManualComponent::perform_write_(uint16_t offset, uint32_t value) {
  const uint32_t control_word = this->build_control_word_(false, offset);
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

uint32_t MCF8316DManualComponent::build_control_word_(bool is_read, uint16_t offset) const {
  const uint32_t dlen_32bit = 0x1;
  return ((is_read ? 1u : 0u) << 23) | (dlen_32bit << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

void MCF8316DManualComponent::delay_between_bytes_() const {
  if (this->inter_byte_delay_us_ > 0) {
    delay_microseconds_safe(this->inter_byte_delay_us_);
  }
}

void MCF8316DManualComponent::publish_faults_(uint32_t fault_status) {
  if (this->fault_summary_text_sensor_ == nullptr) {
    return;
  }

  std::vector<std::string> faults;
  if (fault_status & FAULT_WATCHDOG)
    faults.emplace_back("WATCHDOG_FAULT");
  if (fault_status & FAULT_NO_MTR)
    faults.emplace_back("NO_MTR");
  if (fault_status & FAULT_MTR_LCK)
    faults.emplace_back("MTR_LCK");
  if (fault_status & FAULT_ABN_SPEED)
    faults.emplace_back("ABN_SPEED");
  if (fault_status & FAULT_ABN_BEMF)
    faults.emplace_back("ABN_BEMF");
  if (fault_status & FAULT_MTR_UNDER_VOLTAGE)
    faults.emplace_back("MTR_UNDER_VOLTAGE");
  if (fault_status & FAULT_MTR_OVER_VOLTAGE)
    faults.emplace_back("MTR_OVER_VOLTAGE");
  if (fault_status & FAULT_I2C_CRC)
    faults.emplace_back("I2C_CRC_FAULT_STATUS");
  if (fault_status & FAULT_EEPROM_ERR)
    faults.emplace_back("EEPROM_ERR_STATUS");

  if (faults.empty()) {
    this->fault_summary_text_sensor_->publish_state("none");
    return;
  }

  std::string summary;
  for (size_t i = 0; i < faults.size(); i++) {
    if (i != 0) {
      summary += ",";
    }
    summary += faults[i];
  }
  this->fault_summary_text_sensor_->publish_state(summary);
}

void MCF8316DManualComponent::publish_algo_status_(uint32_t algo_status) {
  const uint16_t duty_raw = algo_status & ALGO_STATUS_DUTY_CMD_MASK;
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
