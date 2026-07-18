#include "mcf8316d_tuning.h"

#include "mcf8316d.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8316d {

using namespace ::mcf8316d_core::regs;

static const char *const TUNING_TAG = "mcf8316d";

namespace {

constexpr uint32_t STARTUP_TUNE_LOCK_ILIMIT_DEG = 6u;
constexpr uint32_t STARTUP_TUNE_LCK_RETRY = 2u;
constexpr uint32_t STARTUP_TUNE_LOCK_ILIMIT_MODE = 3u;  // retry_hiz
constexpr uint32_t STARTUP_TUNE_MTR_LCK_MODE = 3u;      // retry_hiz
constexpr uint32_t STARTUP_TUNE_HW_LOCK_ILIMIT = 15u;
constexpr uint32_t STARTUP_TUNE_HW_LOCK_ILIMIT_DEG = 7u;
constexpr uint32_t STARTUP_TUNE_HW_LOCK_ILIMIT_MODE = 3u;  // retry_hiz
constexpr bool STARTUP_TUNE_LOCK2_EN = false;
constexpr uint32_t STARTUP_TUNE_MTR_STARTUP = 1u;  // Double-align startup
constexpr uint32_t STARTUP_TUNE_ALIGN_TIME = 2u;   // 100ms
constexpr uint32_t STARTUP_TUNE_ALIGN_OR_SLOW_CURRENT_ILIMIT = 6u;
constexpr uint32_t STARTUP_TUNE_OL_ILIMIT = 6u;
constexpr uint32_t STARTUP_TUNE_AUTO_HANDOFF_EN = 0u;
constexpr uint32_t STARTUP_TUNE_OPN_CL_HANDOFF_THR = 0x08u;
constexpr uint32_t STARTUP_TUNE_ALIGN_ANGLE = 0x08u;  // 90 deg
constexpr uint32_t STARTUP_TUNE_SLOW_FIRST_CYC_FREQ = 1u;
constexpr bool STARTUP_TUNE_FIRST_CYCLE_FREQ_SEL = true;
constexpr uint32_t STARTUP_TUNE_MAX_SPEED = 0x2710u;   // 10000 -> 1666 Hz electrical
constexpr uint32_t STARTUP_TUNE_PWM_FREQ_OUT = 0x0Au;  // 60kHz
constexpr bool STARTUP_TUNE_DYNAMIC_CSA_GAIN_EN = true;
constexpr uint32_t STARTUP_TUNE_CSA_GAIN = 0u;  // 0.15V/A
constexpr bool STARTUP_TUNE_ISD_EN = false;
constexpr bool STARTUP_TUNE_BRAKE_EN = false;
constexpr bool STARTUP_TUNE_RESYNC_EN = false;
constexpr bool STARTUP_TUNE_BRK_CONFIG = false;
constexpr uint32_t STARTUP_TUNE_BRK_TIME = 1u;  // 50ms

constexpr uint32_t LOCK_DISABLED_MODE = 7u;
constexpr uint32_t DEBUG_ALIGN_MTR_STARTUP = 0u;  // Align startup
constexpr uint32_t DEBUG_ALIGN_TIME = 2u;         // 100 ms align

constexpr uint8_t STARTUP_SWEEP_STEP_COUNT = 4u;
constexpr float STARTUP_SWEEP_SPEED_PERCENT = 21.0f;
constexpr uint32_t STARTUP_SWEEP_STEP_TIMEOUT_MS = 2000u;
constexpr uint32_t STARTUP_SWEEP_INTER_STEP_DELAY_MS = 1200u;
constexpr uint32_t STARTUP_SWEEP_CLEAR_RETRY_MS = 250u;

constexpr uint8_t SCOPE_PROBE_STAGE_COUNT = 3u;
constexpr uint32_t SCOPE_PROBE_INTER_STAGE_DELAY_MS = 1500u;
constexpr uint32_t SCOPE_PROBE_CLEAR_RETRY_MS = 250u;

constexpr uint16_t ALGORITHM_STATE_OPEN_LOOP = 0x0007u;
constexpr uint16_t ALGORITHM_STATE_CLOSED_LOOP_UNALIGNED = 0x0008u;
constexpr uint16_t ALGORITHM_STATE_CLOSED_LOOP_ALIGNED = 0x0009u;

}  // namespace

