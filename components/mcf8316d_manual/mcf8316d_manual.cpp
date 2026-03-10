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

void MCF8316DApplyStartupTuneButton::press_action() {
  if (this->parent_ != nullptr) {
    if (!this->parent_->apply_startup_tune_profile()) {
      ESP_LOGW(TAG, "Startup tune profile failed");
    }
  }
}

void MCF8316DManualComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up mcf8316d_manual");
  uint32_t ctrl_fault = 0;

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

void MCF8316DManualComponent::update() {
  uint32_t gate_fault_status = 0;
  uint32_t algo_status = 0;
  uint32_t fault_status = 0;
  uint32_t vm_voltage_raw = 0;
  bool fault_active = false;
  bool fault_state_valid = false;

  if (this->read_reg32(REG_ALGO_STATUS, algo_status)) {
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
      ESP_LOGD(TAG, "Fault active but allowing retry (lock-limit-only)");
    }
    this->handle_fault_shutdown_(force_shutdown);
  }

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
  ESP_LOGW(TAG, "Applying startup tune profile (lock-limit retry + startup torque)");
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

  bool ok = true;
  ok &= apply_masked_bits(
      "FAULT_CONFIG1 tuning", REG_FAULT_CONFIG1, FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK | FAULT_CONFIG1_LCK_RETRY_MASK,
      (STARTUP_TUNE_LOCK_ILIMIT_DEG << FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT) |
          (STARTUP_TUNE_LCK_RETRY << FAULT_CONFIG1_LCK_RETRY_SHIFT));
  ok &= apply_masked_bits("MOTOR_STARTUP1 tuning", REG_MOTOR_STARTUP1,
                          MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK,
                          (STARTUP_TUNE_ALIGN_OR_SLOW_CURRENT_ILIMIT
                           << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT));
  ok &= apply_masked_bits(
      "MOTOR_STARTUP2 tuning", REG_MOTOR_STARTUP2,
      MOTOR_STARTUP2_OL_ILIMIT_MASK | MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK | MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK |
          MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK,
      (STARTUP_TUNE_OL_ILIMIT << MOTOR_STARTUP2_OL_ILIMIT_SHIFT) |
          (STARTUP_TUNE_OPN_CL_HANDOFF_THR << MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT) |
          (STARTUP_TUNE_SLOW_FIRST_CYC_FREQ << MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT) |
          (STARTUP_TUNE_FIRST_CYCLE_FREQ_SEL ? MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK : 0u));

  if (ok) {
    ESP_LOGI(TAG, "Startup tune profile applied; pulsing CLR_FLT");
  } else {
    ESP_LOGW(TAG, "Startup tune profile partially applied; pulsing CLR_FLT");
  }
  this->pulse_clear_faults();
  return ok;
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
  uint32_t algo_status = 0;
  uint32_t algo_debug1 = 0;
  uint32_t algo_debug2 = 0;
  uint32_t fault_config1 = 0;
  uint32_t startup1 = 0;
  uint32_t startup2 = 0;
  uint32_t isd_config = 0;
  uint32_t rev_drive_config = 0;

  const bool state_ok = this->read_reg16(REG_ALGORITHM_STATE, algorithm_state);
  const bool algo_ok = this->read_reg32(REG_ALGO_STATUS, algo_status);
  const bool dbg1_ok = this->read_reg32(REG_ALGO_DEBUG1, algo_debug1);
  const bool dbg2_ok = this->read_reg32(REG_ALGO_DEBUG2, algo_debug2);
  const bool fault_cfg_ok = this->read_reg32(REG_FAULT_CONFIG1, fault_config1);
  const bool startup1_ok = this->read_reg32(REG_MOTOR_STARTUP1, startup1);
  const bool startup2_ok = this->read_reg32(REG_MOTOR_STARTUP2, startup2);
  const bool isd_ok = this->read_reg32(REG_ISD_CONFIG, isd_config);
  const bool rev_ok = this->read_reg32(REG_REV_DRIVE_CONFIG, rev_drive_config);

  ESP_LOGW(
      TAG,
      "[%s] LOCK_LIMIT diag: ctrl=0x%08X state=0x%04X(%s) algo=0x%08X dbg1=0x%08X dbg2=0x%08X fcfg1=0x%08X s1=0x%08X "
      "s2=0x%08X isd=0x%08X rev=0x%08X",
      context, controller_fault_status, static_cast<unsigned>(algorithm_state), this->algorithm_state_to_string_(algorithm_state),
      algo_status, algo_debug1, algo_debug2, fault_config1, startup1, startup2, isd_config, rev_drive_config);

  this->log_mpet_entry_conditions_(context, algo_debug2);

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

    ESP_LOGW(TAG,
             "[%s] LOCK_LIMIT fields: lock=%s hw_lock=%s ILIMIT=%u(%.3gA) LOCK_ILIMIT=%u(%.3gA) "
             "HW_LOCK_ILIMIT=%u(%.3gA) LOCK_MODE=%u(%s) LOCK_DEG=%u(%.1fms) LCK_RETRY=%u(%.1fs)",
             context, YESNO(lock_limit), YESNO(hw_lock_limit), static_cast<unsigned>(ilimit), ilimit_a,
             static_cast<unsigned>(lock_ilimit), lock_ilimit_a, static_cast<unsigned>(hw_lock_ilimit), hw_lock_ilimit_a,
             static_cast<unsigned>(lock_mode), lock_mode_name, static_cast<unsigned>(lock_deg), lock_deg_ms,
             static_cast<unsigned>(lck_retry), lck_retry_s);
  }

  if (!(state_ok && algo_ok && dbg1_ok && dbg2_ok && fault_cfg_ok && startup1_ok && startup2_ok && isd_ok && rev_ok)) {
    ESP_LOGW(TAG, "[%s] LOCK_LIMIT diag read warning: state=%s algo=%s dbg1=%s dbg2=%s fcfg1=%s s1=%s s2=%s isd=%s rev=%s",
             context, YESNO(state_ok), YESNO(algo_ok), YESNO(dbg1_ok), YESNO(dbg2_ok), YESNO(fault_cfg_ok),
             YESNO(startup1_ok), YESNO(startup2_ok), YESNO(isd_ok), YESNO(rev_ok));
  }
}

bool MCF8316DManualComponent::should_force_speed_shutdown_(uint32_t gate_fault_status, bool gate_fault_valid,
                                                            uint32_t controller_fault_status,
                                                            bool controller_fault_valid) const {
  if (gate_fault_valid && ((gate_fault_status & GATE_DRIVER_FAULT_ACTIVE_MASK) != 0)) {
    return true;
  }

  if (!controller_fault_valid || ((controller_fault_status & CONTROLLER_FAULT_ACTIVE_MASK) == 0)) {
    return false;
  }

  // Allow device retry behavior for transient lock-current-limit startup faults.
  const uint32_t controller_detail_bits = controller_fault_status & ~CONTROLLER_FAULT_ACTIVE_MASK;
  const uint32_t transient_retry_faults = FAULT_LOCK_LIMIT | FAULT_HW_LOCK_LIMIT;
  return (controller_detail_bits & ~transient_retry_faults) != 0;
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
