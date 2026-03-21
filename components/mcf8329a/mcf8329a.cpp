#include "mcf8329a.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8329a {

static const char* const TAG = "mcf8329a";
static constexpr uint8_t LOCK_ILIMIT_PERCENT_TABLE[16] = {
  5,
  10,
  15,
  20,
  25,
  30,
  40,
  50,
  60,
  65,
  70,
  75,
  80,
  85,
  90,
  95,
};
static const char* const LOCK_ABN_SPEED_THRESHOLD_LABELS[8] = {
  "130%",
  "140%",
  "150%",
  "160%",
  "170%",
  "180%",
  "190%",
  "200%",
};
static const char* const ABNORMAL_BEMF_THRESHOLD_LABELS[8] = {
  "40%",
  "45%",
  "50%",
  "55%",
  "60%",
  "65%",
  "67.5%",
  "70%",
};
static const char* const NO_MOTOR_THRESHOLD_LABELS[8] = {
  "1%",
  "2%",
  "3%",
  "4%",
  "5%",
  "7.5%",
  "10%",
  "20%",
};
static constexpr float OPEN_LOOP_ACCEL_HZ_PER_S_TABLE[16] = {
  0.01f,
  0.05f,
  1.0f,
  2.5f,
  5.0f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  250.0f,
  500.0f,
  750.0f,
  1000.0f,
  5000.0f,
  10000.0f,
};
static constexpr float OPEN_LOOP_ACCEL2_HZ_PER_S2_TABLE[16] = {
  0.0f,
  0.05f,
  1.0f,
  2.5f,
  5.0f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  250.0f,
  500.0f,
  750.0f,
  1000.0f,
  5000.0f,
  10000.0f,
};
static constexpr float OPEN_TO_CLOSED_HANDOFF_PERCENT_TABLE[32] = {
  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,  9.0f,  10.0f, 11.0f,
  12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 22.5f, 25.0f,
  27.5f, 30.0f, 32.5f, 35.0f, 37.5f, 40.0f, 42.5f, 45.0f, 47.5f, 50.0f,
};
static constexpr float THETA_ERROR_RAMP_RATE_TABLE[8] = {
  0.01f,
  0.05f,
  0.1f,
  0.15f,
  0.2f,
  0.5f,
  1.0f,
  2.0f,
};
static constexpr float CL_SLOW_ACC_HZ_PER_S_TABLE[16] = {
  0.1f,
  1.0f,
  2.0f,
  3.0f,
  5.0f,
  10.0f,
  20.0f,
  30.0f,
  40.0f,
  50.0f,
  100.0f,
  200.0f,
  500.0f,
  750.0f,
  1000.0f,
  2000.0f,
};
static constexpr float LOCK_ILIMIT_DEGLITCH_MS_TABLE[16] = {
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
  1000.0f,
};
static constexpr uint8_t HW_LOCK_ILIMIT_DEGLITCH_US_TABLE[8] = {
  0, 1, 2, 3, 4, 5, 6, 7,
};

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