bool MCF8316DTuningController::apply_startup_tune_profile() {
  ESP_LOGW(
    TUNING_TAG,
    "Applying startup tune profile (double-align + higher max speed + startup current tuning)"
  );
  bool ok = true;
  if (!this->parent_->set_speed_percent(0.0f)) {
    ESP_LOGW(TUNING_TAG, "Failed to set speed to 0%% before applying startup tune");
    ok = false;
  }
  if (!this->parent_->set_direction_mode("cw")) {
    ESP_LOGW(TUNING_TAG, "Failed to force direction to cw for startup tune");
    ok = false;
  } else if (this->parent_->direction_select_ != nullptr) {
    this->parent_->direction_select_->publish_state("cw");
  }
  if (!this->parent_->set_brake_override(false)) {
    ESP_LOGW(TUNING_TAG, "Failed to force brake OFF for startup tune");
    ok = false;
  } else if (this->parent_->brake_switch_ != nullptr) {
    this->parent_->brake_switch_->publish_state(false);
  }

  auto apply_masked_bits =
    [this](const char *label, uint16_t reg, uint32_t mask, uint32_t value) {
      uint32_t before = 0;
      if (!this->parent_->read_reg32(reg, before)) {
        ESP_LOGW(TUNING_TAG, "%s read failed (reg=0x%04X)", label, reg);
        return false;
      }

      const uint32_t next = (before & ~mask) | (value & mask);
      if (next != before && !this->parent_->write_reg32(reg, next)) {
        ESP_LOGW(
          TUNING_TAG, "%s write failed (reg=0x%04X): 0x%08X -> 0x%08X", label, reg, before, next
        );
        return false;
      }

      uint32_t after = 0;
      if (!this->parent_->read_reg32(reg, after)) {
        ESP_LOGW(TUNING_TAG, "%s verify read failed (reg=0x%04X)", label, reg);
        return false;
      }
      ESP_LOGI(TUNING_TAG, "%s: 0x%08X -> 0x%08X", label, before, after);

      const bool fields_match = (after & mask) == (value & mask);
      if (!fields_match) {
        ESP_LOGW(
          TUNING_TAG,
          "%s verify mismatch (reg=0x%04X): expected mask=0x%08X actual mask=0x%08X",
          label,
          reg,
          (value & mask),
          (after & mask)
        );
      }
      return fields_match;
    };

  ok &= apply_masked_bits(
    "FAULT_CONFIG1 tuning",
    register_address(RegisterId::FAULT_CONFIG1),
    FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK | FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK |
      FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK | FAULT_CONFIG1_LCK_RETRY_MASK |
      FAULT_CONFIG1_MTR_LCK_MODE_MASK,
    (STARTUP_TUNE_HW_LOCK_ILIMIT << FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT) |
      (STARTUP_TUNE_LOCK_ILIMIT_MODE << FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT) |
      (STARTUP_TUNE_LOCK_ILIMIT_DEG << FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT) |
      (STARTUP_TUNE_LCK_RETRY << FAULT_CONFIG1_LCK_RETRY_SHIFT) |
      (STARTUP_TUNE_MTR_LCK_MODE << FAULT_CONFIG1_MTR_LCK_MODE_SHIFT)
  );
  ok &= apply_masked_bits(
    "FAULT_CONFIG2 tuning",
    register_address(RegisterId::FAULT_CONFIG2),
    FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK | FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK |
      FAULT_CONFIG2_LOCK2_EN_MASK,
    (STARTUP_TUNE_HW_LOCK_ILIMIT_DEG << FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT) |
      (STARTUP_TUNE_HW_LOCK_ILIMIT_MODE << FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT) |
      (STARTUP_TUNE_LOCK2_EN ? FAULT_CONFIG2_LOCK2_EN_MASK : 0u)
  );
  ok &= apply_masked_bits(
    "MOTOR_STARTUP1 tuning",
    register_address(RegisterId::MOTOR_STARTUP1),
    MOTOR_STARTUP1_MTR_STARTUP_MASK | MOTOR_STARTUP1_ALIGN_TIME_MASK |
      MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK,
    (STARTUP_TUNE_MTR_STARTUP << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) |
      (STARTUP_TUNE_ALIGN_TIME << MOTOR_STARTUP1_ALIGN_TIME_SHIFT) |
      (STARTUP_TUNE_ALIGN_OR_SLOW_CURRENT_ILIMIT
       << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT)
  );
  ok &= apply_masked_bits(
    "MOTOR_STARTUP2 tuning",
    register_address(RegisterId::MOTOR_STARTUP2),
    MOTOR_STARTUP2_OL_ILIMIT_MASK | MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK |
      MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK | MOTOR_STARTUP2_ALIGN_ANGLE_MASK |
      MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK | MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK,
    (STARTUP_TUNE_OL_ILIMIT << MOTOR_STARTUP2_OL_ILIMIT_SHIFT) |
      (STARTUP_TUNE_AUTO_HANDOFF_EN << MOTOR_STARTUP2_AUTO_HANDOFF_EN_SHIFT) |
      (STARTUP_TUNE_OPN_CL_HANDOFF_THR << MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT) |
      (STARTUP_TUNE_ALIGN_ANGLE << MOTOR_STARTUP2_ALIGN_ANGLE_SHIFT) |
      (STARTUP_TUNE_SLOW_FIRST_CYC_FREQ << MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT) |
      (STARTUP_TUNE_FIRST_CYCLE_FREQ_SEL ? MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK : 0u)
  );
  ok &= apply_masked_bits(
    "CLOSED_LOOP1 tuning",
    register_address(RegisterId::CLOSED_LOOP1),
    CLOSED_LOOP1_PWM_FREQ_OUT_MASK,
    (STARTUP_TUNE_PWM_FREQ_OUT << CLOSED_LOOP1_PWM_FREQ_OUT_SHIFT)
  );
  ok &= apply_masked_bits(
    "DEVICE_CONFIG2 tuning",
    register_address(RegisterId::DEVICE_CONFIG2),
    DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK,
    STARTUP_TUNE_DYNAMIC_CSA_GAIN_EN ? DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK : 0u
  );
  ok &= apply_masked_bits(
    "GD_CONFIG1 tuning",
    register_address(RegisterId::GD_CONFIG1),
    GD_CONFIG1_CSA_GAIN_MASK,
    (STARTUP_TUNE_CSA_GAIN << GD_CONFIG1_CSA_GAIN_SHIFT)
  );
  ok &= apply_masked_bits(
    "ISD_CONFIG tuning",
    register_address(RegisterId::ISD_CONFIG),
    ISD_CONFIG_ISD_EN_MASK | ISD_CONFIG_BRAKE_EN_MASK | ISD_CONFIG_RESYNC_EN_MASK |
      ISD_CONFIG_BRK_CONFIG_MASK | ISD_CONFIG_BRK_TIME_MASK,
    (STARTUP_TUNE_ISD_EN ? ISD_CONFIG_ISD_EN_MASK : 0u) |
      (STARTUP_TUNE_BRAKE_EN ? ISD_CONFIG_BRAKE_EN_MASK : 0u) |
      (STARTUP_TUNE_RESYNC_EN ? ISD_CONFIG_RESYNC_EN_MASK : 0u) |
      (STARTUP_TUNE_BRK_CONFIG ? ISD_CONFIG_BRK_CONFIG_MASK : 0u) |
      (STARTUP_TUNE_BRK_TIME << ISD_CONFIG_BRK_TIME_SHIFT)
  );
  ok &= apply_masked_bits(
    "CLOSED_LOOP4 tuning",
    register_address(RegisterId::CLOSED_LOOP4),
    CLOSED_LOOP4_MAX_SPEED_MASK,
    (STARTUP_TUNE_MAX_SPEED << CLOSED_LOOP4_MAX_SPEED_SHIFT)
  );

  if (ok) {
    ESP_LOGI(TUNING_TAG, "Startup tune profile applied; pulsing CLR_FLT");
  } else {
    ESP_LOGW(TUNING_TAG, "Startup tune profile partially applied; pulsing CLR_FLT");
  }
  if (!STARTUP_TUNE_LOCK2_EN) {
    ESP_LOGW(
      TUNING_TAG, "Startup tune disables ABN_BEMF lock (LOCK2_EN=0) for manual bring-up stability"
    );
  }
  this->parent_->pulse_clear_faults();
  return ok;
}

