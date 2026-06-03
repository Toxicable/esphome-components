#include "esc_higher.h"

#include "esc_higher_text.h"

#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
namespace {
constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;

template<typename T> void publish_sensor(sensor::Sensor* sensor, T value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}

void publish_text(text_sensor::TextSensor* sensor, const std::string& value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}
}  // namespace

void ESCHigherStartButton::press_action() {
  this->parent_->start_motor();
}

void ESCHigherStopButton::press_action() {
  this->parent_->stop_motor();
}

void ESCHigherClearFaultsButton::press_action() {
  this->parent_->clear_faults();
}

void ESCHigherEstopButton::press_action() {
  this->parent_->estop();
}

void ESCHigherRunBringupTestButton::press_action() {
  this->parent_->run_bringup_test();
}

void ESCHigherRunBridgeStaticVectorTestButton::press_action() {
  this->parent_->run_bridge_static_vector_test();
}

void ESCHigherRunForcedTimerDiffPwmTestButton::press_action() {
  this->parent_->run_forced_timer_diff_pwm_test();
}

void ESCHigherBringupTestSelect::control(const std::string& value) {
  if (this->parent_ == nullptr)
    return;
  if (value == "full_spin_sequence") {
    this->parent_->set_bringup_test_id(101);
  } else if (value == "bridge_static_vector_test") {
    this->parent_->set_bringup_test_id(102);
  } else if (value == "forced_timer_diff_pwm") {
    this->parent_->set_bringup_test_id(103);
  } else {
    ESP_LOGW(TAG, "Unknown bringup test selection: %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void ESCHigherSpeedTargetNumber::control(float value) {
  if (this->parent_ == nullptr)
    return;
  const int32_t target = static_cast<int32_t>(value);
  if (!this->parent_->set_speed_target_dhz_and_send(target)) {
    ESP_LOGW(TAG, "Failed to set speed target dHz=%d", static_cast<int>(target));
    return;
  }
  this->publish_state(static_cast<float>(target));
}

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
  this->initialized_ = false;
  this->next_init_retry_ms_ = 0;
  if (this->bringup_test_select_ != nullptr) {
    this->bringup_test_select_->publish_state(
      this->bringup_test_id_ == BRINGUP_TEST_BRIDGE_STATIC_VECTOR   ? "bridge_static_vector_test" :
      this->bringup_test_id_ == BRINGUP_TEST_FORCED_TIMER_DIFF_PWM ? "forced_timer_diff_pwm" :
                                                                     "full_spin_sequence"
    );
  }
}

bool ESCHigherComponent::read_register_(uint8_t reg, uint8_t* out, size_t len) {
  const i2c::ErrorCode err = this->write_read(&reg, 1, out, len);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Read reg 0x%02X failed (%s)", reg, i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::write_command_(uint8_t opcode, int32_t param0, int32_t param1, int32_t param2) {
  uint8_t tx[17]{0};
  tx[0] = REG_COMMAND;
  tx[1] = this->command_seq_++;
  tx[2] = opcode;
  tx[3] = 0;
  tx[4] = 0;
  tx[5] = static_cast<uint8_t>(param0 & 0xFF);
  tx[6] = static_cast<uint8_t>((param0 >> 8) & 0xFF);
  tx[7] = static_cast<uint8_t>((param0 >> 16) & 0xFF);
  tx[8] = static_cast<uint8_t>((param0 >> 24) & 0xFF);
  tx[9] = static_cast<uint8_t>(param1 & 0xFF);
  tx[10] = static_cast<uint8_t>((param1 >> 8) & 0xFF);
  tx[11] = static_cast<uint8_t>((param1 >> 16) & 0xFF);
  tx[12] = static_cast<uint8_t>((param1 >> 24) & 0xFF);
  tx[13] = static_cast<uint8_t>(param2 & 0xFF);
  tx[14] = static_cast<uint8_t>((param2 >> 8) & 0xFF);
  tx[15] = static_cast<uint8_t>((param2 >> 16) & 0xFF);
  tx[16] = static_cast<uint8_t>((param2 >> 24) & 0xFF);

  ESP_LOGI(TAG, "Cmd %s (seq %u, p0=%d, p1=%d, p2=%d)",
           opcode_to_cstr(opcode), tx[1], param0, param1, param2);

  const i2c::ErrorCode err = this->write(tx, sizeof(tx));
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Cmd %s (seq %u, p0=%d, p1=%d, p2=%d) failed: %s",
           opcode_to_cstr(opcode), tx[1], param0, param1, param2, i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::configure_watchdog_() {
  const uint32_t timeout_ms = this->disable_watchdog_ ? 0 : this->watchdog_timeout_ms_;
  if (this->disable_watchdog_) {
    ESP_LOGI(TAG, "Disabling command watchdog");
  } else {
    ESP_LOGI(TAG, "Setting command watchdog to %u ms", static_cast<unsigned>(timeout_ms));
  }
  if (this->write_command_(OPCODE_SET_WATCHDOG, static_cast<int32_t>(timeout_ms), 0, 0))
    return true;
  ESP_LOGW(TAG, "Failed to configure command watchdog");
  return false;
}

bool ESCHigherComponent::initialize_() {
  uint8_t id[8]{0};
  if (!this->read_register_(REG_ID, id, sizeof(id))) {
    ESP_LOGW(TAG, "Failed to read ID register");
    return false;
  }

  ESP_LOGI(
    TAG,
    "Interface detected proto=%u.%u fw=%u.%u hw=%u max_block_len=%u caps=0x%04X",
    id[0],
    id[1],
    id[2],
    id[3],
    id[4],
    id[5],
    u16_(id, 6)
  );

  return this->configure_watchdog_();
}

bool ESCHigherComponent::start_motor() {
  return this->write_command_(OPCODE_START, 0, 0, 0);
}

bool ESCHigherComponent::stop_motor() {
  return this->write_command_(OPCODE_STOP, 0, 0, 0);
}

bool ESCHigherComponent::clear_faults() {
  return this->write_command_(OPCODE_CLEAR_FAULTS, 0, 0, 0);
}

bool ESCHigherComponent::estop() {
  return this->write_command_(OPCODE_ESTOP, 0, 0, 0);
}

bool ESCHigherComponent::set_speed_ramp() {
  return this->write_command_(OPCODE_SET_SPEED_RAMP, speed_ramp_target_dhz_, speed_ramp_time_ms_, 0);
}

bool ESCHigherComponent::run_bringup_test() {
  const uint8_t test_id = bringup_test_id_;
  int32_t duration_ms = bringup_test_duration_ms_;
  if (test_id == BRINGUP_TEST_BRIDGE_STATIC_VECTOR)
    duration_ms = BRINGUP_TEST_BRIDGE_STATIC_VECTOR_DURATION_MS;
  else if (test_id == BRINGUP_TEST_FORCED_TIMER_DIFF_PWM) {
    if (duration_ms == 5000)
      duration_ms = BRINGUP_TEST_FORCED_TIMER_DIFF_PWM_DURATION_MS;
  } else if (test_id != BRINGUP_TEST_FULL_SPIN_SEQUENCE) {
    ESP_LOGW(TAG, "Unsupported bringup test id: %u", static_cast<unsigned>(test_id));
    return false;
  }
  return this->write_command_(OPCODE_RUN_BRINGUP_TEST, test_id, duration_ms, bringup_test_options_);
}

bool ESCHigherComponent::run_bridge_static_vector_test() {
  return this->write_command_(
    OPCODE_RUN_BRINGUP_TEST,
    BRINGUP_TEST_BRIDGE_STATIC_VECTOR,
    BRINGUP_TEST_BRIDGE_STATIC_VECTOR_DURATION_MS,
    bringup_test_options_
  );
}

bool ESCHigherComponent::run_forced_timer_diff_pwm_test() {
  return this->write_command_(
    OPCODE_RUN_BRINGUP_TEST,
    BRINGUP_TEST_FORCED_TIMER_DIFF_PWM,
    BRINGUP_TEST_FORCED_TIMER_DIFF_PWM_DURATION_MS,
    bringup_test_options_
  );
}

bool ESCHigherComponent::set_speed_target_dhz_and_send(int32_t target_dhz) {
  this->speed_ramp_target_dhz_ = target_dhz;
  return this->set_speed_ramp();
}

void ESCHigherComponent::update() {
  if (!this->initialized_) {
    const uint32_t now = millis();
    if (now < this->next_init_retry_ms_)
      return;
    if (!this->initialize_()) {
      this->status_set_warning();
      this->next_init_retry_ms_ = now + INIT_RETRY_INTERVAL_MS;
      return;
    }
    this->initialized_ = true;
    this->status_clear_warning();
  }

  bool all_ok = true;

  if (
    proto_major_sensor_ != nullptr || proto_minor_sensor_ != nullptr || fw_major_sensor_ != nullptr ||
    fw_minor_sensor_ != nullptr || hw_id_sensor_ != nullptr || max_block_len_sensor_ != nullptr ||
    capabilities_sensor_ != nullptr
  ) {
    uint8_t id[8]{0};
    if (this->read_register_(REG_ID, id, sizeof(id))) {
      publish_sensor(proto_major_sensor_, id[0]);
      publish_sensor(proto_minor_sensor_, id[1]);
      publish_sensor(fw_major_sensor_, id[2]);
      publish_sensor(fw_minor_sensor_, id[3]);
      publish_sensor(hw_id_sensor_, id[4]);
      publish_sensor(max_block_len_sensor_, id[5]);
      publish_sensor(capabilities_sensor_, u16_(id, 6));
      publish_text(capabilities_text_sensor_, bitmask_to_names(u16_(id, 6), CAP_NAMES, 6));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t status[16]{0};
    if (this->read_register_(REG_STATUS, status, sizeof(status))) {
      publish_sensor(status_seq_sensor_, status[0]);
      publish_sensor(esc_state_sensor_, status[1]);
      publish_text(esc_state_text_sensor_, esc_state_to_cstr(status[1]));
      publish_sensor(mc_state_sensor_, status[2]);
      publish_text(mc_state_text_sensor_, mc_state_to_cstr(status[2]));
      publish_sensor(last_cmd_seq_sensor_, status[3]);
      publish_sensor(last_cmd_error_sensor_, status[4]);
      publish_text(last_cmd_error_text_sensor_, last_cmd_error_to_cstr(status[4]));
      publish_sensor(fault_detail_sensor_, status[5]);
      publish_text(fault_detail_text_sensor_, fault_detail_to_cstr(status[5]));
      publish_sensor(current_faults_sensor_, u16_(status, 6));
      publish_text(current_faults_text_sensor_, fault_bitmask_to_names(u16_(status, 6)));
      publish_sensor(occurred_faults_sensor_, u16_(status, 8));
      publish_text(occurred_faults_text_sensor_, fault_bitmask_to_names(u16_(status, 8)));
      publish_sensor(status_flags_sensor_, u16_(status, 10));
      publish_text(status_flags_text_sensor_, bitmask_to_names(u16_(status, 10), STATUS_FLAG_NAMES, 8));
      publish_sensor(watchdog_ms_left_sensor_, u16_(status, 12));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t tel[48]{0};
    if (this->read_register_(REG_TELEMETRY, tel, sizeof(tel))) {
      publish_sensor(telemetry_seq_sensor_, tel[0]);
      publish_sensor(esc_state_sensor_, tel[1]);
      publish_text(esc_state_text_sensor_, esc_state_to_cstr(tel[1]));
      publish_sensor(mc_state_sensor_, tel[2]);
      publish_text(mc_state_text_sensor_, mc_state_to_cstr(tel[2]));
      publish_sensor(last_cmd_error_sensor_, tel[3]);
      publish_text(last_cmd_error_text_sensor_, last_cmd_error_to_cstr(tel[3]));
      publish_sensor(status_flags_sensor_, u16_(tel, 4));
      publish_text(status_flags_text_sensor_, bitmask_to_names(u16_(tel, 4), STATUS_FLAG_NAMES, 8));
      publish_sensor(current_faults_sensor_, u16_(tel, 6));
      publish_text(current_faults_text_sensor_, fault_bitmask_to_names(u16_(tel, 6)));
      publish_sensor(vbus_mv_sensor_, u32_(tel, 8) / 1000.0f);
      publish_sensor(ibus_ma_sensor_, i32_(tel, 12) / 1000.0f);
      publish_sensor(motor_current_ma_sensor_, i32_(tel, 16) / 1000.0f);
      publish_sensor(speed_dhz_sensor_, i32_(tel, 20));
      publish_sensor(duty_centi_pct_sensor_, i16_(tel, 24));
      publish_sensor(last_cmd_seq_sensor_, tel[26]);
      publish_sensor(fault_detail_sensor_, tel[27]);
      publish_text(fault_detail_text_sensor_, fault_detail_to_cstr(tel[27]));
      publish_sensor(temp_mc_sensor_, i32_(tel, 28) / 1000.0f);
      publish_sensor(target_speed_dhz_sensor_, i32_(tel, 32));
      publish_sensor(watchdog_ms_left_sensor_, u16_(tel, 36));
      publish_sensor(drive_limit_centi_pct_sensor_, u16_(tel, 38));
      publish_sensor(uptime_s_sensor_, u32_(tel, 40));
      publish_sensor(telemetry_debug0_sensor_, i16_(tel, 44));
      publish_sensor(telemetry_debug1_sensor_, i16_(tel, 46));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t bringup[48]{0};
    if (this->read_register_(REG_BRINGUP, bringup, sizeof(bringup))) {
      publish_sensor(bringup_seq_sensor_, bringup[0]);
      publish_sensor(bringup_active_sensor_, bringup[1]);
      publish_sensor(bringup_test_id_sensor_, bringup[2]);
      publish_text(bringup_test_id_text_sensor_, bringup_test_id_to_cstr(bringup[2]));
      publish_sensor(bringup_step_id_sensor_, bringup[3]);
      publish_sensor(bringup_state_sensor_, bringup[4]);
      publish_text(bringup_state_text_sensor_, bringup_state_to_cstr(bringup[4]));
      publish_sensor(bringup_result_sensor_, bringup[5]);
      publish_text(bringup_result_text_sensor_, bringup_result_to_cstr(bringup[5]));
      publish_sensor(bringup_failure_code_sensor_, bringup[6]);
      publish_sensor(bringup_measured0_sensor_, i32_(bringup, 8));
      publish_sensor(bringup_measured1_sensor_, i32_(bringup, 12));
      publish_sensor(bringup_limit_min_sensor_, i32_(bringup, 16));
      publish_sensor(bringup_limit_max_sensor_, i32_(bringup, 20));
      publish_sensor(bringup_phase_a_count_sensor_, static_cast<uint16_t>(u32_(bringup, 8) & 0xFFFF));
      publish_sensor(bringup_phase_b_count_sensor_, static_cast<uint16_t>((u32_(bringup, 8) >> 16) & 0xFFFF));
      publish_sensor(bringup_phase_c_count_sensor_, static_cast<uint16_t>(u32_(bringup, 12) & 0xFFFF));
      publish_sensor(bringup_pwm_spread_ticks_sensor_, i32_(bringup, 16));
      publish_sensor(bringup_max_phase_current_ma_sensor_, i32_(bringup, 20));
      publish_sensor(bringup_vbus_mv_at_test_sensor_, u32_(bringup, 24));
      publish_sensor(bringup_current_faults_at_test_sensor_, u16_(bringup, 28));
      publish_text(bringup_current_faults_text_sensor_, fault_bitmask_to_names(u16_(bringup, 28)));
      publish_sensor(bringup_occurred_faults_at_test_sensor_, u16_(bringup, 30));
      publish_text(bringup_occurred_faults_text_sensor_, fault_bitmask_to_names(u16_(bringup, 30)));
      publish_sensor(bringup_mc_state_at_test_sensor_, bringup[32]);
      publish_sensor(bringup_esc_state_at_test_sensor_, bringup[33]);
      publish_sensor(bringup_gd_ready_sensor_, bringup[34]);
      publish_sensor(bringup_elapsed_ms_sensor_, u32_(bringup, 36));
      publish_sensor(bringup_last_passed_step_sensor_, bringup[40]);
      publish_sensor(bringup_steps_total_sensor_, bringup[41]);
      publish_sensor(bringup_attempt_count_sensor_, u16_(bringup, 42));
      publish_sensor(bringup_debug0_sensor_, i16_(bringup, 44));
      publish_sensor(bringup_debug1_sensor_, i16_(bringup, 46));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t debug[32]{0};
    if (this->read_register_(REG_DEBUG_TELEMETRY, debug, sizeof(debug))) {
      publish_sensor(debug_seq_sensor_, debug[0]);
      publish_sensor(debug_v_alpha_raw_s16_sensor_, i16_(debug, 2));
      publish_sensor(debug_v_beta_raw_s16_sensor_, i16_(debug, 4));
      publish_sensor(debug_v_q_raw_s16_sensor_, i16_(debug, 6));
      publish_sensor(debug_v_d_raw_s16_sensor_, i16_(debug, 8));
      publish_sensor(debug_v_u_raw_s16_sensor_, i16_(debug, 10));
      publish_sensor(debug_v_v_raw_s16_sensor_, i16_(debug, 12));
      publish_sensor(debug_v_w_raw_s16_sensor_, i16_(debug, 14));
      publish_sensor(debug_v_amp_raw_s16_sensor_, i16_(debug, 16));
      publish_sensor(debug_phase_ia_ma_sensor_, i16_(debug, 18));
      publish_sensor(debug_phase_ib_ma_sensor_, i16_(debug, 20));
      publish_sensor(debug_phase_ic_ma_sensor_, i16_(debug, 22));
    } else {
      all_ok = false;
    }
  }

  if (all_ok)
    this->status_clear_warning();
  else
    this->status_set_warning();
}

void ESCHigherComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "esc_higher:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  disable_watchdog: %s", this->disable_watchdog_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  watchdog_timeout_ms: %u", static_cast<unsigned>(this->watchdog_timeout_ms_));
  ESP_LOGCONFIG(TAG, "  speed_ramp_target_dhz: %d", static_cast<int>(speed_ramp_target_dhz_));
  ESP_LOGCONFIG(TAG, "  speed_ramp_time_ms: %d", static_cast<int>(speed_ramp_time_ms_));
  ESP_LOGCONFIG(TAG, "  bringup_test_duration_ms: %d", static_cast<int>(bringup_test_duration_ms_));
  ESP_LOGCONFIG(TAG, "  bringup_test_options: 0x%08X", static_cast<unsigned>(bringup_test_options_));
  ESP_LOGCONFIG(TAG, "  bringup_test_id: %u", static_cast<unsigned>(bringup_test_id_));
}

}  // namespace esc_higher
}  // namespace esphome