void MCF8329ADirectionSelect::control(const std::string& value) {
  if (this->parent_ == nullptr || !this->parent_->set_direction_mode(value)) {
    ESP_LOGW(TAG, "Failed to set direction mode to %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void MCF8329ASpeedNumber::control(float value) {
  if (this->parent_ == nullptr || !this->parent_->set_speed_percent(value, "number_control")) {
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
  this->algorithm_state_valid_ = false;
  this->algorithm_state_read_error_latched_ = false;
  this->last_algorithm_state_ = 0xFFFFu;
  this->startup_profile_last_check_ms_ = 0u;
  this->startup_profile_last_recovery_ms_ = 0u;
  this->speed_target_percent_ = 0.0f;
  this->speed_applied_percent_ = 0.0f;
  this->speed_target_active_ = false;
  this->start_boost_active_ = false;
  this->start_boost_until_ms_ = 0u;
  this->last_ramp_update_ms_ = 0u;

  ESP_LOGW(
    TAG,
    "MCx83xx I2C note: datasheet requires >=100us inter-byte gap. ESPHome I2C cannot enforce "
    "byte-level gaps; keep bus at <=50kHz and verify communication stability."
  );

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
    this->log_algorithm_state_transition_(algo_status, "update");
  }

  this->recover_from_mcf_reset_if_needed_();

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

  if (fault_state_valid) {
    if (this->fault_active_binary_sensor_ != nullptr) {
      this->fault_active_binary_sensor_->publish_state(fault_active);
    }
    this->handle_fault_shutdown_(fault_active, controller_fault_status, controller_ok);
  } else {
    this->fault_latched_ = false;
  }

  if (!fault_active) {
    this->process_speed_command_ramp_();
  }

  if (this->motor_bemf_constant_sensor_ != nullptr) {
    uint32_t mtr_params = 0;
    if (this->read_reg32(REG_MTR_PARAMS, mtr_params)) {
      const uint32_t motor_bemf_const =
        (mtr_params & MTR_PARAMS_MOTOR_BEMF_CONST_MASK) >> MTR_PARAMS_MOTOR_BEMF_CONST_SHIFT;
      this->motor_bemf_constant_sensor_->publish_state(static_cast<float>(motor_bemf_const));
    }
  }

  if (this->speed_fdbk_hz_sensor_ != nullptr || this->speed_ref_open_loop_hz_sensor_ != nullptr ||
      this->fg_speed_fdbk_hz_sensor_ != nullptr) {
    uint32_t closed_loop4 = 0;
    float max_speed_hz = this->cfg_max_speed_set_ ? this->max_speed_code_to_hz_(this->cfg_max_speed_code_) : 0.0f;
    if (this->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
      const uint16_t max_speed_code = static_cast<uint16_t>(
        (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT
      );
      max_speed_hz = this->max_speed_code_to_hz_(max_speed_code);
    }

    if (max_speed_hz > 0.0f) {
      uint32_t raw_speed_fdbk = 0;
      if (this->speed_fdbk_hz_sensor_ != nullptr && this->read_reg32(REG_SPEED_FDBK, raw_speed_fdbk)) {
        this->speed_fdbk_hz_sensor_->publish_state(
          this->speed_raw_to_hz_(static_cast<int32_t>(raw_speed_fdbk), max_speed_hz)
        );
      }

      uint32_t raw_speed_ref_open_loop = 0;
      if (this->speed_ref_open_loop_hz_sensor_ != nullptr &&
          this->read_reg32(REG_SPEED_REF_OPEN_LOOP, raw_speed_ref_open_loop)) {
        this->speed_ref_open_loop_hz_sensor_->publish_state(
          this->speed_raw_to_hz_(static_cast<int32_t>(raw_speed_ref_open_loop), max_speed_hz)
        );
      }

      uint32_t raw_fg_speed_fdbk = 0;
      if (this->fg_speed_fdbk_hz_sensor_ != nullptr &&
          this->read_reg32(REG_FG_SPEED_FDBK, raw_fg_speed_fdbk)) {
        this->fg_speed_fdbk_hz_sensor_->publish_state(
          this->fg_speed_raw_to_hz_(raw_fg_speed_fdbk, max_speed_hz)
        );
      }
    }
  }

  if (this->read_reg32(REG_VM_VOLTAGE, vm_voltage_raw)) {
    const float vm_voltage =
      static_cast<float>(static_cast<double>(vm_voltage_raw) * VM_VOLTAGE_SCALE);
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
  ESP_LOGCONFIG(
    TAG, "  Inter-byte delay: %u us", static_cast<unsigned>(this->inter_byte_delay_us_)
  );
  if (this->inter_byte_delay_us_ > 0) {
    ESP_LOGCONFIG(
      TAG, "  Note: inter-byte delay is currently not applied with ESPHome I2C transactions"
    );
  }
  ESP_LOGW(
    TAG,
    "  MCx83xx I2C requirement: >=100us byte gap. Use i2c.frequency <=50kHz and verify comms."
  );
  ESP_LOGCONFIG(TAG, "  Auto tickle watchdog: %s", YESNO(this->auto_tickle_watchdog_));
  ESP_LOGCONFIG(TAG, "  Clear MPET bits on startup: %s", YESNO(this->clear_mpet_on_startup_));
  if (this->cfg_motor_bemf_const_set_) {
    ESP_LOGCONFIG(TAG, "  Motor BEMF const: 0x%02X", this->cfg_motor_bemf_const_);
  } else {
    ESP_LOGCONFIG(TAG, "  Motor BEMF const: (unchanged)");
  }
  ESP_LOGCONFIG(
    TAG,
    "  Motor config brake mode: %s",
    this->cfg_brake_mode_set_ ? this->brake_mode_to_string_(this->cfg_brake_mode_)
                                  : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config brake time: %s",
    this->cfg_brake_time_set_ ? this->brake_time_to_string_(this->cfg_brake_time_)
                                  : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config mode: %s",
    this->cfg_mode_set_ ? this->mode_to_string_(this->cfg_mode_) : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config align time: %s",
    this->cfg_align_time_set_ ? this->align_time_to_string_(this->cfg_align_time_)
                                  : "(unchanged)"
  );
  if (this->cfg_csa_gain_set_) {
    static constexpr float CSA_GAIN_VV_TABLE[4] = {5.0f, 10.0f, 20.0f, 40.0f};
    ESP_LOGCONFIG(
      TAG,
      "  Motor config CSA gain override: %.0fV/V (code=%u)",
      CSA_GAIN_VV_TABLE[this->cfg_csa_gain_ & 0x3u],
      static_cast<unsigned>(this->cfg_csa_gain_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config CSA gain override: (unchanged)");
  }
  if (this->cfg_base_current_set_) {
    const float cfg_base_current_amps =
      (static_cast<float>(this->cfg_base_current_code_) * 1200.0f) / 32768.0f;
    ESP_LOGCONFIG(
      TAG,
      "  Motor config BASE_CURRENT override: %u (~%.2fA)",
      static_cast<unsigned>(this->cfg_base_current_code_),
      cfg_base_current_amps
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config BASE_CURRENT override: (unchanged)");
  }
  ESP_LOGCONFIG(
    TAG,
    "  Motor config direction: %s",
    this->cfg_direction_mode_set_ ? this->cfg_direction_mode_.c_str() : "(hardware default)"
  );
  if (this->cfg_ilimit_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config ILIMIT (phase peak): %u%% BASE_CURRENT (code=%u)",
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[this->cfg_ilimit_ & 0x0Fu]),
      static_cast<unsigned>(this->cfg_ilimit_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config ILIMIT (phase peak): (unchanged)");
  }
  if (this->cfg_lock_mode_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config lock mode: %s (code=%u)",
      this->lock_mode_to_string_(this->cfg_lock_mode_),
      static_cast<unsigned>(this->cfg_lock_mode_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config lock mode: (unchanged)");
  }
  if (this->cfg_lock_ilimit_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config lock current limit: %u%% BASE_CURRENT (code=%u)",
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[this->cfg_lock_ilimit_ & 0x0Fu]),
      static_cast<unsigned>(this->cfg_lock_ilimit_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config lock current limit: (unchanged)");
  }
  if (this->cfg_hw_lock_ilimit_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config HW lock current limit: %u%% BASE_CURRENT (code=%u)",
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[this->cfg_hw_lock_ilimit_ & 0x0Fu]),
      static_cast<unsigned>(this->cfg_hw_lock_ilimit_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config HW lock current limit: (unchanged)");
  }
  uint32_t gd_config1 = 0;
  uint32_t gd_config2 = 0;
  if (this->read_reg32(REG_GD_CONFIG1, gd_config1) && this->read_reg32(REG_GD_CONFIG2, gd_config2)) {
    const uint8_t csa_gain_code = static_cast<uint8_t>(
      (gd_config1 & GD_CONFIG1_CSA_GAIN_MASK) >> GD_CONFIG1_CSA_GAIN_SHIFT
    );
    const uint32_t base_current_code =
      (gd_config2 & GD_CONFIG2_BASE_CURRENT_MASK) >> GD_CONFIG2_BASE_CURRENT_SHIFT;
    static constexpr float CSA_GAIN_VV_TABLE[4] = {5.0f, 10.0f, 20.0f, 40.0f};
    const float csa_gain_vv = CSA_GAIN_VV_TABLE[csa_gain_code & 0x3u];
    const float base_current_a = (static_cast<float>(base_current_code) * 1200.0f) / 32768.0f;
    ESP_LOGCONFIG(
      TAG,
      "  Current scaling: GD_CONFIG1.CSA_GAIN=%u(%.0fV/V) GD_CONFIG2.BASE_CURRENT=%u (~%.2fA)",
      static_cast<unsigned>(csa_gain_code),
      csa_gain_vv,
      static_cast<unsigned>(base_current_code),
      base_current_a
    );
    if (this->cfg_ilimit_set_) {
      const float limit_a =
        base_current_a * (static_cast<float>(LOCK_ILIMIT_PERCENT_TABLE[this->cfg_ilimit_ & 0x0Fu]) / 100.0f);
      ESP_LOGCONFIG(TAG, "  Motor config ILIMIT approx: %.2fA", limit_a);
    }
    if (this->cfg_open_loop_ilimit_set_) {
      const float limit_a = base_current_a *
                            (static_cast<float>(
                               LOCK_ILIMIT_PERCENT_TABLE[this->cfg_open_loop_ilimit_ & 0x0Fu]
                             ) /
                             100.0f);
      ESP_LOGCONFIG(TAG, "  Motor config open-loop ILIMIT approx: %.2fA", limit_a);
    }
    if (this->cfg_align_or_slow_current_ilimit_set_) {
      const float limit_a =
        base_current_a *
        (static_cast<float>(
           LOCK_ILIMIT_PERCENT_TABLE[this->cfg_align_or_slow_current_ilimit_ & 0x0Fu]
         ) /
         100.0f);
      ESP_LOGCONFIG(TAG, "  Motor config align/slow current limit approx: %.2fA", limit_a);
    }
    if (this->cfg_lock_ilimit_set_) {
      const float limit_a = base_current_a *
                            (static_cast<float>(
                               LOCK_ILIMIT_PERCENT_TABLE[this->cfg_lock_ilimit_ & 0x0Fu]
                             ) /
                             100.0f);
      ESP_LOGCONFIG(TAG, "  Motor config lock ILIMIT approx: %.2fA", limit_a);
    }
    if (this->cfg_hw_lock_ilimit_set_) {
      const float limit_a = base_current_a *
                            (static_cast<float>(
                               LOCK_ILIMIT_PERCENT_TABLE[this->cfg_hw_lock_ilimit_ & 0x0Fu]
                             ) /
                             100.0f);
      ESP_LOGCONFIG(TAG, "  Motor config HW lock ILIMIT approx: %.2fA", limit_a);
    }
  } else {
    ESP_LOGCONFIG(
      TAG,
      "  Current scaling: unable to read GD_CONFIG1/GD_CONFIG2 (CSA_GAIN/BASE_CURRENT)"
    );
  }
  ESP_LOGCONFIG(
    TAG,
    "  Motor config lock retry: %s",
    this->cfg_lock_retry_time_set_
      ? this->lock_retry_time_to_string_(this->cfg_lock_retry_time_)
      : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config ABN speed lock enable: %s",
    this->cfg_abn_speed_lock_enable_set_ ? YESNO(this->cfg_abn_speed_lock_enable_)
                                             : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config ABN BEMF lock enable: %s",
    this->cfg_abn_bemf_lock_enable_set_ ? YESNO(this->cfg_abn_bemf_lock_enable_)
                                            : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config no-motor lock enable: %s",
    this->cfg_no_motor_lock_enable_set_ ? YESNO(this->cfg_no_motor_lock_enable_)
                                            : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config ABN speed threshold: %s",
    this->cfg_lock_abn_speed_threshold_set_
      ? LOCK_ABN_SPEED_THRESHOLD_LABELS[this->cfg_lock_abn_speed_threshold_ & 0x7u]
      : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config ABN BEMF threshold: %s",
    this->cfg_abnormal_bemf_threshold_set_
      ? ABNORMAL_BEMF_THRESHOLD_LABELS[this->cfg_abnormal_bemf_threshold_ & 0x7u]
      : "(unchanged)"
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config no-motor threshold: %s",
    this->cfg_no_motor_threshold_set_
      ? NO_MOTOR_THRESHOLD_LABELS[this->cfg_no_motor_threshold_ & 0x7u]
      : "(unchanged)"
  );
  if (this->cfg_max_speed_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config max speed: %.1f Hz electrical (code=%u)",
      this->max_speed_code_to_hz_(this->cfg_max_speed_code_),
      static_cast<unsigned>(this->cfg_max_speed_code_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config max speed: (unchanged)");
  }
  if (this->cfg_open_loop_ilimit_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config open-loop current limit: %u%% BASE_CURRENT (code=%u)",
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[this->cfg_open_loop_ilimit_ & 0x0Fu]),
      static_cast<unsigned>(this->cfg_open_loop_ilimit_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config open-loop current limit: (unchanged)");
  }
  if (this->cfg_align_or_slow_current_ilimit_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config align/slow current limit: %u%% BASE_CURRENT (code=%u)",
      static_cast<unsigned>(
        LOCK_ILIMIT_PERCENT_TABLE[this->cfg_align_or_slow_current_ilimit_ & 0x0Fu]
      ),
      static_cast<unsigned>(this->cfg_align_or_slow_current_ilimit_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config align/slow current limit: (unchanged)");
  }
  if (this->cfg_open_loop_limit_source_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config open-loop limit source: %s",
      this->cfg_open_loop_limit_use_ilimit_ ? "ILIMIT (FAULT_CONFIG1.ILIMIT)"
                                                : "OL_ILIMIT (MOTOR_STARTUP2.OL_ILIMIT)"
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config open-loop limit source: (unchanged)");
  }
  if (this->cfg_open_loop_accel_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config open-loop accel A1: %.2f Hz/s (code=%u)",
      this->open_loop_accel_code_to_hz_per_s_(this->cfg_open_loop_accel_),
      static_cast<unsigned>(this->cfg_open_loop_accel_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config open-loop accel A1: (unchanged)");
  }
  if (this->cfg_open_loop_accel2_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config open-loop accel A2: %.2f Hz/s2 (code=%u)",
      OPEN_LOOP_ACCEL2_HZ_PER_S2_TABLE[this->cfg_open_loop_accel2_ & 0x0Fu],
      static_cast<unsigned>(this->cfg_open_loop_accel2_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config open-loop accel A2: (unchanged)");
  }
  ESP_LOGCONFIG(
    TAG,
    "  Motor config auto handoff: %s",
    this->cfg_auto_handoff_enable_set_ ? YESNO(this->cfg_auto_handoff_enable_)
                                           : "(unchanged)"
  );
  if (this->cfg_open_to_closed_handoff_threshold_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config open->closed handoff threshold: %.1f%% MAX_SPEED (code=%u)",
      this->open_to_closed_handoff_code_to_percent_(this->cfg_open_to_closed_handoff_threshold_
      ),
      static_cast<unsigned>(this->cfg_open_to_closed_handoff_threshold_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config open->closed handoff threshold: (unchanged)");
  }
  if (this->cfg_theta_error_ramp_rate_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config theta error ramp rate: %.2f (code=%u)",
      THETA_ERROR_RAMP_RATE_TABLE[this->cfg_theta_error_ramp_rate_ & 0x07u],
      static_cast<unsigned>(this->cfg_theta_error_ramp_rate_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config theta error ramp rate: (unchanged)");
  }
  if (this->cfg_cl_slow_acc_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config CL slow accel: %.1f Hz/s (code=%u)",
      CL_SLOW_ACC_HZ_PER_S_TABLE[this->cfg_cl_slow_acc_ & 0x0Fu],
      static_cast<unsigned>(this->cfg_cl_slow_acc_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config CL slow accel: (unchanged)");
  }
  if (this->cfg_lock_ilimit_deglitch_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config LOCK_ILIMIT deglitch: %.1fms (code=%u)",
      LOCK_ILIMIT_DEGLITCH_MS_TABLE[this->cfg_lock_ilimit_deglitch_ & 0x0Fu],
      static_cast<unsigned>(this->cfg_lock_ilimit_deglitch_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config LOCK_ILIMIT deglitch: (unchanged)");
  }
  if (this->cfg_hw_lock_ilimit_deglitch_set_) {
    ESP_LOGCONFIG(
      TAG,
      "  Motor config HW_LOCK_ILIMIT deglitch: %uus (code=%u)",
      static_cast<unsigned>(HW_LOCK_ILIMIT_DEGLITCH_US_TABLE[this->cfg_hw_lock_ilimit_deglitch_ & 0x07u]),
      static_cast<unsigned>(this->cfg_hw_lock_ilimit_deglitch_)
    );
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config HW_LOCK_ILIMIT deglitch: (unchanged)");
  }
  if (this->cfg_speed_loop_kp_code_set_) {
    if (this->cfg_speed_loop_kp_code_ == 0u) {
      ESP_LOGCONFIG(TAG, "  Motor config speed-loop Kp code: 0 (keep auto)");
    } else {
      ESP_LOGCONFIG(TAG, "  Motor config speed-loop Kp code: %u", static_cast<unsigned>(this->cfg_speed_loop_kp_code_));
    }
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config speed-loop Kp code: (unchanged)");
  }
  if (this->cfg_speed_loop_ki_code_set_) {
    if (this->cfg_speed_loop_ki_code_ == 0u) {
      ESP_LOGCONFIG(TAG, "  Motor config speed-loop Ki code: 0 (keep auto)");
    } else {
      ESP_LOGCONFIG(TAG, "  Motor config speed-loop Ki code: %u", static_cast<unsigned>(this->cfg_speed_loop_ki_code_));
    }
  } else {
    ESP_LOGCONFIG(TAG, "  Motor config speed-loop Ki code: (unchanged)");
  }
  ESP_LOGCONFIG(
    TAG,
    "  Speed command shaping: ramp_up=%.2f%%/s ramp_down=%.2f%%/s boost=%.1f%% hold=%ums",
    this->speed_ramp_up_percent_per_s_,
    this->speed_ramp_down_percent_per_s_,
    this->start_boost_percent_,
    static_cast<unsigned>(this->start_boost_hold_ms_)
  );
  ESP_LOGCONFIG(
    TAG,
    "  Motor config comm gate: scan 0x%02X..0x%02X, attempts=%u, retry=%ums, deferred_retry=%ums",
    static_cast<unsigned>(I2C_SCAN_ADDRESS_MIN),
    static_cast<unsigned>(I2C_SCAN_ADDRESS_MAX),
    static_cast<unsigned>(STARTUP_COMMS_ATTEMPTS),
    static_cast<unsigned>(STARTUP_COMMS_RETRY_DELAY_MS),
    static_cast<unsigned>(DEFERRED_COMMS_RETRY_INTERVAL_MS)
  );
}

const char* MCF8329AComponent::i2c_error_to_string_(i2c::ErrorCode error_code) const {
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

bool MCF8329AComponent::probe_device_ack_(i2c::ErrorCode& error_code) const {
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

bool MCF8329AComponent::establish_communications_(
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

void MCF8329AComponent::apply_post_comms_setup_() {
  if (this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(0.0f);
  }
  if (!this->set_speed_percent(0.0f, "motor_init")) {
    ESP_LOGW(TAG, "Failed to force speed to 0%% during setup");
  }

  if (this->brake_switch_ != nullptr) {
    this->brake_switch_->publish_state(true);
  }
  if (!this->set_brake_override(true)) {
    ESP_LOGW(TAG, "Failed to force brake ON during setup");
  }

  const std::string direction_mode =
    this->cfg_direction_mode_set_ ? this->cfg_direction_mode_ : "hardware";
  if (this->direction_select_ != nullptr) {
    this->direction_select_->publish_state(direction_mode);
  }
  if (!this->set_direction_mode(direction_mode)) {
    ESP_LOGW(TAG, "Failed to set motor direction mode to %s", direction_mode.c_str());
  }

  if (this->clear_mpet_on_startup_) {
    (void)this->clear_mpet_bits_("motor_init");
  }

  if (!this->apply_motor_config_()) {
    ESP_LOGW(TAG, "Failed to apply one or more configured motor settings");
  }

  if (this->clear_mpet_on_startup_) {
    uint32_t controller_fault_status = 0;
    if (this->read_reg32(REG_CONTROLLER_FAULT_STATUS, controller_fault_status) && (controller_fault_status & FAULT_MPET_BEMF) != 0u) {
      ESP_LOGW(TAG, "MPET_BEMF fault present after motor init; pulsing CLR_FLT once");
      this->pulse_clear_faults();
    }
  }
}

void MCF8329AComponent::recover_from_mcf_reset_if_needed_() {
  if (!this->cfg_motor_bemf_const_set_) {
    return;
  }

  const uint32_t now = millis();
  if (this->startup_profile_last_check_ms_ != 0u &&
      (now - this->startup_profile_last_check_ms_) < STARTUP_PROFILE_CHECK_INTERVAL_MS) {
    return;
  }
  this->startup_profile_last_check_ms_ = now;

  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP3, closed_loop3) ||
      !this->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
    return;
  }

  const uint8_t bemf_const = static_cast<uint8_t>(
    (closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT
  );
  const uint16_t max_speed_code = static_cast<uint16_t>(
    (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT
  );

  // Signature of a fresh-reset/default profile observed in field logs:
  // BEMF const at 0x00 with MAX_SPEED at default code 1200 (200 Hz).
  if (bemf_const != 0u || max_speed_code != 1200u) {
    return;
  }

  if (this->startup_profile_last_recovery_ms_ != 0u &&
      (now - this->startup_profile_last_recovery_ms_) < STARTUP_PROFILE_RECOVERY_COOLDOWN_MS) {
    return;
  }
  this->startup_profile_last_recovery_ms_ = now;

  ESP_LOGW(
    TAG,
    "Detected probable MCF reset/default profile (bemf=0x%02X max_speed_code=%u). "
    "Reapplying motor config without ESP reboot.",
    static_cast<unsigned>(bemf_const),
    static_cast<unsigned>(max_speed_code)
  );

  // Clear local latches so recovered device state is not blocked by stale software lockouts.
  this->fault_latched_ = false;
  this->mpet_bemf_fault_latched_ = false;
  this->hw_lock_fault_latched_ = false;
  this->severe_fault_speed_lockout_ = false;
  this->apply_post_comms_setup_();
}

bool MCF8329AComponent::apply_motor_config_() {
  const std::string effective_direction =
    this->cfg_direction_mode_set_ ? this->cfg_direction_mode_ : "hardware";

  bool ok = true;

  uint32_t gd_config1 = 0;
  if (!this->read_reg32(REG_GD_CONFIG1, gd_config1)) {
    ESP_LOGW(TAG, "Failed to read GD_CONFIG1 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t gd_config1_next = gd_config1;
  if (this->cfg_csa_gain_set_) {
    gd_config1_next =
      (gd_config1_next & ~GD_CONFIG1_CSA_GAIN_MASK) |
      ((static_cast<uint32_t>(this->cfg_csa_gain_) << GD_CONFIG1_CSA_GAIN_SHIFT) &
       GD_CONFIG1_CSA_GAIN_MASK);
  }
  if (gd_config1_next != gd_config1) {
    ok &= this->write_reg32(REG_GD_CONFIG1, gd_config1_next);
    ESP_LOGI(TAG, "GD_CONFIG1 motor cfg: 0x%08X -> 0x%08X", gd_config1, gd_config1_next);
  }

  uint32_t gd_config2 = 0;
  if (!this->read_reg32(REG_GD_CONFIG2, gd_config2)) {
    ESP_LOGW(TAG, "Failed to read GD_CONFIG2 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t gd_config2_next = gd_config2;
  if (this->cfg_base_current_set_) {
    gd_config2_next =
      (gd_config2_next & ~GD_CONFIG2_BASE_CURRENT_MASK) |
      ((static_cast<uint32_t>(this->cfg_base_current_code_) << GD_CONFIG2_BASE_CURRENT_SHIFT) &
       GD_CONFIG2_BASE_CURRENT_MASK);
  }
  if (gd_config2_next != gd_config2) {
    ok &= this->write_reg32(REG_GD_CONFIG2, gd_config2_next);
    ESP_LOGI(TAG, "GD_CONFIG2 motor cfg: 0x%08X -> 0x%08X", gd_config2, gd_config2_next);
  }

  uint32_t fault_config1 = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG1, fault_config1)) {
    ESP_LOGW(TAG, "Failed to read FAULT_CONFIG1 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t fault_config1_next = fault_config1;
  if (this->cfg_ilimit_set_) {
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_ILIMIT_MASK) |
      ((static_cast<uint32_t>(this->cfg_ilimit_) << FAULT_CONFIG1_ILIMIT_SHIFT) &
       FAULT_CONFIG1_ILIMIT_MASK);
  }
  if (this->cfg_hw_lock_ilimit_set_) {
    fault_config1_next = (fault_config1_next & ~FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) |
                         ((static_cast<uint32_t>(this->cfg_hw_lock_ilimit_)
                           << FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT) &
                          FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK);
  }
  if (this->cfg_lock_ilimit_set_) {
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_LOCK_ILIMIT_MASK) |
      ((static_cast<uint32_t>(this->cfg_lock_ilimit_) << FAULT_CONFIG1_LOCK_ILIMIT_SHIFT) &
       FAULT_CONFIG1_LOCK_ILIMIT_MASK);
  }
  if (this->cfg_lock_mode_set_) {
    const uint32_t mode = static_cast<uint32_t>(this->cfg_lock_mode_);
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) |
      ((mode << FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT) & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK);
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_MTR_LCK_MODE_MASK) |
      ((mode << FAULT_CONFIG1_MTR_LCK_MODE_SHIFT) & FAULT_CONFIG1_MTR_LCK_MODE_MASK);
  }
  if (this->cfg_lock_retry_time_set_) {
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_LCK_RETRY_MASK) |
      ((static_cast<uint32_t>(this->cfg_lock_retry_time_) << FAULT_CONFIG1_LCK_RETRY_SHIFT) &
       FAULT_CONFIG1_LCK_RETRY_MASK);
  }
  if (this->cfg_lock_ilimit_deglitch_set_) {
    fault_config1_next =
      (fault_config1_next & ~FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK) |
      ((static_cast<uint32_t>(this->cfg_lock_ilimit_deglitch_) << FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT) &
       FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK);
  }
  if (fault_config1_next != fault_config1) {
    ok &= this->write_reg32(REG_FAULT_CONFIG1, fault_config1_next);
    ESP_LOGI(TAG, "FAULT_CONFIG1 motor cfg: 0x%08X -> 0x%08X", fault_config1, fault_config1_next);
  }

  uint32_t fault_config2 = 0;
  if (!this->read_reg32(REG_FAULT_CONFIG2, fault_config2)) {
    ESP_LOGW(TAG, "Failed to read FAULT_CONFIG2 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t fault_config2_next = fault_config2;
  if (this->cfg_abn_speed_lock_enable_set_) {
    if (this->cfg_abn_speed_lock_enable_) {
      fault_config2_next |= FAULT_CONFIG2_LOCK1_EN_MASK;
    } else {
      fault_config2_next &= ~FAULT_CONFIG2_LOCK1_EN_MASK;
    }
  }
  if (this->cfg_abn_bemf_lock_enable_set_) {
    if (this->cfg_abn_bemf_lock_enable_) {
      fault_config2_next |= FAULT_CONFIG2_LOCK2_EN_MASK;
    } else {
      fault_config2_next &= ~FAULT_CONFIG2_LOCK2_EN_MASK;
    }
  }
  if (this->cfg_no_motor_lock_enable_set_) {
    if (this->cfg_no_motor_lock_enable_) {
      fault_config2_next |= FAULT_CONFIG2_LOCK3_EN_MASK;
    } else {
      fault_config2_next &= ~FAULT_CONFIG2_LOCK3_EN_MASK;
    }
  }
  if (this->cfg_lock_abn_speed_threshold_set_) {
    fault_config2_next = (fault_config2_next & ~FAULT_CONFIG2_LOCK_ABN_SPEED_MASK) |
                         ((static_cast<uint32_t>(this->cfg_lock_abn_speed_threshold_)
                           << FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT) &
                          FAULT_CONFIG2_LOCK_ABN_SPEED_MASK);
  }
  if (this->cfg_abnormal_bemf_threshold_set_) {
    fault_config2_next = (fault_config2_next & ~FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK) |
                         ((static_cast<uint32_t>(this->cfg_abnormal_bemf_threshold_)
                           << FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT) &
                          FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK);
  }
  if (this->cfg_no_motor_threshold_set_) {
    fault_config2_next = (fault_config2_next & ~FAULT_CONFIG2_NO_MTR_THR_MASK) |
                         ((static_cast<uint32_t>(this->cfg_no_motor_threshold_)
                           << FAULT_CONFIG2_NO_MTR_THR_SHIFT) &
                          FAULT_CONFIG2_NO_MTR_THR_MASK);
  }
  if (this->cfg_lock_mode_set_) {
    fault_config2_next = (fault_config2_next & ~FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) |
                         ((static_cast<uint32_t>(this->cfg_lock_mode_)
                           << FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT) &
                          FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK);
  }
  if (this->cfg_hw_lock_ilimit_deglitch_set_) {
    fault_config2_next =
      (fault_config2_next & ~FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK) |
      ((static_cast<uint32_t>(this->cfg_hw_lock_ilimit_deglitch_)
        << FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT) &
       FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK);
  }
  if (fault_config2_next != fault_config2) {
    ok &= this->write_reg32(REG_FAULT_CONFIG2, fault_config2_next);
    ESP_LOGI(TAG, "FAULT_CONFIG2 motor cfg: 0x%08X -> 0x%08X", fault_config2, fault_config2_next);
  }

  uint32_t closed_loop2 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP2, closed_loop2)) {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP2 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t closed_loop2_next = closed_loop2;
  if (this->cfg_brake_mode_set_) {
    closed_loop2_next =
      (closed_loop2_next & ~CLOSED_LOOP2_MTR_STOP_MASK) |
      ((static_cast<uint32_t>(this->cfg_brake_mode_) << CLOSED_LOOP2_MTR_STOP_SHIFT) &
       CLOSED_LOOP2_MTR_STOP_MASK);
  }
  if (this->cfg_brake_time_set_) {
    closed_loop2_next =
      (closed_loop2_next & ~CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK) |
      ((static_cast<uint32_t>(this->cfg_brake_time_) << CLOSED_LOOP2_MTR_STOP_BRK_TIME_SHIFT) &
       CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK);
  }
  if (this->cfg_motor_bemf_const_set_) {
    const uint32_t motor_res =
      (closed_loop2_next & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT;
    const uint32_t motor_ind =
      (closed_loop2_next & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT;
    if (motor_res == 0u) {
      closed_loop2_next = (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_RES_MASK) |
                          (CLOSED_LOOP_SEED_MOTOR_RES << CLOSED_LOOP2_MOTOR_RES_SHIFT);
      ESP_LOGW(
        TAG,
        "Seeding MOTOR_RES from 0 to %u while applying cfg_motor_bemf_const",
        static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_RES)
      );
    }
    if (motor_ind == 0u) {
      closed_loop2_next = (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_IND_MASK) |
                          (CLOSED_LOOP_SEED_MOTOR_IND << CLOSED_LOOP2_MOTOR_IND_SHIFT);
      ESP_LOGW(
        TAG,
        "Seeding MOTOR_IND from 0 to %u while applying cfg_motor_bemf_const",
        static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_IND)
      );
    }
  }
  if (closed_loop2_next != closed_loop2) {
    ok &= this->write_reg32(REG_CLOSED_LOOP2, closed_loop2_next);
    ESP_LOGI(TAG, "CLOSED_LOOP2 motor cfg: 0x%08X -> 0x%08X", closed_loop2, closed_loop2_next);
  }

  uint32_t int_algo2 = 0;
  if (!this->read_reg32(REG_INT_ALGO_2, int_algo2)) {
    ESP_LOGW(TAG, "Failed to read INT_ALGO_2 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t int_algo2_next = int_algo2;
  if (this->cfg_cl_slow_acc_set_) {
    int_algo2_next =
      (int_algo2_next & ~INT_ALGO_2_CL_SLOW_ACC_MASK) |
      ((static_cast<uint32_t>(this->cfg_cl_slow_acc_) << INT_ALGO_2_CL_SLOW_ACC_SHIFT) &
       INT_ALGO_2_CL_SLOW_ACC_MASK);
  }
  if (int_algo2_next != int_algo2) {
    ok &= this->write_reg32(REG_INT_ALGO_2, int_algo2_next);
    ESP_LOGI(TAG, "INT_ALGO_2 motor cfg: 0x%08X -> 0x%08X", int_algo2, int_algo2_next);
  }

  uint32_t motor_startup1 = 0;
  if (!this->read_reg32(REG_MOTOR_STARTUP1, motor_startup1)) {
    ESP_LOGW(TAG, "Failed to read MOTOR_STARTUP1 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t motor_startup1_next = motor_startup1;
  if (this->cfg_mode_set_) {
    motor_startup1_next =
      (motor_startup1_next & ~MOTOR_STARTUP1_MTR_STARTUP_MASK) |
      ((static_cast<uint32_t>(this->cfg_mode_) << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) &
       MOTOR_STARTUP1_MTR_STARTUP_MASK);
  }
  if (this->cfg_align_time_set_) {
    motor_startup1_next =
      (motor_startup1_next & ~MOTOR_STARTUP1_ALIGN_TIME_MASK) |
      ((static_cast<uint32_t>(this->cfg_align_time_) << MOTOR_STARTUP1_ALIGN_TIME_SHIFT) &
       MOTOR_STARTUP1_ALIGN_TIME_MASK);
  }
  if (this->cfg_align_or_slow_current_ilimit_set_) {
    motor_startup1_next =
      (motor_startup1_next & ~MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK) |
      ((static_cast<uint32_t>(this->cfg_align_or_slow_current_ilimit_)
        << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT) &
       MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK);
  }
  if (this->cfg_open_loop_limit_source_set_) {
    motor_startup1_next =
      (motor_startup1_next & ~MOTOR_STARTUP1_OL_ILIMIT_CONFIG_MASK) |
      ((static_cast<uint32_t>(this->cfg_open_loop_limit_use_ilimit_ ? 1u : 0u)
        << MOTOR_STARTUP1_OL_ILIMIT_CONFIG_SHIFT) &
       MOTOR_STARTUP1_OL_ILIMIT_CONFIG_MASK);
  }
  if (motor_startup1_next != motor_startup1) {
    ok &= this->write_reg32(REG_MOTOR_STARTUP1, motor_startup1_next);
    ESP_LOGI(
      TAG, "MOTOR_STARTUP1 motor cfg: 0x%08X -> 0x%08X", motor_startup1, motor_startup1_next
    );
  }

  uint32_t motor_startup2 = 0;
  if (!this->read_reg32(REG_MOTOR_STARTUP2, motor_startup2)) {
    ESP_LOGW(TAG, "Failed to read MOTOR_STARTUP2 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t motor_startup2_next = motor_startup2;
  if (this->cfg_open_loop_ilimit_set_) {
    motor_startup2_next =
      (motor_startup2_next & ~MOTOR_STARTUP2_OL_ILIMIT_MASK) |
      ((static_cast<uint32_t>(this->cfg_open_loop_ilimit_) << MOTOR_STARTUP2_OL_ILIMIT_SHIFT) &
       MOTOR_STARTUP2_OL_ILIMIT_MASK);
  }
  if (this->cfg_open_loop_accel_set_) {
    motor_startup2_next =
      (motor_startup2_next & ~MOTOR_STARTUP2_OL_ACC_A1_MASK) |
      ((static_cast<uint32_t>(this->cfg_open_loop_accel_) << MOTOR_STARTUP2_OL_ACC_A1_SHIFT) &
       MOTOR_STARTUP2_OL_ACC_A1_MASK);
  }
  if (this->cfg_open_loop_accel2_set_) {
    motor_startup2_next =
      (motor_startup2_next & ~MOTOR_STARTUP2_OL_ACC_A2_MASK) |
      ((static_cast<uint32_t>(this->cfg_open_loop_accel2_) << MOTOR_STARTUP2_OL_ACC_A2_SHIFT) &
       MOTOR_STARTUP2_OL_ACC_A2_MASK);
  }
  if (this->cfg_auto_handoff_enable_set_) {
    if (this->cfg_auto_handoff_enable_) {
      motor_startup2_next |= MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK;
    } else {
      motor_startup2_next &= ~MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK;
    }
  }
  if (this->cfg_open_to_closed_handoff_threshold_set_) {
    motor_startup2_next = (motor_startup2_next & ~MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK) |
                          ((static_cast<uint32_t>(this->cfg_open_to_closed_handoff_threshold_)
                            << MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT) &
                           MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK);
  }
  if (this->cfg_theta_error_ramp_rate_set_) {
    motor_startup2_next =
      (motor_startup2_next & ~MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK) |
      ((static_cast<uint32_t>(this->cfg_theta_error_ramp_rate_)
        << MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_SHIFT) &
       MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK);
  }
  if (motor_startup2_next != motor_startup2) {
    ok &= this->write_reg32(REG_MOTOR_STARTUP2, motor_startup2_next);
    ESP_LOGI(
      TAG, "MOTOR_STARTUP2 motor cfg: 0x%08X -> 0x%08X", motor_startup2, motor_startup2_next
    );
  }

  uint32_t closed_loop3 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP3, closed_loop3)) {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP3 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t closed_loop3_next = closed_loop3;
  uint32_t closed_loop4 = 0;
  if (!this->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP4 for motor config");
    this->motor_config_summary_ = "read_error";
    return false;
  }
  uint32_t closed_loop4_next = closed_loop4;
  if (this->cfg_max_speed_set_) {
    closed_loop4_next =
      (closed_loop4_next & ~CLOSED_LOOP4_MAX_SPEED_MASK) |
      ((static_cast<uint32_t>(this->cfg_max_speed_code_) << CLOSED_LOOP4_MAX_SPEED_SHIFT) &
       CLOSED_LOOP4_MAX_SPEED_MASK);
  }
  if (this->cfg_motor_bemf_const_set_) {
    closed_loop3_next = (closed_loop3_next & ~CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) |
                        ((static_cast<uint32_t>(this->cfg_motor_bemf_const_)
                          << CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT) &
                         CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK);

    const uint32_t spd_loop_kp_msb =
      (closed_loop3_next & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT;
    const uint32_t spd_loop_kp_lsb =
      (closed_loop4_next & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT;
    const uint32_t spd_loop_kp = (spd_loop_kp_msb << 7) | spd_loop_kp_lsb;
    const uint32_t spd_loop_ki =
      (closed_loop4_next & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT;

    if (spd_loop_kp == 0u && (!this->cfg_speed_loop_kp_code_set_ || this->cfg_speed_loop_kp_code_ != 0u)) {
      // A zero speed-loop Kp can force MPET flow on non-zero speed commands.
      closed_loop3_next = (closed_loop3_next & ~CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) |
                          (0u << CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT);
      closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) |
                          (1u << CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT);
      ESP_LOGW(TAG, "Seeding SPD_LOOP_KP from 0 to 1 while applying cfg_motor_bemf_const");
    }

    if (spd_loop_ki == 0u && (!this->cfg_speed_loop_ki_code_set_ || this->cfg_speed_loop_ki_code_ != 0u)) {
      closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KI_MASK) |
                          (1u << CLOSED_LOOP4_SPD_LOOP_KI_SHIFT);
      ESP_LOGW(TAG, "Seeding SPD_LOOP_KI from 0 to 1 while applying cfg_motor_bemf_const");
    }
  }
  if (this->cfg_speed_loop_kp_code_set_ && this->cfg_speed_loop_kp_code_ != 0u) {
    const uint16_t kp_code = this->cfg_speed_loop_kp_code_ & 0x03FFu;
    const uint8_t kp_msb = static_cast<uint8_t>((kp_code >> 7) & 0x07u);
    const uint8_t kp_lsb = static_cast<uint8_t>(kp_code & 0x7Fu);
    closed_loop3_next = (closed_loop3_next & ~CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) |
                        ((static_cast<uint32_t>(kp_msb) << CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT) &
                         CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK);
    closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) |
                        ((static_cast<uint32_t>(kp_lsb) << CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT) &
                         CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK);
  }
  if (this->cfg_speed_loop_ki_code_set_ && this->cfg_speed_loop_ki_code_ != 0u) {
    const uint16_t ki_code = this->cfg_speed_loop_ki_code_ & 0x03FFu;
    closed_loop4_next = (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KI_MASK) |
                        ((static_cast<uint32_t>(ki_code) << CLOSED_LOOP4_SPD_LOOP_KI_SHIFT) &
                         CLOSED_LOOP4_SPD_LOOP_KI_MASK);
  }
  if (closed_loop3_next != closed_loop3) {
    ok &= this->write_reg32(REG_CLOSED_LOOP3, closed_loop3_next);
    ESP_LOGI(TAG, "CLOSED_LOOP3 motor cfg: 0x%08X -> 0x%08X", closed_loop3, closed_loop3_next);
  }
  if (closed_loop4_next != closed_loop4) {
    ok &= this->write_reg32(REG_CLOSED_LOOP4, closed_loop4_next);
    ESP_LOGI(TAG, "CLOSED_LOOP4 motor cfg: 0x%08X -> 0x%08X", closed_loop4, closed_loop4_next);
  }

  uint32_t gd_config1_effective = gd_config1_next;
  uint32_t gd_config2_effective = gd_config2_next;
  if (!this->read_reg32(REG_GD_CONFIG1, gd_config1_effective)) {
    ESP_LOGW(TAG, "Failed to read back GD_CONFIG1 after motor config apply");
  }
  if (!this->read_reg32(REG_GD_CONFIG2, gd_config2_effective)) {
    ESP_LOGW(TAG, "Failed to read back GD_CONFIG2 after motor config apply");
  }

  uint32_t fault_config1_effective = fault_config1_next;
  uint32_t fault_config2_effective = fault_config2_next;
  if (!this->read_reg32(REG_FAULT_CONFIG1, fault_config1_effective)) {
    ESP_LOGW(TAG, "Failed to read back FAULT_CONFIG1 after motor config apply");
  }
  if (!this->read_reg32(REG_FAULT_CONFIG2, fault_config2_effective)) {
    ESP_LOGW(TAG, "Failed to read back FAULT_CONFIG2 after motor config apply");
  }

  uint32_t int_algo2_effective = int_algo2_next;
  if (!this->read_reg32(REG_INT_ALGO_2, int_algo2_effective)) {
    ESP_LOGW(TAG, "Failed to read back INT_ALGO_2 after motor config apply");
  }

  uint32_t closed_loop2_effective = closed_loop2_next;
  uint32_t motor_startup1_effective = motor_startup1_next;
  uint32_t motor_startup2_effective = motor_startup2_next;
  if (!this->read_reg32(REG_CLOSED_LOOP2, closed_loop2_effective)) {
    ESP_LOGW(TAG, "Failed to read back CLOSED_LOOP2 after motor config apply");
  }
  if (!this->read_reg32(REG_MOTOR_STARTUP1, motor_startup1_effective)) {
    ESP_LOGW(TAG, "Failed to read back MOTOR_STARTUP1 after motor config apply");
  }
  if (!this->read_reg32(REG_MOTOR_STARTUP2, motor_startup2_effective)) {
    ESP_LOGW(TAG, "Failed to read back MOTOR_STARTUP2 after motor config apply");
  }
  uint32_t closed_loop3_effective = closed_loop3_next;
  if (!this->read_reg32(REG_CLOSED_LOOP3, closed_loop3_effective)) {
    ESP_LOGW(TAG, "Failed to read back CLOSED_LOOP3 after motor config apply");
  }
  uint32_t closed_loop4_effective = closed_loop4_next;
  if (!this->read_reg32(REG_CLOSED_LOOP4, closed_loop4_effective)) {
    ESP_LOGW(TAG, "Failed to read back CLOSED_LOOP4 after motor config apply");
  }

  const uint8_t effective_brake_mode = static_cast<uint8_t>(
    (closed_loop2_effective & CLOSED_LOOP2_MTR_STOP_MASK) >> CLOSED_LOOP2_MTR_STOP_SHIFT
  );
  const uint8_t effective_brake_time = static_cast<uint8_t>(
    (closed_loop2_effective & CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK) >>
    CLOSED_LOOP2_MTR_STOP_BRK_TIME_SHIFT
  );
  const uint8_t effective_cfg_mode = static_cast<uint8_t>(
    (motor_startup1_effective & MOTOR_STARTUP1_MTR_STARTUP_MASK) >> MOTOR_STARTUP1_MTR_STARTUP_SHIFT
  );
  const uint8_t effective_align_time = static_cast<uint8_t>(
    (motor_startup1_effective & MOTOR_STARTUP1_ALIGN_TIME_MASK) >> MOTOR_STARTUP1_ALIGN_TIME_SHIFT
  );
  const uint8_t effective_align_or_slow_ilimit = static_cast<uint8_t>(
    (motor_startup1_effective & MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK) >>
    MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT
  );
  const bool effective_open_loop_limit_use_ilimit =
    ((motor_startup1_effective & MOTOR_STARTUP1_OL_ILIMIT_CONFIG_MASK) >>
     MOTOR_STARTUP1_OL_ILIMIT_CONFIG_SHIFT) != 0u;
  const uint8_t effective_ol_ilimit = static_cast<uint8_t>(
    (motor_startup2_effective & MOTOR_STARTUP2_OL_ILIMIT_MASK) >> MOTOR_STARTUP2_OL_ILIMIT_SHIFT
  );
  const uint8_t effective_ol_accel = static_cast<uint8_t>(
    (motor_startup2_effective & MOTOR_STARTUP2_OL_ACC_A1_MASK) >> MOTOR_STARTUP2_OL_ACC_A1_SHIFT
  );
  const uint8_t effective_ol_accel2 = static_cast<uint8_t>(
    (motor_startup2_effective & MOTOR_STARTUP2_OL_ACC_A2_MASK) >> MOTOR_STARTUP2_OL_ACC_A2_SHIFT
  );
  const bool effective_auto_handoff =
    (motor_startup2_effective & MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK) != 0u;
  const uint8_t effective_handoff_threshold = static_cast<uint8_t>(
    (motor_startup2_effective & MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK) >>
    MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT
  );
  const uint8_t effective_theta_error_ramp_rate = static_cast<uint8_t>(
    (motor_startup2_effective & MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK) >>
    MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_SHIFT
  );
  const uint8_t effective_motor_bemf_const = static_cast<uint8_t>(
    (closed_loop3_effective & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >>
    CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT
  );
  const uint32_t effective_motor_res =
    (closed_loop2_effective & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT;
  const uint32_t effective_motor_ind =
    (closed_loop2_effective & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT;
  const uint32_t effective_spd_loop_kp =
    (((closed_loop3_effective & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >>
      CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT)
     << 7) |
    ((closed_loop4_effective & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >>
     CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT);
  const uint32_t effective_spd_loop_ki =
    (closed_loop4_effective & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT;
  const uint16_t effective_max_speed_code = static_cast<uint16_t>(
    (closed_loop4_effective & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT
  );
  const float effective_max_speed_hz = this->max_speed_code_to_hz_(effective_max_speed_code);
  const uint8_t effective_ilimit = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_ILIMIT_MASK) >> FAULT_CONFIG1_ILIMIT_SHIFT
  );
  const uint8_t effective_hw_lock_ilimit = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) >>
    FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT
  );
  const uint8_t effective_lock_ilimit = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_SHIFT
  );
  const uint8_t effective_lock_mode = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) >>
    FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT
  );
  const uint8_t effective_mtr_lock_mode = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_MTR_LCK_MODE_MASK) >> FAULT_CONFIG1_MTR_LCK_MODE_SHIFT
  );
  const uint8_t effective_lck_retry = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_LCK_RETRY_MASK) >> FAULT_CONFIG1_LCK_RETRY_SHIFT
  );
  const uint8_t effective_lock_ilimit_deg = static_cast<uint8_t>(
    (fault_config1_effective & FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK) >>
    FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT
  );
  const uint8_t effective_hw_lock_mode = static_cast<uint8_t>(
    (fault_config2_effective & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >>
    FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT
  );
  const uint8_t effective_hw_lock_ilimit_deg = static_cast<uint8_t>(
    (fault_config2_effective & FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK) >>
    FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT
  );
  const bool effective_lock1_en = (fault_config2_effective & FAULT_CONFIG2_LOCK1_EN_MASK) != 0u;
  const bool effective_lock2_en = (fault_config2_effective & FAULT_CONFIG2_LOCK2_EN_MASK) != 0u;
  const bool effective_lock3_en = (fault_config2_effective & FAULT_CONFIG2_LOCK3_EN_MASK) != 0u;
  const uint8_t effective_lock_abn_speed = static_cast<uint8_t>(
    (fault_config2_effective & FAULT_CONFIG2_LOCK_ABN_SPEED_MASK) >>
    FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT
  );
  const uint8_t effective_abnormal_bemf_threshold = static_cast<uint8_t>(
    (fault_config2_effective & FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK) >>
    FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT
  );
  const uint8_t effective_no_motor_threshold = static_cast<uint8_t>(
    (fault_config2_effective & FAULT_CONFIG2_NO_MTR_THR_MASK) >> FAULT_CONFIG2_NO_MTR_THR_SHIFT
  );
  const uint8_t effective_csa_gain_code = static_cast<uint8_t>(
    (gd_config1_effective & GD_CONFIG1_CSA_GAIN_MASK) >> GD_CONFIG1_CSA_GAIN_SHIFT
  );
  const uint16_t effective_base_current_code = static_cast<uint16_t>(
    (gd_config2_effective & GD_CONFIG2_BASE_CURRENT_MASK) >> GD_CONFIG2_BASE_CURRENT_SHIFT
  );
  const uint8_t effective_cl_slow_acc = static_cast<uint8_t>(
    (int_algo2_effective & INT_ALGO_2_CL_SLOW_ACC_MASK) >> INT_ALGO_2_CL_SLOW_ACC_SHIFT
  );
  static constexpr float CSA_GAIN_VV_TABLE[4] = {5.0f, 10.0f, 20.0f, 40.0f};
  const float effective_csa_gain_vv = CSA_GAIN_VV_TABLE[effective_csa_gain_code & 0x3u];
  const float effective_base_current_amps =
    (static_cast<float>(effective_base_current_code) * 1200.0f) / 32768.0f;
  char summary[1280];
  std::snprintf(
    summary,
    sizeof(summary),
    "profile=custom dir=%s csa_gain=%u(%.0fV/V) base_current=%u(~%.2fA) "
    "bemf=0x%02X mres=%u mind=%u spd_kp=%u spd_ki=%u "
    "max_speed=%.1fHz(code=%u) "
    "mode=%s align=%s align_ilimit=%u(%u%%) ol_limit_src=%s ol_ilimit=%u(%u%%) "
    "ol_acc_a1=%.2fHz/s ol_acc_a2=%.2fHz/s2 auto_handoff=%s handoff=%.1f%% theta_ramp=%.2f "
    "cl_slow_acc=%.1fHz/s "
    "stop=%s stop_brake=%s ilimit=%u(%u%%) lock_ilimit=%u(%u%%) hw_lock_ilimit=%u(%u%%) "
    "lock_mode=%s mtr_lock_mode=%s hw_lock_mode=%s lck_retry=%s lock_deg=%.1fms hw_lock_deg=%uus "
    "lock1=%s lock2=%s lock3=%s lock_abn_speed=%s abn_bemf_thr=%s no_mtr_thr=%s",
    effective_direction.c_str(),
    static_cast<unsigned>(effective_csa_gain_code),
    effective_csa_gain_vv,
    static_cast<unsigned>(effective_base_current_code),
    effective_base_current_amps,
    static_cast<unsigned>(effective_motor_bemf_const),
    static_cast<unsigned>(effective_motor_res),
    static_cast<unsigned>(effective_motor_ind),
    static_cast<unsigned>(effective_spd_loop_kp),
    static_cast<unsigned>(effective_spd_loop_ki),
    effective_max_speed_hz,
    static_cast<unsigned>(effective_max_speed_code),
    this->mode_to_string_(effective_cfg_mode),
    this->align_time_to_string_(effective_align_time),
    static_cast<unsigned>(effective_align_or_slow_ilimit),
    static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[effective_align_or_slow_ilimit & 0x0Fu]),
    effective_open_loop_limit_use_ilimit ? "ilimit" : "ol_ilimit",
    static_cast<unsigned>(effective_ol_ilimit),
    static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[effective_ol_ilimit & 0x0Fu]),
    this->open_loop_accel_code_to_hz_per_s_(effective_ol_accel),
    OPEN_LOOP_ACCEL2_HZ_PER_S2_TABLE[effective_ol_accel2 & 0x0Fu],
    YESNO(effective_auto_handoff),
    this->open_to_closed_handoff_code_to_percent_(effective_handoff_threshold),
    THETA_ERROR_RAMP_RATE_TABLE[effective_theta_error_ramp_rate & 0x07u],
    CL_SLOW_ACC_HZ_PER_S_TABLE[effective_cl_slow_acc & 0x0Fu],
    this->brake_mode_to_string_(effective_brake_mode),
    this->brake_time_to_string_(effective_brake_time),
    static_cast<unsigned>(effective_ilimit),
    static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[effective_ilimit & 0x0Fu]),
    static_cast<unsigned>(effective_lock_ilimit),
    static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[effective_lock_ilimit & 0x0Fu]),
    static_cast<unsigned>(effective_hw_lock_ilimit),
    static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[effective_hw_lock_ilimit & 0x0Fu]),
    this->lock_mode_to_string_(effective_lock_mode),
    this->lock_mode_to_string_(effective_mtr_lock_mode),
    this->lock_mode_to_string_(effective_hw_lock_mode),
    this->lock_retry_time_to_string_(effective_lck_retry),
    LOCK_ILIMIT_DEGLITCH_MS_TABLE[effective_lock_ilimit_deg & 0x0Fu],
    static_cast<unsigned>(HW_LOCK_ILIMIT_DEGLITCH_US_TABLE[effective_hw_lock_ilimit_deg & 0x07u]),
    YESNO(effective_lock1_en),
    YESNO(effective_lock2_en),
    YESNO(effective_lock3_en),
    LOCK_ABN_SPEED_THRESHOLD_LABELS[effective_lock_abn_speed & 0x7u],
    ABNORMAL_BEMF_THRESHOLD_LABELS[effective_abnormal_bemf_threshold & 0x7u],
    NO_MOTOR_THRESHOLD_LABELS[effective_no_motor_threshold & 0x7u]
  );
  this->motor_config_summary_ = summary;
  ESP_LOGI(TAG, "Motor config: %s", this->motor_config_summary_.c_str());
  return ok;
}

bool MCF8329AComponent::read_reg32(uint16_t offset, uint32_t& value) {
  return this->perform_read_(offset, value);
}

bool MCF8329AComponent::read_reg16(uint16_t offset, uint16_t& value) {
  return this->perform_read16_(offset, value);
}

bool MCF8329AComponent::write_reg32(uint16_t offset, uint32_t value) {
  return this->perform_write_(offset, value);
}

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

bool MCF8329AComponent::set_direction_mode(const std::string& direction_mode) {
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

bool MCF8329AComponent::apply_speed_command_(float speed_percent, const char* reason, bool publish_number) {
  float clamped = speed_percent;
  if (std::isnan(clamped)) {
    return false;
  }
  if (clamped < 0.0f) {
    clamped = 0.0f;
  } else if (clamped > 100.0f) {
    clamped = 100.0f;
  }

  if (clamped > 0.0f && this->severe_fault_speed_lockout_) {
    ESP_LOGE(
      TAG,
      "Speed command blocked by safety lockout after severe current fault. "
      "Clear active faults before commanding non-zero speed."
    );
    return false;
  }

  const uint16_t digital_speed_ctrl = static_cast<uint16_t>(lroundf((clamped / 100.0f) * 32767.0f));

  if (clamped > 0.0f && this->clear_mpet_on_startup_) {
    (void)this->clear_mpet_bits_("speed_cmd");
  }

  if (clamped > 0.0f) {
    if (!this->set_brake_override(false)) {
      ESP_LOGW(TAG, "Failed to release brake before speed command");
      return false;
    }
    if (this->brake_switch_ != nullptr) {
      this->brake_switch_->publish_state(false);
    }
  }

  uint32_t value = ALGO_DEBUG1_OVERRIDE_MASK;
  value |= (static_cast<uint32_t>(digital_speed_ctrl) << 16) & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK;

  if (!this->update_bits32(
        REG_ALGO_DEBUG1, ALGO_DEBUG1_OVERRIDE_MASK | ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK, value
      )) {
    return false;
  }

  if (reason != nullptr && std::strcmp(reason, "ramp_step") == 0) {
    ESP_LOGD(TAG, "Speed ramp write: %.1f%% (raw=%u)", clamped, static_cast<unsigned>(digital_speed_ctrl));
  } else {
    ESP_LOGI(
      TAG,
      "Speed command write [%s]: %.1f%% (raw=%u)",
      reason != nullptr ? reason : "external",
      clamped,
      static_cast<unsigned>(digital_speed_ctrl)
    );
  }

  this->speed_applied_percent_ = clamped;
  if (publish_number && this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(clamped);
  }
  return true;
}

void MCF8329AComponent::process_speed_command_ramp_() {
  if (!this->speed_target_active_) {
    return;
  }
  if (this->severe_fault_speed_lockout_) {
    this->speed_target_active_ = false;
    this->start_boost_active_ = false;
    this->start_boost_until_ms_ = 0u;
    this->last_ramp_update_ms_ = 0u;
    return;
  }

  const uint32_t now = millis();
  if (this->last_ramp_update_ms_ == 0u) {
    this->last_ramp_update_ms_ = now;
  }
  float dt = static_cast<float>(now - this->last_ramp_update_ms_) / 1000.0f;
  if (dt < 0.0f) {
    dt = 0.0f;
  }
  if (dt == 0.0f && std::fabs(this->speed_target_percent_ - this->speed_applied_percent_) > 0.0f) {
    dt = 0.01f;
  }
  this->last_ramp_update_ms_ = now;

  float desired = this->speed_target_percent_;
  if (this->start_boost_active_) {
    if (this->start_boost_hold_ms_ == 0u || now >= this->start_boost_until_ms_) {
      this->start_boost_active_ = false;
    } else {
      desired = std::max(desired, this->start_boost_percent_);
    }
  }

  float next = this->speed_applied_percent_;
  if (desired > this->speed_applied_percent_) {
    if (this->speed_ramp_up_percent_per_s_ <= 0.0f) {
      next = desired;
    } else {
      next = std::min(desired, this->speed_applied_percent_ + (this->speed_ramp_up_percent_per_s_ * dt));
    }
  } else if (desired < this->speed_applied_percent_) {
    if (this->speed_ramp_down_percent_per_s_ <= 0.0f) {
      next = desired;
    } else {
      next = std::max(desired, this->speed_applied_percent_ - (this->speed_ramp_down_percent_per_s_ * dt));
    }
  }

  if (std::fabs(next - this->speed_applied_percent_) > 0.001f) {
    (void)this->apply_speed_command_(next, "ramp_step", false);
  }

  if (!this->start_boost_active_ && std::fabs(this->speed_target_percent_ - this->speed_applied_percent_) <= 0.05f) {
    this->speed_target_active_ = false;
    this->last_ramp_update_ms_ = 0u;
  }
}

bool MCF8329AComponent::set_speed_percent(float speed_percent, const char* reason) {
  if (std::isnan(speed_percent)) {
    return false;
  }

  float clamped = speed_percent;
  if (clamped < 0.0f) {
    clamped = 0.0f;
  } else if (clamped > 100.0f) {
    clamped = 100.0f;
  }

  if (clamped > 0.0f && this->severe_fault_speed_lockout_) {
    ESP_LOGE(
      TAG,
      "Speed command blocked by safety lockout after severe current fault. "
      "Clear active faults before commanding non-zero speed."
    );
    return false;
  }

  if (clamped == 0.0f) {
    this->speed_target_percent_ = 0.0f;
    this->speed_target_active_ = false;
    this->start_boost_active_ = false;
    this->start_boost_until_ms_ = 0u;
    this->last_ramp_update_ms_ = 0u;
    return this->apply_speed_command_(0.0f, reason, true);
  }

  const bool ramp_enabled =
    this->speed_ramp_up_percent_per_s_ > 0.0f || this->speed_ramp_down_percent_per_s_ > 0.0f;
  const bool start_boost_enabled =
    this->start_boost_percent_ > clamped && this->start_boost_hold_ms_ > 0u &&
    this->speed_applied_percent_ <= 0.05f;

  if (!ramp_enabled && !start_boost_enabled) {
    this->speed_target_active_ = false;
    this->start_boost_active_ = false;
    this->start_boost_until_ms_ = 0u;
    this->last_ramp_update_ms_ = 0u;
    return this->apply_speed_command_(clamped, reason, true);
  }

  this->speed_target_percent_ = clamped;
  this->speed_target_active_ = true;
  if (start_boost_enabled) {
    this->start_boost_active_ = true;
    this->start_boost_until_ms_ = millis() + this->start_boost_hold_ms_;
  } else {
    this->start_boost_active_ = false;
    this->start_boost_until_ms_ = 0u;
  }

  this->last_ramp_update_ms_ = millis();
  this->process_speed_command_ramp_();

  if (this->speed_number_ != nullptr) {
    this->speed_number_->publish_state(clamped);
  }
  return true;
}

float MCF8329AComponent::speed_raw_to_hz_(int32_t raw, float max_speed_hz) const {
  return static_cast<float>(raw) * SPEED_Q27_SCALE * max_speed_hz;
}

float MCF8329AComponent::fg_speed_raw_to_hz_(uint32_t raw, float max_speed_hz) const {
  return static_cast<float>(raw) * SPEED_Q27_SCALE * max_speed_hz;
}

void MCF8329AComponent::pulse_clear_faults() {
  ESP_LOGI(TAG, "Pulsing clear faults");

  uint32_t gate_before = 0;
  uint32_t ctrl_before = 0;
  uint32_t gate_after = 0;
  uint32_t ctrl_after = 0;
  const bool gate_before_ok = this->read_reg32(REG_GATE_DRIVER_FAULT_STATUS, gate_before);
  const bool ctrl_before_ok = this->read_reg32(REG_CONTROLLER_FAULT_STATUS, ctrl_before);

  // MPET_BEMF fault condition can remain true while MPET bits are set.
  (void)this->clear_mpet_bits_("clear_faults");

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
    this->handle_fault_shutdown_(fault_active, ctrl_after, ctrl_after_ok);
    if (!fault_active && this->severe_fault_speed_lockout_) {
      this->severe_fault_speed_lockout_ = false;
      ESP_LOGI(TAG, "Safety lockout cleared after faults were cleared");
    }
  }
}

void MCF8329AComponent::pulse_watchdog_tickle() {
  ESP_LOGD(TAG, "Pulsing watchdog tickle");
  if (this->update_bits32(
        REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK
      )) {
    (void)this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0);
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
    this->log_algorithm_state_transition_(algo_status, "probe");
  }
  if (gate_ok || controller_ok) {
    this->publish_faults_(gate_fault_status, gate_ok, controller_fault_status, controller_ok);
  }
  if (fault_state_valid && this->fault_active_binary_sensor_ != nullptr) {
    this->fault_active_binary_sensor_->publish_state(fault_active);
  }

  return ok;
}

bool MCF8329AComponent::clear_mpet_bits_(const char* context) {
  uint32_t algo_debug2 = 0;
  if (!this->read_reg32(REG_ALGO_DEBUG2, algo_debug2)) {
    ESP_LOGW(TAG, "[%s] Failed to read ALGO_DEBUG2 for MPET clear", context);
    return false;
  }

  const uint32_t mpet_bits = algo_debug2 & ALGO_DEBUG2_MPET_ALL_MASK;
  if (mpet_bits == 0u) {
    return true;
  }

  const uint32_t next = algo_debug2 & ~ALGO_DEBUG2_MPET_ALL_MASK;
  if (!this->write_reg32(REG_ALGO_DEBUG2, next)) {
    ESP_LOGW(TAG, "[%s] Failed to clear MPET bits in ALGO_DEBUG2", context);
    return false;
  }

  ESP_LOGI(
    TAG, "[%s] Cleared MPET bits in ALGO_DEBUG2: 0x%08X -> 0x%08X", context, algo_debug2, next
  );
  return true;
}

bool MCF8329AComponent::perform_read_(uint16_t offset, uint32_t& value) {
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

  value = static_cast<uint32_t>(rx[0]) | (static_cast<uint32_t>(rx[1]) << 8) |
          (static_cast<uint32_t>(rx[2]) << 16) | (static_cast<uint32_t>(rx[3]) << 24);
  return true;
}

bool MCF8329AComponent::perform_read16_(uint16_t offset, uint16_t& value) {
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
    ESP_LOGW(
      TAG, "write_reg32(0x%04X, 0x%08X) failed: i2c error %d", offset, value, static_cast<int>(err)
    );
    return false;
  }
  return true;
}

uint32_t MCF8329AComponent::build_control_word_(bool is_read, uint16_t offset, bool is_32bit)
  const {
  const uint32_t dlen = is_32bit ? 0x1u : 0x0u;
  return ((is_read ? 1u : 0u) << 23) | (dlen << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

void MCF8329AComponent::publish_faults_(
  uint32_t gate_fault_status,
  bool gate_fault_valid,
  uint32_t controller_fault_status,
  bool controller_fault_valid
) {
  std::vector<std::string> faults;
  bool gate_detail_found = false;
  bool controller_detail_found = false;
  const bool mpet_bemf_active =
    controller_fault_valid && ((controller_fault_status & FAULT_MPET_BEMF) != 0u);
  const bool hw_lock_active =
    controller_fault_valid && ((controller_fault_status & FAULT_HW_LOCK_LIMIT) != 0u);

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
        summary += ", ";
      }
      summary += faults[i];
    }
  }

  if (this->current_fault_text_sensor_ != nullptr) {
    this->current_fault_text_sensor_->publish_state(summary);
  }

  if (mpet_bemf_active) {
    if (!this->mpet_bemf_fault_latched_) {
      this->mpet_bemf_fault_latched_ = true;
      this->log_mpet_bemf_diagnostics_();
    }
  } else {
    this->mpet_bemf_fault_latched_ = false;
  }

  if (hw_lock_active) {
    if (!this->hw_lock_fault_latched_) {
      this->hw_lock_fault_latched_ = true;
      this->log_hw_lock_diagnostics_();
    }
  } else {
    this->hw_lock_fault_latched_ = false;
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

void MCF8329AComponent::log_mpet_bemf_diagnostics_() {
  uint32_t algo_debug1 = 0;
  uint32_t algo_debug2 = 0;
  uint32_t pin_config = 0;
  uint32_t closed_loop2 = 0;
  uint32_t mtr_params = 0;
  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;

  const bool algo_debug1_ok = this->read_reg32(REG_ALGO_DEBUG1, algo_debug1);
  const bool algo_debug2_ok = this->read_reg32(REG_ALGO_DEBUG2, algo_debug2);
  const bool pin_config_ok = this->read_reg32(REG_PIN_CONFIG, pin_config);
  const bool closed_loop2_ok = this->read_reg32(REG_CLOSED_LOOP2, closed_loop2);
  const bool mtr_params_ok = this->read_reg32(REG_MTR_PARAMS, mtr_params);
  const bool closed_loop3_ok = this->read_reg32(REG_CLOSED_LOOP3, closed_loop3);
  const bool closed_loop4_ok = this->read_reg32(REG_CLOSED_LOOP4, closed_loop4);

  float speed_cmd_percent = -1.0f;
  if (algo_debug1_ok) {
    const uint16_t digital_speed_ctrl =
      static_cast<uint16_t>((algo_debug1 & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16);
    speed_cmd_percent = (static_cast<float>(digital_speed_ctrl) * 100.0f) / 32767.0f;
  }

  const uint32_t brake_input =
    pin_config_ok ? ((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 2) : 0u;
  const uint32_t motor_bemf_const =
    mtr_params_ok
      ? ((mtr_params & MTR_PARAMS_MOTOR_BEMF_CONST_MASK) >> MTR_PARAMS_MOTOR_BEMF_CONST_SHIFT)
      : 0u;
  const uint32_t configured_motor_res =
    closed_loop2_ok ? ((closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT)
                    : 0u;
  const uint32_t configured_motor_ind =
    closed_loop2_ok ? ((closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT)
                    : 0u;
  const uint32_t configured_bemf_const =
    closed_loop3_ok
      ? ((closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT)
      : 0u;
  const uint32_t configured_spd_loop_kp =
    (closed_loop3_ok && closed_loop4_ok)
      ? ((((closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT)
          << 7) |
         ((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT))
      : 0u;
  const uint32_t configured_spd_loop_ki =
    closed_loop4_ok
      ? ((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT)
      : 0u;
  const uint16_t configured_max_speed_code =
    closed_loop4_ok ? static_cast<uint16_t>(
                        (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT
                      )
                    : 0u;
  const float configured_max_speed_hz = this->max_speed_code_to_hz_(configured_max_speed_code);

  if (algo_debug1_ok && algo_debug2_ok && pin_config_ok && closed_loop2_ok && mtr_params_ok && closed_loop3_ok && closed_loop4_ok) {
    ESP_LOGW(
      TAG,
      "MPET_BEMF diag: speed_cmd=%.1f%% brake=%s mpet(cmd=%s ke=%s mech=%s write_shadow=%s) "
      "measured_bemf=%u configured_bemf=0x%02X configured_mres=%u configured_mind=%u "
      "configured_spd_kp=%u configured_spd_ki=%u configured_max_speed=%.1fHz(code=%u)",
      speed_cmd_percent,
      this->brake_input_to_string_(brake_input),
      YESNO((algo_debug2 & ALGO_DEBUG2_MPET_CMD_MASK) != 0u),
      YESNO((algo_debug2 & ALGO_DEBUG2_MPET_KE_MASK) != 0u),
      YESNO((algo_debug2 & ALGO_DEBUG2_MPET_MECH_MASK) != 0u),
      YESNO((algo_debug2 & ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK) != 0u),
      static_cast<unsigned>(motor_bemf_const),
      static_cast<unsigned>(configured_bemf_const),
      static_cast<unsigned>(configured_motor_res),
      static_cast<unsigned>(configured_motor_ind),
      static_cast<unsigned>(configured_spd_loop_kp),
      static_cast<unsigned>(configured_spd_loop_ki),
      configured_max_speed_hz,
      static_cast<unsigned>(configured_max_speed_code)
    );
  } else {
    ESP_LOGW(
      TAG,
      "MPET_BEMF diag: read failure ad1=%s ad2=%s pin_cfg=%s cl2=%s mtr_params=%s closed_loop3=%s "
      "closed_loop4=%s",
      YESNO(algo_debug1_ok),
      YESNO(algo_debug2_ok),
      YESNO(pin_config_ok),
      YESNO(closed_loop2_ok),
      YESNO(mtr_params_ok),
      YESNO(closed_loop3_ok),
      YESNO(closed_loop4_ok)
    );
  }

  ESP_LOGW(
    TAG,
    "MPET_BEMF hint: ensure brake is OFF before commanding speed; if starting from low duty, try "
    ">10%% then ramp down."
  );
}

void MCF8329AComponent::log_hw_lock_diagnostics_() {
  uint32_t algo_debug1 = 0;
  uint32_t fault_config1 = 0;
  uint32_t fault_config2 = 0;

  const bool algo_debug1_ok = this->read_reg32(REG_ALGO_DEBUG1, algo_debug1);
  const bool fault_config1_ok = this->read_reg32(REG_FAULT_CONFIG1, fault_config1);
  const bool fault_config2_ok = this->read_reg32(REG_FAULT_CONFIG2, fault_config2);

  float speed_cmd_percent = -1.0f;
  if (algo_debug1_ok) {
    const uint16_t digital_speed_ctrl =
      static_cast<uint16_t>((algo_debug1 & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16);
    speed_cmd_percent = (static_cast<float>(digital_speed_ctrl) * 100.0f) / 32767.0f;
  }

  if (algo_debug1_ok && fault_config1_ok && fault_config2_ok) {
    const uint8_t ilimit = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_ILIMIT_MASK) >> FAULT_CONFIG1_ILIMIT_SHIFT
    );
    const uint8_t hw_lock_ilimit = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT
    );
    const uint8_t lock_ilimit = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_SHIFT
    );
    const uint8_t lock_mode = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK) >> FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT
    );
    const uint8_t mtr_lock_mode = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_MTR_LCK_MODE_MASK) >> FAULT_CONFIG1_MTR_LCK_MODE_SHIFT
    );
    const uint8_t hw_lock_mode = static_cast<uint8_t>(
      (fault_config2 & FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK) >>
      FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT
    );
    const uint8_t lck_retry = static_cast<uint8_t>(
      (fault_config1 & FAULT_CONFIG1_LCK_RETRY_MASK) >> FAULT_CONFIG1_LCK_RETRY_SHIFT
    );
    const bool lock1_en = (fault_config2 & FAULT_CONFIG2_LOCK1_EN_MASK) != 0u;
    const bool lock2_en = (fault_config2 & FAULT_CONFIG2_LOCK2_EN_MASK) != 0u;
    const bool lock3_en = (fault_config2 & FAULT_CONFIG2_LOCK3_EN_MASK) != 0u;
    const uint8_t lock_abn_speed = static_cast<uint8_t>(
      (fault_config2 & FAULT_CONFIG2_LOCK_ABN_SPEED_MASK) >> FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT
    );
    const uint8_t abnormal_bemf_threshold = static_cast<uint8_t>(
      (fault_config2 & FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK) >>
      FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT
    );
    const uint8_t no_motor_threshold = static_cast<uint8_t>(
      (fault_config2 & FAULT_CONFIG2_NO_MTR_THR_MASK) >> FAULT_CONFIG2_NO_MTR_THR_SHIFT
    );

    ESP_LOGW(
      TAG,
      "HW_LOCK_LIMIT diag: speed_cmd=%.1f%% ilimit=%u(%u%%) lock_ilimit=%u(%u%%) "
      "hw_lock_ilimit=%u(%u%%) "
      "lock_mode=%u(%s) mtr_lock_mode=%u(%s) hw_lock_mode=%u(%s) lck_retry=%u(%s) "
      "lock1=%s lock2=%s lock3=%s lock_abn_speed=%s abn_bemf_thr=%s no_mtr_thr=%s",
      speed_cmd_percent,
      static_cast<unsigned>(ilimit),
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[ilimit & 0x0Fu]),
      static_cast<unsigned>(lock_ilimit),
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[lock_ilimit & 0x0Fu]),
      static_cast<unsigned>(hw_lock_ilimit),
      static_cast<unsigned>(LOCK_ILIMIT_PERCENT_TABLE[hw_lock_ilimit & 0x0Fu]),
      static_cast<unsigned>(lock_mode),
      this->lock_mode_to_string_(lock_mode),
      static_cast<unsigned>(mtr_lock_mode),
      this->lock_mode_to_string_(mtr_lock_mode),
      static_cast<unsigned>(hw_lock_mode),
      this->lock_mode_to_string_(hw_lock_mode),
      static_cast<unsigned>(lck_retry),
      this->lock_retry_time_to_string_(lck_retry),
      YESNO(lock1_en),
      YESNO(lock2_en),
      YESNO(lock3_en),
      LOCK_ABN_SPEED_THRESHOLD_LABELS[lock_abn_speed & 0x7u],
      ABNORMAL_BEMF_THRESHOLD_LABELS[abnormal_bemf_threshold & 0x7u],
      NO_MOTOR_THRESHOLD_LABELS[no_motor_threshold & 0x7u]
    );

    const bool limits_at_guardrail =
      ilimit >= 7u && lock_ilimit >= 7u && hw_lock_ilimit >= 7u;
    if (limits_at_guardrail) {
      ESP_LOGW(
        TAG,
        "HW_LOCK_LIMIT hint: limits are already at the 50%% safety guardrail. "
        "Do not raise current further; tune mode/alignment/open-loop handoff, reduce load, "
        "and verify motor phase wiring."
      );
    } else {
      ESP_LOGW(
        TAG,
        "HW_LOCK_LIMIT hint: use cfg_lock_mode=retry and tune motor currents within the "
        "guardrail range (30-50%%), then adjust open-loop accel/handoff."
      );
    }
  } else {
    ESP_LOGW(
      TAG,
      "HW_LOCK_LIMIT diag: read failure ad1=%s fault_cfg1=%s fault_cfg2=%s",
      YESNO(algo_debug1_ok),
      YESNO(fault_config1_ok),
      YESNO(fault_config2_ok)
    );
    ESP_LOGW(
      TAG,
      "HW_LOCK_LIMIT hint: keep current limits within guardrails and tune mode/alignment/"
      "handoff before increasing stress."
    );
  }
}

const char* MCF8329AComponent::mode_to_string_(uint8_t mode) const {
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

const char* MCF8329AComponent::align_time_to_string_(uint8_t code) const {
  static const char* const kAlignTimeLabels[16] = {
    "10ms",
    "50ms",
    "100ms",
    "200ms",
    "300ms",
    "400ms",
    "500ms",
    "750ms",
    "1000ms",
    "1500ms",
    "2000ms",
    "3000ms",
    "4000ms",
    "5000ms",
    "7500ms",
    "10000ms",
  };
  return kAlignTimeLabels[code & 0x0Fu];
}

const char* MCF8329AComponent::brake_mode_to_string_(uint8_t code) const {
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

const char* MCF8329AComponent::brake_time_to_string_(uint8_t code) const {
  static const char* const kBrakeTimeLabels[16] = {
    "1ms",
    "1ms",
    "1ms",
    "1ms",
    "1ms",
    "5ms",
    "10ms",
    "50ms",
    "100ms",
    "250ms",
    "500ms",
    "1000ms",
    "2500ms",
    "5000ms",
    "10000ms",
    "15000ms",
  };
  return kBrakeTimeLabels[code & 0x0Fu];
}

float MCF8329AComponent::max_speed_code_to_hz_(uint16_t code) const {
  const uint16_t clamped = code & 0x3FFFu;
  if (clamped <= 9600u) {
    return static_cast<float>(clamped) / 6.0f;
  }
  return (static_cast<float>(clamped) / 4.0f) - 800.0f;
}

float MCF8329AComponent::open_loop_accel_code_to_hz_per_s_(uint8_t code) const {
  return OPEN_LOOP_ACCEL_HZ_PER_S_TABLE[code & 0x0Fu];
}

float MCF8329AComponent::open_to_closed_handoff_code_to_percent_(uint8_t code) const {
  return OPEN_TO_CLOSED_HANDOFF_PERCENT_TABLE[code & 0x1Fu];
}

const char* MCF8329AComponent::algorithm_state_to_string_(uint16_t state) const {
  switch (state) {
    case 0x0000:
      return "MOTOR_IDLE";
    case 0x0001:
      return "MOTOR_ISD";
    case 0x0002:
      return "MOTOR_TRISTATE";
    case 0x0003:
      return "MOTOR_BRAKE_ON_START";
    case 0x0004:
      return "MOTOR_IPD";
    case 0x0005:
      return "MOTOR_SLOW_FIRST_CYCLE";
    case 0x0006:
      return "MOTOR_ALIGN";
    case 0x0007:
      return "MOTOR_OPEN_LOOP";
    case 0x0008:
      return "MOTOR_CLOSED_LOOP_UNALIGNED";
    case 0x0009:
      return "MOTOR_CLOSED_LOOP_ALIGNED";
    case 0x000A:
      return "MOTOR_CLOSED_LOOP_ACTIVE_BRAKING";
    case 0x000B:
      return "MOTOR_SOFT_STOP";
    case 0x000C:
      return "MOTOR_RECIRCULATE_STOP";
    case 0x000D:
      return "MOTOR_BRAKE_ON_STOP";
    case 0x000E:
      return "MOTOR_FAULT";
    case 0x000F:
      return "MOTOR_MPET_MOTOR_STOP_CHECK";
    case 0x0010:
      return "MOTOR_MPET_MOTOR_STOP_WAIT";
    case 0x0011:
      return "MOTOR_MPET_MOTOR_BRAKE";
    case 0x0012:
      return "MOTOR_MPET_ALGORITHM_PARAMETERS_INIT";
    case 0x0013:
      return "MOTOR_MPET_RL_MEASURE";
    case 0x0014:
      return "MOTOR_MPET_KE_MEASURE";
    case 0x0015:
      return "MOTOR_MPET_STALL_CURRENT_MEASURE";
    case 0x0016:
      return "MOTOR_MPET_TORQUE_MODE";
    case 0x0017:
      return "MOTOR_MPET_DONE";
    case 0x0018:
      return "MOTOR_MPET_FAULT";
    default:
      return "MOTOR_STATE_UNKNOWN";
  }
}

void MCF8329AComponent::log_algorithm_state_transition_(uint32_t algo_status, const char* context) {
  uint16_t algo_state = 0;
  if (!this->read_reg16(REG_ALGORITHM_STATE, algo_state)) {
    if (!this->algorithm_state_read_error_latched_) {
      ESP_LOGW(TAG, "Unable to read ALGORITHM_STATE for %s logging", context);
      this->algorithm_state_read_error_latched_ = true;
    }
    return;
  }
  this->algorithm_state_read_error_latched_ = false;

  if (this->algorithm_state_valid_ && algo_state == this->last_algorithm_state_) {
    return;
  }

  const uint16_t duty_raw =
    static_cast<uint16_t>((algo_status & ALGO_STATUS_DUTY_CMD_MASK) >> ALGO_STATUS_DUTY_CMD_SHIFT);
  const uint16_t volt_mag_raw =
    static_cast<uint16_t>((algo_status & ALGO_STATUS_VOLT_MAG_MASK) >> ALGO_STATUS_VOLT_MAG_SHIFT);
  const float duty_percent = (static_cast<float>(duty_raw) / 4095.0f) * 100.0f;
  const float volt_mag_percent = (static_cast<float>(volt_mag_raw) * 100.0f) / 32768.0f;
  const bool sys_enable = (algo_status & ALGO_STATUS_SYS_ENABLE_FLAG_MASK) != 0u;
  float speed_cmd_percent = -1.0f;
  uint32_t algo_debug1 = 0;
  if (this->read_reg32(REG_ALGO_DEBUG1, algo_debug1)) {
    const uint16_t digital_speed_ctrl =
      static_cast<uint16_t>((algo_debug1 & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16);
    speed_cmd_percent = (static_cast<float>(digital_speed_ctrl) * 100.0f) / 32767.0f;
  }

  if (!this->algorithm_state_valid_) {
    ESP_LOGI(
      TAG,
      "Algorithm state [%s]: init -> 0x%04X(%s) speed_cmd=%.1f%% duty=%.1f%% volt_mag=%.1f%% "
      "sys_enable=%s",
      context,
      static_cast<unsigned>(algo_state),
      this->algorithm_state_to_string_(algo_state),
      speed_cmd_percent,
      duty_percent,
      volt_mag_percent,
      YESNO(sys_enable)
    );
  } else {
    ESP_LOGI(
      TAG,
      "Algorithm state [%s]: 0x%04X(%s) -> 0x%04X(%s) speed_cmd=%.1f%% duty=%.1f%% volt_mag=%.1f%% "
      "sys_enable=%s",
      context,
      static_cast<unsigned>(this->last_algorithm_state_),
      this->algorithm_state_to_string_(this->last_algorithm_state_),
      static_cast<unsigned>(algo_state),
      this->algorithm_state_to_string_(algo_state),
      speed_cmd_percent,
      duty_percent,
      volt_mag_percent,
      YESNO(sys_enable)
    );
  }

  this->last_algorithm_state_ = algo_state;
  this->algorithm_state_valid_ = true;
}

const char* MCF8329AComponent::lock_mode_to_string_(uint8_t mode) const {
  switch (mode & 0x0Fu) {
    case 0:
    case 1:
      return "latched_tristate";
    case 2:
    case 3:
      return "latched_low_side_brake";
    case 4:
    case 5:
      return "retry_tristate";
    case 6:
    case 7:
      return "retry_low_side_brake";
    case 8:
      return "report_only";
    default:
      return "disabled";
  }
}

const char* MCF8329AComponent::lock_retry_time_to_string_(uint8_t code) const {
  static const char* const kLockRetryLabels[16] = {
    "300ms",
    "500ms",
    "1s",
    "2s",
    "3s",
    "4s",
    "5s",
    "6s",
    "7s",
    "8s",
    "9s",
    "10s",
    "11s",
    "12s",
    "13s",
    "14s",
  };
  return kLockRetryLabels[code & 0x0Fu];
}

const char* MCF8329AComponent::brake_input_to_string_(uint32_t brake_input_value) const {
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

const char* MCF8329AComponent::direction_input_to_string_(uint32_t direction_input_value) const {
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

void MCF8329AComponent::handle_fault_shutdown_(
  bool fault_active, uint32_t controller_fault_status, bool controller_fault_valid
) {
  if (!fault_active) {
    this->fault_latched_ = false;
    return;
  }

  if (this->severe_current_fault_active_(controller_fault_status, controller_fault_valid)) {
    if (!this->severe_fault_speed_lockout_) {
      this->severe_fault_speed_lockout_ = true;
      ESP_LOGE(
        TAG,
        "Safety lockout engaged due to severe current fault. "
        "Non-zero speed commands are blocked until faults are cleared."
      );
    }
  }

  if (this->fault_latched_) {
    return;
  }

  this->fault_latched_ = true;
  ESP_LOGW(TAG, "Fault detected, forcing speed command to 0%%");
  (void)this->set_speed_percent(0.0f, "fault_shutdown");
}

bool MCF8329AComponent::severe_current_fault_active_(
  uint32_t controller_fault_status, bool controller_fault_valid
) const {
  if (!controller_fault_valid) {
    return false;
  }
  const uint32_t severe_faults = FAULT_HW_LOCK_LIMIT | FAULT_LOCK_LIMIT | FAULT_BUS_CURRENT_LIMIT;
  return (controller_fault_status & severe_faults) != 0u;
}

}  // namespace mcf8329a
}  // namespace esphome