bool MCF8316DTuningController::apply_hw_lock_report_only_profile() {
  ESP_LOGW(TUNING_TAG, "Applying locks-disabled + short-align debug profile");
  ESP_LOGW(
    TUNING_TAG, "WARNING: LOCK/HW_LOCK/MTR_LCK protective actions are disabled for this debug mode"
  );

  bool ok = true;
  if (!this->parent_->set_speed_percent(0.0f)) {
    ESP_LOGW(TUNING_TAG, "Failed to set speed to 0%% before applying HW lock report-only mode");
    ok = false;
  }
  if (!this->parent_->set_direction_mode("cw")) {
    ESP_LOGW(TUNING_TAG, "Failed to force direction to cw for locks-disabled debug mode");
    ok = false;
  } else if (this->parent_->direction_select_ != nullptr) {
    this->parent_->direction_select_->publish_state("cw");
  }
  if (!this->parent_->set_brake_override(false)) {
    ESP_LOGW(TUNING_TAG, "Failed to force brake OFF for locks-disabled debug mode");
    ok = false;
  } else if (this->parent_->brake_switch_ != nullptr) {
    this->parent_->brake_switch_->publish_state(false);
  }

  uint32_t before = 0;
  if (!this->parent_->read_reg32(RegisterId::FAULT_CONFIG2, before)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG2 read failed");
    return false;
  }

  const uint32_t mask = FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK;
  const uint32_t value = (LOCK_DISABLED_MODE << FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT);
  const uint32_t next = (before & ~mask) | (value & mask);
  if (next != before && !this->parent_->write_reg32(RegisterId::FAULT_CONFIG2, next)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG2 write failed: 0x%08X -> 0x%08X", before, next);
    return false;
  }

  uint32_t after = 0;
  if (!this->parent_->read_reg32(RegisterId::FAULT_CONFIG2, after)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG2 verify read failed");
    return false;
  }

  ESP_LOGI(TUNING_TAG, "FAULT_CONFIG2 HW_LOCK_ILIMIT_MODE: 0x%08X -> 0x%08X", before, after);
  const bool mode_ok = (after & mask) == (value & mask);
  if (!mode_ok) {
    ESP_LOGW(
      TUNING_TAG,
      "FAULT_CONFIG2 verify mismatch: expected mask=0x%08X actual mask=0x%08X",
      (value & mask),
      (after & mask)
    );
  }
  ok &= mode_ok;

  uint32_t fc1_before = 0;
  if (!this->parent_->read_reg32(RegisterId::FAULT_CONFIG1, fc1_before)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG1 read failed");
    return false;
  }

  const uint32_t fc1_mask = FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK | FAULT_CONFIG1_MTR_LCK_MODE_MASK;
  const uint32_t fc1_value = (LOCK_DISABLED_MODE << FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT) |
                             (LOCK_DISABLED_MODE << FAULT_CONFIG1_MTR_LCK_MODE_SHIFT);
  const uint32_t fc1_next = (fc1_before & ~fc1_mask) | (fc1_value & fc1_mask);
  if (fc1_next != fc1_before && !this->parent_->write_reg32(RegisterId::FAULT_CONFIG1, fc1_next)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG1 write failed: 0x%08X -> 0x%08X", fc1_before, fc1_next);
    return false;
  }

  uint32_t fc1_after = 0;
  if (!this->parent_->read_reg32(RegisterId::FAULT_CONFIG1, fc1_after)) {
    ESP_LOGW(TUNING_TAG, "FAULT_CONFIG1 verify read failed");
    return false;
  }

  ESP_LOGI(TUNING_TAG, "FAULT_CONFIG1 LOCK/MTR modes: 0x%08X -> 0x%08X", fc1_before, fc1_after);
  const bool fc1_mode_ok = (fc1_after & fc1_mask) == (fc1_value & fc1_mask);
  if (!fc1_mode_ok) {
    ESP_LOGW(
      TUNING_TAG,
      "FAULT_CONFIG1 verify mismatch: expected mask=0x%08X actual mask=0x%08X",
      (fc1_value & fc1_mask),
      (fc1_after & fc1_mask)
    );
  }
  ok &= fc1_mode_ok;

  uint32_t s1_before = 0;
  if (!this->parent_->read_reg32(RegisterId::MOTOR_STARTUP1, s1_before)) {
    ESP_LOGW(TUNING_TAG, "MOTOR_STARTUP1 read failed");
    return false;
  }
  const uint32_t s1_mask = MOTOR_STARTUP1_MTR_STARTUP_MASK | MOTOR_STARTUP1_ALIGN_TIME_MASK;
  const uint32_t s1_value = (DEBUG_ALIGN_MTR_STARTUP << MOTOR_STARTUP1_MTR_STARTUP_SHIFT) |
                            (DEBUG_ALIGN_TIME << MOTOR_STARTUP1_ALIGN_TIME_SHIFT);
  const uint32_t s1_next = (s1_before & ~s1_mask) | (s1_value & s1_mask);
  if (s1_next != s1_before && !this->parent_->write_reg32(RegisterId::MOTOR_STARTUP1, s1_next)) {
    ESP_LOGW(TUNING_TAG, "MOTOR_STARTUP1 write failed: 0x%08X -> 0x%08X", s1_before, s1_next);
    return false;
  }

  uint32_t s1_after = 0;
  if (!this->parent_->read_reg32(RegisterId::MOTOR_STARTUP1, s1_after)) {
    ESP_LOGW(TUNING_TAG, "MOTOR_STARTUP1 verify read failed");
    return false;
  }
  ESP_LOGI(TUNING_TAG, "MOTOR_STARTUP1 mode/align-time: 0x%08X -> 0x%08X", s1_before, s1_after);
  const bool s1_ok = (s1_after & s1_mask) == (s1_value & s1_mask);
  if (!s1_ok) {
    ESP_LOGW(
      TUNING_TAG,
      "MOTOR_STARTUP1 verify mismatch: expected mask=0x%08X actual mask=0x%08X",
      (s1_value & s1_mask),
      (s1_after & s1_mask)
    );
  }
  ok &= s1_ok;

  if (ok) {
    ESP_LOGI(TUNING_TAG, "Locks-disabled debug profile applied; pulsing CLR_FLT");
  } else {
    ESP_LOGW(TUNING_TAG, "Locks-disabled debug profile partially applied; pulsing CLR_FLT");
  }
  this->parent_->pulse_clear_faults();
  return ok;
}

bool MCF8316DTuningController::start_startup_current_sweep() {
  if (this->scope_probe_test_active_) {
    ESP_LOGW(TUNING_TAG, "Scope probe test active; stopping probe sequence before startup sweep");
    this->scope_probe_test_active_ = false;
    this->scope_probe_stage_pending_ = false;
  }
  if (this->startup_sweep_active_) {
    ESP_LOGW(TUNING_TAG, "Startup current sweep already active; restarting from step 1");
  } else {
    ESP_LOGI(
      TUNING_TAG,
      "Starting startup current sweep (%u steps)",
      static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT)
    );
  }

  if (!this->apply_startup_tune_profile()) {
    ESP_LOGW(TUNING_TAG, "Startup tune baseline was partial; continuing with current sweep");
  }

  this->startup_sweep_active_ = true;
  this->startup_sweep_step_pending_ = false;
  this->startup_sweep_step_index_ = 0u;
  this->startup_sweep_pass_count_ = 0u;
  this->startup_sweep_step_start_ms_ = 0u;
  this->startup_sweep_next_step_due_ms_ = 0u;
  return this->begin_startup_sweep_step_();
}

bool MCF8316DTuningController::start_scope_probe_test() {
  if (this->startup_sweep_active_) {
    ESP_LOGW(TUNING_TAG, "Startup sweep is active; stopping sweep before scope probe test");
    this->startup_sweep_active_ = false;
    this->startup_sweep_step_pending_ = false;
  }
  if (this->scope_probe_test_active_) {
    ESP_LOGW(TUNING_TAG, "Scope probe test already active; restarting from stage 1");
  } else {
    ESP_LOGI(
      TUNING_TAG,
      "Starting scope probe test (%u stages)",
      static_cast<unsigned>(SCOPE_PROBE_STAGE_COUNT)
    );
  }

  if (!this->apply_startup_tune_profile()) {
    ESP_LOGW(TUNING_TAG, "Scope probe baseline startup tune was partial; continuing");
  }

  this->scope_probe_test_active_ = true;
  this->scope_probe_stage_pending_ = false;
  this->scope_probe_stage_index_ = 0u;
  this->scope_probe_stage_start_ms_ = 0u;
  this->scope_probe_next_stage_due_ms_ = 0u;
  return this->begin_scope_probe_stage_();
}

bool MCF8316DTuningController::needs_algorithm_state() const {
  return this->startup_sweep_active_ || this->scope_probe_test_active_;
}

void MCF8316DTuningController::update(
  bool algorithm_state_valid,
  uint16_t algorithm_state,
  bool fault_active,
  bool fault_state_valid,
  bool controller_valid,
  uint32_t controller_fault_status,
  uint16_t volt_mag_raw
) {
  this->process_startup_sweep_(
    algorithm_state_valid,
    algorithm_state,
    fault_active,
    fault_state_valid,
    controller_valid,
    controller_fault_status,
    volt_mag_raw
  );
  this->process_scope_probe_test_(
    algorithm_state_valid,
    algorithm_state,
    fault_active,
    fault_state_valid,
    controller_valid,
    controller_fault_status,
    volt_mag_raw
  );

  const bool mpet_fault_active =
    controller_valid && ((controller_fault_status & (FAULT_MPET_IPD | FAULT_MPET_BEMF)) != 0);
  if (mpet_fault_active &&
      ((this->last_mpet_diag_log_ms_ == 0U) || (millis() - this->last_mpet_diag_log_ms_ >= 2000U))) {
    this->log_mpet_diagnostics_("loop_mpet_fault");
    this->last_mpet_diag_log_ms_ = millis();
  }
  if (!mpet_fault_active) {
    this->last_mpet_diag_log_ms_ = 0u;
  }
}

void MCF8316DTuningController::apply_post_comms_setup() {
  uint32_t ctrl_fault = 0;

  if (!this->ensure_buck_current_limit_for_manual_()) {
    ESP_LOGW(TUNING_TAG, "Failed to ensure buck current limit for manual validation");
  }

  if (!this->seed_closed_loop_params_if_zero_()) {
    ESP_LOGW(TUNING_TAG, "Failed to seed one or more zero CLOSED_LOOP motor parameters");
  }

  // Force manual mode by disabling any MPET command/config bits carried over at boot.
  if (!this->parent_->update_bits32(RegisterId::ALGO_DEBUG2, ALGO_DEBUG2_MPET_ALL_MASK, 0)) {
    ESP_LOGW(TUNING_TAG, "Failed to disable MPET control bits in ALGO_DEBUG2");
  } else {
    uint32_t algo_debug2 = 0;
    if (this->parent_->read_reg32(RegisterId::ALGO_DEBUG2, algo_debug2)) {
      ESP_LOGI(TUNING_TAG, "ALGO_DEBUG2 after MPET disable: 0x%08X", algo_debug2);
    }
  }

  if (this->parent_->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, ctrl_fault) &&
      ((ctrl_fault & (FAULT_MPET_IPD | FAULT_MPET_BEMF)) != 0)) {
    ESP_LOGW(TUNING_TAG, "MPET fault latched at startup (0x%08X), attempting clear", ctrl_fault);
    this->parent_->pulse_clear_faults();
  }

  this->log_mpet_diagnostics_("setup");
}

bool MCF8316DTuningController::ensure_buck_current_limit_for_manual_() {
  uint32_t gd_config2 = 0;
  if (!this->parent_->read_reg32(RegisterId::GD_CONFIG2, gd_config2)) {
    ESP_LOGW(TUNING_TAG, "Failed to read GD_CONFIG2 for BUCK_CL check");
    return false;
  }

  const bool buck_cl_150ma = (gd_config2 & GD_CONFIG2_BUCK_CL_MASK) != 0;
  if (!buck_cl_150ma) {
    ESP_LOGI(TUNING_TAG, "GD_CONFIG2 BUCK_CL already 600mA (gd2=0x%08X)", gd_config2);
    return true;
  }

  ESP_LOGW(
    TUNING_TAG,
    "GD_CONFIG2 has BUCK_CL=150mA (gd2=0x%08X); setting to 600mA for manual validation",
    gd_config2
  );
  if (!this->parent_->update_bits32(RegisterId::GD_CONFIG2, GD_CONFIG2_BUCK_CL_MASK, 0)) {
    ESP_LOGW(TUNING_TAG, "Failed to write GD_CONFIG2 BUCK_CL to 600mA");
    return false;
  }

  uint32_t gd_verify = 0;
  if (!this->parent_->read_reg32(RegisterId::GD_CONFIG2, gd_verify)) {
    ESP_LOGW(TUNING_TAG, "Failed to verify GD_CONFIG2 after BUCK_CL update");
    return false;
  }
  ESP_LOGI(
    TUNING_TAG,
    "GD_CONFIG2 after BUCK_CL update: 0x%08X (BUCK_CL=%s)",
    gd_verify,
    ((gd_verify & GD_CONFIG2_BUCK_CL_MASK) != 0) ? "150mA" : "600mA"
  );
  return true;
}

bool MCF8316DTuningController::seed_closed_loop_params_if_zero_() {
  uint32_t closed_loop2 = 0;
  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;
  if (!this->parent_->read_reg32(RegisterId::CLOSED_LOOP2, closed_loop2) ||
      !this->parent_->read_reg32(RegisterId::CLOSED_LOOP3, closed_loop3) ||
      !this->parent_->read_reg32(RegisterId::CLOSED_LOOP4, closed_loop4)) {
    ESP_LOGW(TUNING_TAG, "Failed to read CLOSED_LOOP2/3/4 for MPET seed check");
    return false;
  }

  const uint32_t motor_res = static_cast<uint32_t>(
    (closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT
  );
  const uint32_t motor_ind = static_cast<uint32_t>(
    (closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT
  );
  const uint32_t motor_bemf = static_cast<uint32_t>(
    (closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT
  );
  const uint32_t spd_loop_kp_msb = static_cast<uint32_t>(
    (closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT
  );
  const uint32_t spd_loop_kp_lsb = static_cast<uint32_t>(
    (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT
  );
  const uint32_t spd_loop_kp = (spd_loop_kp_msb << 7) | spd_loop_kp_lsb;
  const uint32_t spd_loop_ki = static_cast<uint32_t>(
    (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT
  );

  uint32_t closed_loop2_next = closed_loop2;
  uint32_t closed_loop3_next = closed_loop3;
  uint32_t closed_loop4_next = closed_loop4;
  bool needs_seed = false;

  if (motor_res == 0U) {
    closed_loop2_next =
      (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_RES_MASK) |
      ((CLOSED_LOOP_SEED_MOTOR_RES << CLOSED_LOOP2_MOTOR_RES_SHIFT) & CLOSED_LOOP2_MOTOR_RES_MASK);
    needs_seed = true;
  }
  if (motor_ind == 0U) {
    closed_loop2_next =
      (closed_loop2_next & ~CLOSED_LOOP2_MOTOR_IND_MASK) |
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
    closed_loop4_next =
      (closed_loop4_next & ~CLOSED_LOOP4_SPD_LOOP_KI_MASK) |
      ((CLOSED_LOOP_SEED_SPD_KI << CLOSED_LOOP4_SPD_LOOP_KI_SHIFT) & CLOSED_LOOP4_SPD_LOOP_KI_MASK);
    needs_seed = true;
  }

  if (!needs_seed) {
    ESP_LOGI(TUNING_TAG, "CLOSED_LOOP params already non-zero; not seeding manual startup defaults");
    return true;
  }

  ESP_LOGW(
    TUNING_TAG,
    "Seeding zero CLOSED_LOOP params to avoid forced MPET: cl2 0x%08X->0x%08X cl3 0x%08X->0x%08X "
    "cl4 0x%08X->0x%08X",
    closed_loop2,
    closed_loop2_next,
    closed_loop3,
    closed_loop3_next,
    closed_loop4,
    closed_loop4_next
  );
  ESP_LOGW(
    TUNING_TAG,
    "Seed codes: MOTOR_RES=0x%02X MOTOR_IND=0x%02X MOTOR_BEMF=0x%02X SPD_KP=%u SPD_KI=%u",
    static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_RES),
    static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_IND),
    static_cast<unsigned>(CLOSED_LOOP_SEED_MOTOR_BEMF),
    static_cast<unsigned>(CLOSED_LOOP_SEED_SPD_KP),
    static_cast<unsigned>(CLOSED_LOOP_SEED_SPD_KI)
  );

  bool ok = true;
  if (closed_loop2_next != closed_loop2) {
    ok &= this->parent_->write_reg32(RegisterId::CLOSED_LOOP2, closed_loop2_next);
  }
  if (closed_loop3_next != closed_loop3) {
    ok &= this->parent_->write_reg32(RegisterId::CLOSED_LOOP3, closed_loop3_next);
  }
  if (closed_loop4_next != closed_loop4) {
    ok &= this->parent_->write_reg32(RegisterId::CLOSED_LOOP4, closed_loop4_next);
  }
  if (!ok) {
    ESP_LOGW(TUNING_TAG, "Failed writing one or more seeded CLOSED_LOOP registers");
    return false;
  }

  uint32_t verify2 = 0;
  uint32_t verify3 = 0;
  uint32_t verify4 = 0;
  if (this->parent_->read_reg32(RegisterId::CLOSED_LOOP2, verify2) &&
      this->parent_->read_reg32(RegisterId::CLOSED_LOOP3, verify3) &&
      this->parent_->read_reg32(RegisterId::CLOSED_LOOP4, verify4)) {
    ESP_LOGI(
      TUNING_TAG,
      "CLOSED_LOOP after seed: cl2=0x%08X cl3=0x%08X cl4=0x%08X",
      verify2,
      verify3,
      verify4
    );
  }

  return true;
}

void MCF8316DTuningController::log_mpet_diagnostics_(const char *context) {
  uint32_t ctrl_fault = 0;
  uint32_t algo_debug2 = 0;
  uint32_t algo_status_mpet = 0;
  uint32_t mtr_params = 0;
  uint16_t algorithm_state = 0;

  const bool ctrl_ok = this->parent_->read_reg32(RegisterId::CONTROLLER_FAULT_STATUS, ctrl_fault);
  const bool dbg2_ok = this->parent_->read_reg32(RegisterId::ALGO_DEBUG2, algo_debug2);
  const bool mpet_ok = this->parent_->read_reg32(RegisterId::ALGO_STATUS_MPET, algo_status_mpet);
  const bool mtr_ok = this->parent_->read_reg32(RegisterId::MTR_PARAMS, mtr_params);
  const bool state_ok = this->parent_->read_reg16(RegisterId::ALGORITHM_STATE, algorithm_state);

  ESP_LOGI(
    TUNING_TAG,
    "[%s] MPET diag: state=0x%04X(%s) ctrl=0x%08X dbg2=0x%08X mpet=0x%08X mtr=0x%08X",
    context,
    static_cast<unsigned>(algorithm_state),
    this->parent_->algorithm_state_to_string_(algorithm_state),
    ctrl_fault,
    algo_debug2,
    algo_status_mpet,
    mtr_params
  );

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
    const uint32_t mpet_pwm_freq = static_cast<uint32_t>(
      (algo_status_mpet & ALGO_STATUS_MPET_PWM_FREQ_MASK) >> ALGO_STATUS_MPET_PWM_FREQ_SHIFT
    );
    const uint32_t motor_r =
      static_cast<uint32_t>((mtr_params & MTR_PARAMS_R_MASK) >> MTR_PARAMS_R_SHIFT);
    const uint32_t motor_l =
      static_cast<uint32_t>((mtr_params & MTR_PARAMS_L_MASK) >> MTR_PARAMS_L_SHIFT);
    const uint32_t motor_ke =
      static_cast<uint32_t>((mtr_params & MTR_PARAMS_KE_MASK) >> MTR_PARAMS_KE_SHIFT);

    ESP_LOGI(
      TUNING_TAG,
      "[%s] MPET fields: cmd=%s r=%s l=%s ke=%s mech=%s wr_shadow=%s done[r=%s l=%s ke=%s mech=%s] "
      "pwm=%u params[R=%u L=%u Ke=%u]",
      context,
      YESNO(mpet_cmd),
      YESNO(mpet_r),
      YESNO(mpet_l),
      YESNO(mpet_ke),
      YESNO(mpet_mech),
      YESNO(mpet_write_shadow),
      YESNO(mpet_r_done),
      YESNO(mpet_l_done),
      YESNO(mpet_ke_done),
      YESNO(mpet_mech_done),
      static_cast<unsigned>(mpet_pwm_freq),
      static_cast<unsigned>(motor_r),
      static_cast<unsigned>(motor_l),
      static_cast<unsigned>(motor_ke)
    );

    this->log_mpet_entry_conditions(context, algo_debug2);
  }

  if (!(ctrl_ok && dbg2_ok && mpet_ok && mtr_ok && state_ok)) {
    ESP_LOGW(
      TUNING_TAG,
      "[%s] MPET diag read warning: ctrl=%s dbg2=%s mpet=%s mtr=%s state=%s",
      context,
      YESNO(ctrl_ok),
      YESNO(dbg2_ok),
      YESNO(mpet_ok),
      YESNO(mtr_ok),
      YESNO(state_ok)
    );
  }
}

void MCF8316DTuningController::log_mpet_entry_conditions(const char *context, uint32_t algo_debug2) {
  uint32_t closed_loop2 = 0;
  uint32_t closed_loop3 = 0;
  uint32_t closed_loop4 = 0;

  const bool cl2_ok = this->parent_->read_reg32(RegisterId::CLOSED_LOOP2, closed_loop2);
  const bool cl3_ok = this->parent_->read_reg32(RegisterId::CLOSED_LOOP3, closed_loop3);
  const bool cl4_ok = this->parent_->read_reg32(RegisterId::CLOSED_LOOP4, closed_loop4);

  if (cl2_ok && cl3_ok && cl4_ok) {
    const uint32_t motor_res = static_cast<uint32_t>(
      (closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT
    );
    const uint32_t motor_ind = static_cast<uint32_t>(
      (closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT
    );
    const uint32_t motor_bemf = static_cast<uint32_t>(
      (closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >> CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT
    );
    const uint32_t curr_loop_kp = static_cast<uint32_t>(
      (closed_loop3 & CLOSED_LOOP3_CURR_LOOP_KP_MASK) >> CLOSED_LOOP3_CURR_LOOP_KP_SHIFT
    );
    const uint32_t curr_loop_ki = static_cast<uint32_t>(
      (closed_loop3 & CLOSED_LOOP3_CURR_LOOP_KI_MASK) >> CLOSED_LOOP3_CURR_LOOP_KI_SHIFT
    );
    const uint32_t spd_loop_kp_msb = static_cast<uint32_t>(
      (closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >> CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT
    );
    const uint32_t spd_loop_kp_lsb = static_cast<uint32_t>(
      (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >> CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT
    );
    const uint32_t spd_loop_kp = (spd_loop_kp_msb << 7) | spd_loop_kp_lsb;
    const uint32_t spd_loop_ki = static_cast<uint32_t>(
      (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >> CLOSED_LOOP4_SPD_LOOP_KI_SHIFT
    );
    const uint32_t max_speed = static_cast<uint32_t>(
      (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >> CLOSED_LOOP4_MAX_SPEED_SHIFT
    );

    const bool mpet_cmd = (algo_debug2 & ALGO_DEBUG2_MPET_CMD_MASK) != 0;
    const bool mpet_r = (algo_debug2 & ALGO_DEBUG2_MPET_R_MASK) != 0;
    const bool mpet_l = (algo_debug2 & ALGO_DEBUG2_MPET_L_MASK) != 0;
    const bool mpet_ke = (algo_debug2 & ALGO_DEBUG2_MPET_KE_MASK) != 0;
    const bool mpet_mech = (algo_debug2 & ALGO_DEBUG2_MPET_MECH_MASK) != 0;

    const bool rl_forced_by_zero = (motor_res == 0U) || (motor_ind == 0U);
    const bool ke_forced_by_zero = (motor_bemf == 0U);
    const bool mech_forced_by_zero = (spd_loop_kp == 0U) || (spd_loop_ki == 0U);
    const bool mpet_on_nonzero_speed =
      mpet_r || mpet_l || mpet_ke || mpet_mech || rl_forced_by_zero || ke_forced_by_zero ||
      mech_forced_by_zero;

    ESP_LOGI(
      TUNING_TAG,
      "[%s] MPET cfg: cl2=0x%08X cl3=0x%08X cl4=0x%08X motor_res=0x%02X motor_ind=0x%02X motor_bemf=0x%02X "
      "curr_kp=%u curr_ki=%u spd_kp=%u spd_ki=%u max_speed=%u",
      context,
      closed_loop2,
      closed_loop3,
      closed_loop4,
      static_cast<unsigned>(motor_res),
      static_cast<unsigned>(motor_ind),
      static_cast<unsigned>(motor_bemf),
      static_cast<unsigned>(curr_loop_kp),
      static_cast<unsigned>(curr_loop_ki),
      static_cast<unsigned>(spd_loop_kp),
      static_cast<unsigned>(spd_loop_ki),
      static_cast<unsigned>(max_speed)
    );
    ESP_LOGI(
      TUNING_TAG,
      "[%s] MPET triggers: cmd=%s bits[r=%s l=%s ke=%s mech=%s] zero_forced[rl=%s ke=%s mech=%s] enter_on_speed=%s",
      context,
      YESNO(mpet_cmd),
      YESNO(mpet_r),
      YESNO(mpet_l),
      YESNO(mpet_ke),
      YESNO(mpet_mech),
      YESNO(rl_forced_by_zero),
      YESNO(ke_forced_by_zero),
      YESNO(mech_forced_by_zero),
      YESNO(mpet_on_nonzero_speed)
    );
  } else {
    ESP_LOGW(
      TUNING_TAG,
      "[%s] MPET cfg read warning: cl2=%s cl3=%s cl4=%s",
      context,
      YESNO(cl2_ok),
      YESNO(cl3_ok),
      YESNO(cl4_ok)
    );
  }
}

float MCF8316DTuningController::scope_probe_stage_speed_percent_(uint8_t stage_index) const {
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

uint32_t MCF8316DTuningController::scope_probe_stage_hold_ms_(uint8_t stage_index) const {
  (void) stage_index;
  return 7000u;
}

bool MCF8316DTuningController::begin_scope_probe_stage_() {
  if (!this->scope_probe_test_active_) {
    return false;
  }

  if (this->scope_probe_stage_index_ >= SCOPE_PROBE_STAGE_COUNT) {
    ESP_LOGI(TUNING_TAG, "Scope probe test finished");
    this->scope_probe_test_active_ = false;
    this->scope_probe_stage_pending_ = false;
    (void) this->parent_->set_speed_percent(0.0f);
    return true;
  }

  const float speed_percent = this->scope_probe_stage_speed_percent_(this->scope_probe_stage_index_);
  const uint32_t hold_ms = this->scope_probe_stage_hold_ms_(this->scope_probe_stage_index_);

  if (!this->parent_->set_speed_percent(0.0f)) {
    ESP_LOGW(
      TUNING_TAG,
      "Scope probe stage %u failed to set speed to 0%% before configure",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u)
    );
  }
  if (!this->parent_->set_direction_mode("cw")) {
    ESP_LOGW(
      TUNING_TAG,
      "Scope probe stage %u failed to force direction cw",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u)
    );
  }
  if (!this->parent_->set_brake_override(false)) {
    ESP_LOGW(
      TUNING_TAG,
      "Scope probe stage %u failed to force brake OFF",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u)
    );
  }
  this->parent_->pulse_clear_faults();
  if (!this->parent_->set_speed_percent(speed_percent)) {
    ESP_LOGW(
      TUNING_TAG,
      "Scope probe stage %u failed to set speed to %.1f%%",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u),
      speed_percent
    );
    this->scope_probe_test_active_ = false;
    return false;
  }

  this->scope_probe_stage_start_ms_ = millis();
  ESP_LOGI(
    TUNING_TAG,
    "[scope_probe] Stage %u/%u start: speed=%.1f%% hold=%ums",
    static_cast<unsigned>(this->scope_probe_stage_index_ + 1u),
    static_cast<unsigned>(SCOPE_PROBE_STAGE_COUNT),
    speed_percent,
    static_cast<unsigned>(hold_ms)
  );
  return true;
}

void MCF8316DTuningController::process_scope_probe_test_(
  bool algorithm_state_valid,
  uint16_t algorithm_state,
  bool fault_active,
  bool fault_state_valid,
  bool controller_valid,
  uint32_t controller_fault_status,
  uint16_t volt_mag_raw
) {
  if (!this->scope_probe_test_active_) {
    return;
  }

  const uint32_t now = millis();
  if (this->scope_probe_stage_pending_) {
    if (now < this->scope_probe_next_stage_due_ms_) {
      return;
    }
    if (fault_state_valid && fault_active) {
      ESP_LOGI(
        TUNING_TAG,
        "[scope_probe] waiting for fault clear before stage %u",
        static_cast<unsigned>(this->scope_probe_stage_index_ + 1u)
      );
      this->parent_->pulse_clear_faults();
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
    ESP_LOGW(
      TUNING_TAG,
      "[scope_probe] Stage %u FAULT: speed=%.1f%% elapsed=%ums ctrl_fault=0x%08X ctrl_valid=%s "
      "state=%s volt_mag=%.1f%%",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u),
      speed_percent,
      static_cast<unsigned>(elapsed_ms),
      ctrl_fault_word,
      YESNO(controller_valid),
      algorithm_state_valid ? this->parent_->algorithm_state_to_string_(algorithm_state) : "UNKNOWN",
      volt_mag_percent
    );
    (void) this->parent_->set_speed_percent(0.0f);
    this->scope_probe_stage_index_++;
    this->scope_probe_stage_pending_ = true;
    this->scope_probe_next_stage_due_ms_ = now + SCOPE_PROBE_INTER_STAGE_DELAY_MS;
    return;
  }

  if (elapsed_ms >= hold_ms) {
    ESP_LOGI(
      TUNING_TAG,
      "[scope_probe] Stage %u complete: speed=%.1f%% elapsed=%ums state=%s volt_mag=%.1f%%",
      static_cast<unsigned>(this->scope_probe_stage_index_ + 1u),
      speed_percent,
      static_cast<unsigned>(elapsed_ms),
      algorithm_state_valid ? this->parent_->algorithm_state_to_string_(algorithm_state) : "UNKNOWN",
      volt_mag_percent
    );
    (void) this->parent_->set_speed_percent(0.0f);
    this->scope_probe_stage_index_++;
    this->scope_probe_stage_pending_ = true;
    this->scope_probe_next_stage_due_ms_ = now + SCOPE_PROBE_INTER_STAGE_DELAY_MS;
  }
}

uint32_t MCF8316DTuningController::startup_sweep_current_code_(uint8_t step_index) const {
  switch (step_index) {
    case 0:
      return 3u;
    case 1:
      return 4u;
    case 2:
      return 5u;
    case 3:
    default:
      return 6u;
  }
}

float MCF8316DTuningController::current_limit_code_to_amps_(uint32_t current_limit_code) const {
  static const float kCurrentThresholdA[16] = {
    0.125f, 0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f,
    3.5f,   4.0f,  4.5f, 5.0f, 5.5f, 6.0f, 7.0f, 8.0f,
  };
  return kCurrentThresholdA[current_limit_code & 0xFu];
}

bool MCF8316DTuningController::apply_startup_sweep_current_limits_(uint32_t current_limit_code) {
  const uint32_t s1_value =
    (current_limit_code << MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT);
  const uint32_t s2_value = (current_limit_code << MOTOR_STARTUP2_OL_ILIMIT_SHIFT);
  if (!this->parent_->update_bits32(
        register_address(RegisterId::MOTOR_STARTUP1), MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK, s1_value
      )) {
    ESP_LOGW(TUNING_TAG, "Startup sweep MOTOR_STARTUP1 write failed");
    return false;
  }
  if (!this->parent_->update_bits32(RegisterId::MOTOR_STARTUP2, MOTOR_STARTUP2_OL_ILIMIT_MASK, s2_value)) {
    ESP_LOGW(TUNING_TAG, "Startup sweep MOTOR_STARTUP2 write failed");
    return false;
  }

  uint32_t startup1 = 0;
  uint32_t startup2 = 0;
  const bool startup1_ok = this->parent_->read_reg32(RegisterId::MOTOR_STARTUP1, startup1);
  const bool startup2_ok = this->parent_->read_reg32(RegisterId::MOTOR_STARTUP2, startup2);
  if (startup1_ok && startup2_ok) {
    const uint32_t align_ilimit = (startup1 & MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK) >>
                                  MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT;
    const uint32_t ol_ilimit =
      (startup2 & MOTOR_STARTUP2_OL_ILIMIT_MASK) >> MOTOR_STARTUP2_OL_ILIMIT_SHIFT;
    ESP_LOGI(
      TUNING_TAG,
      "Startup sweep current limits: startup1=0x%08X startup2=0x%08X align_ilimit=%u(%.3gA) "
      "ol_ilimit=%u(%.3gA)",
      startup1,
      startup2,
      static_cast<unsigned>(align_ilimit),
      this->current_limit_code_to_amps_(align_ilimit),
      static_cast<unsigned>(ol_ilimit),
      this->current_limit_code_to_amps_(ol_ilimit)
    );
  }
  return true;
}

bool MCF8316DTuningController::begin_startup_sweep_step_() {
  if (!this->startup_sweep_active_) {
    return false;
  }

  if (this->startup_sweep_step_index_ >= STARTUP_SWEEP_STEP_COUNT) {
    ESP_LOGI(
      TUNING_TAG,
      "Startup current sweep finished: %u/%u steps reached open/closed loop",
      static_cast<unsigned>(this->startup_sweep_pass_count_),
      static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT)
    );
    this->startup_sweep_active_ = false;
    this->startup_sweep_step_pending_ = false;
    (void) this->parent_->set_speed_percent(0.0f);
    return true;
  }

  const uint32_t current_limit_code = this->startup_sweep_current_code_(this->startup_sweep_step_index_);
  const float current_limit_a = this->current_limit_code_to_amps_(current_limit_code);

  if (!this->parent_->set_speed_percent(0.0f)) {
    ESP_LOGW(
      TUNING_TAG,
      "Startup sweep step %u failed to set speed to 0%% before configuring",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u)
    );
  }
  if (!this->parent_->set_direction_mode("cw")) {
    ESP_LOGW(
      TUNING_TAG,
      "Startup sweep step %u failed to force direction cw",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u)
    );
  }
  if (!this->parent_->set_brake_override(false)) {
    ESP_LOGW(
      TUNING_TAG,
      "Startup sweep step %u failed to force brake OFF",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u)
    );
  }

  if (!this->apply_startup_sweep_current_limits_(current_limit_code)) {
    ESP_LOGW(
      TUNING_TAG,
      "Startup sweep step %u failed to apply current limits",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u)
    );
    this->startup_sweep_active_ = false;
    return false;
  }

  this->parent_->pulse_clear_faults();
  if (!this->parent_->set_speed_percent(STARTUP_SWEEP_SPEED_PERCENT)) {
    ESP_LOGW(
      TUNING_TAG,
      "Startup sweep step %u failed to set speed to %.1f%%",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u),
      STARTUP_SWEEP_SPEED_PERCENT
    );
    this->startup_sweep_active_ = false;
    return false;
  }
  this->startup_sweep_step_start_ms_ = millis();

  ESP_LOGI(
    TUNING_TAG,
    "[startup_sweep] Step %u/%u start: current_limit=%u (%.3gA), speed=%.1f%%",
    static_cast<unsigned>(this->startup_sweep_step_index_ + 1u),
    static_cast<unsigned>(STARTUP_SWEEP_STEP_COUNT),
    static_cast<unsigned>(current_limit_code),
    current_limit_a,
    STARTUP_SWEEP_SPEED_PERCENT
  );
  return true;
}

void MCF8316DTuningController::schedule_startup_sweep_step_(uint32_t delay_ms) {
  (void) this->parent_->set_speed_percent(0.0f);
  this->startup_sweep_step_pending_ = true;
  this->startup_sweep_next_step_due_ms_ = millis() + delay_ms;
}

void MCF8316DTuningController::process_startup_sweep_(
  bool algorithm_state_valid,
  uint16_t algorithm_state,
  bool fault_active,
  bool fault_state_valid,
  bool controller_valid,
  uint32_t controller_fault_status,
  uint16_t volt_mag_raw
) {
  if (!this->startup_sweep_active_) {
    return;
  }

  const uint32_t now = millis();
  if (this->startup_sweep_step_pending_) {
    if (now < this->startup_sweep_next_step_due_ms_) {
      return;
    }
    if (fault_state_valid && fault_active) {
      ESP_LOGI(
        TUNING_TAG,
        "[startup_sweep] waiting for fault clear before step %u",
        static_cast<unsigned>(this->startup_sweep_step_index_ + 1u)
      );
      this->parent_->pulse_clear_faults();
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
    algorithm_state_valid && ((algorithm_state == ALGORITHM_STATE_OPEN_LOOP) ||
                              (algorithm_state == ALGORITHM_STATE_CLOSED_LOOP_UNALIGNED) ||
                              (algorithm_state == ALGORITHM_STATE_CLOSED_LOOP_ALIGNED));

  if (reached_drive_state && !fault_active && (volt_mag_raw > 0u)) {
    ESP_LOGI(
      TUNING_TAG,
      "[startup_sweep] Step %u PASS: current_limit=%u (%.3gA), elapsed=%ums, state=%s, "
      "volt_mag=%.1f%%",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u),
      static_cast<unsigned>(current_limit_code),
      current_limit_a,
      static_cast<unsigned>(elapsed_ms),
      this->parent_->algorithm_state_to_string_(algorithm_state),
      volt_mag_percent
    );
    this->startup_sweep_pass_count_++;
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
    return;
  }

  if (fault_state_valid && fault_active) {
    const uint32_t ctrl_fault_word = controller_valid ? controller_fault_status : 0u;
    ESP_LOGW(
      TUNING_TAG,
      "[startup_sweep] Step %u FAIL: current_limit=%u (%.3gA), elapsed=%ums, ctrl_fault=0x%08X "
      "ctrl_valid=%s state=%s volt_mag=%.1f%%",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u),
      static_cast<unsigned>(current_limit_code),
      current_limit_a,
      static_cast<unsigned>(elapsed_ms),
      ctrl_fault_word,
      YESNO(controller_valid),
      algorithm_state_valid ? this->parent_->algorithm_state_to_string_(algorithm_state) : "UNKNOWN",
      volt_mag_percent
    );
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
    return;
  }

  if (elapsed_ms >= STARTUP_SWEEP_STEP_TIMEOUT_MS) {
    ESP_LOGW(
      TUNING_TAG,
      "[startup_sweep] Step %u TIMEOUT: current_limit=%u (%.3gA), state=%s, volt_mag=%.1f%%, no "
      "drive-state transition",
      static_cast<unsigned>(this->startup_sweep_step_index_ + 1u),
      static_cast<unsigned>(current_limit_code),
      current_limit_a,
      algorithm_state_valid ? this->parent_->algorithm_state_to_string_(algorithm_state) : "UNKNOWN",
      volt_mag_percent
    );
    this->startup_sweep_step_index_++;
    this->schedule_startup_sweep_step_(STARTUP_SWEEP_INTER_STEP_DELAY_MS);
  }
}

}  // namespace mcf8316d
}  // namespace esphome
