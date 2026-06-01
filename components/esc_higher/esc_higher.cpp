#include "esc_higher.h"

#include "esc_higher_text.h"
#include <string>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
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
}

bool ESCHigherComponent::read_register_(uint8_t reg, uint8_t* out, size_t len) {
  const i2c::ErrorCode err = this->write_read(&reg, 1, out, len);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Read reg 0x%02X failed (%s)", reg, i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::write_command_(uint8_t opcode, int32_t param0, int32_t param1, int32_t param2) {
  // Wire format: [0x20][16-byte payload]
  // payload: seq,u8 opcode,u8 flags,u8 reserved,u8 param0 i32 LE, param1 i32 LE, param2 i32 LE
  uint8_t tx[17]{0};
  tx[0] = REG_COMMAND;
  tx[1] = this->command_seq_++;
  tx[2] = opcode;
  tx[3] = 0;  // flags
  tx[4] = 0;  // reserved
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

  ESP_LOGI(TAG, "Cmd %s (seq %d, p0=%d, p1=%d, p2=%d)",
           opcode_to_cstr(opcode), tx[1], param0, param1, param2);

  const i2c::ErrorCode err = this->write(tx, sizeof(tx));
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Cmd %s (seq %d, p0=%d, p1=%d, p2=%d) failed: %s",
             opcode_to_cstr(opcode), tx[1], param0, param1, param2, i2c_error_to_cstr(err));
  return false;
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

bool ESCHigherComponent::set_speed_target_dhz_and_send(int32_t target_dhz) {
  this->speed_ramp_target_dhz_ = target_dhz;
  return this->set_speed_ramp();
}

void ESCHigherComponent::update() {
  bool all_ok = true;

  if (
    proto_major_sensor_ != nullptr || proto_minor_sensor_ != nullptr || fw_major_sensor_ != nullptr ||
    fw_minor_sensor_ != nullptr || hw_id_sensor_ != nullptr || max_block_len_sensor_ != nullptr ||
    capabilities_sensor_ != nullptr
  ) {
    uint8_t id[8]{0};
    if (this->read_register_(REG_ID, id, sizeof(id))) {
      if (proto_major_sensor_ != nullptr)
        proto_major_sensor_->publish_state(id[0]);
      if (proto_minor_sensor_ != nullptr)
        proto_minor_sensor_->publish_state(id[1]);
      if (fw_major_sensor_ != nullptr)
        fw_major_sensor_->publish_state(id[2]);
      if (fw_minor_sensor_ != nullptr)
        fw_minor_sensor_->publish_state(id[3]);
      if (hw_id_sensor_ != nullptr)
        hw_id_sensor_->publish_state(id[4]);
      if (max_block_len_sensor_ != nullptr)
        max_block_len_sensor_->publish_state(id[5]);
      if (capabilities_sensor_ != nullptr)
        capabilities_sensor_->publish_state(u16_(id, 6));
      if (capabilities_text_sensor_ != nullptr)
        capabilities_text_sensor_->publish_state(bitmask_to_names(u16_(id, 6), CAP_NAMES, 6));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t status[16]{0};
    if (this->read_register_(REG_STATUS, status, sizeof(status))) {
      // Field offsets follow the documented STATUS layout in i2c_interface.md.
      if (seq_sensor_ != nullptr)
        seq_sensor_->publish_state(status[0]);
      if (esc_state_sensor_ != nullptr)
        esc_state_sensor_->publish_state(status[1]);
      if (esc_state_text_sensor_ != nullptr)
        esc_state_text_sensor_->publish_state(esc_state_to_cstr(status[1]));
      if (mc_state_sensor_ != nullptr)
        mc_state_sensor_->publish_state(status[2]);
      if (mc_state_text_sensor_ != nullptr)
        mc_state_text_sensor_->publish_state(mc_state_to_cstr(status[2]));
      if (last_cmd_seq_sensor_ != nullptr)
        last_cmd_seq_sensor_->publish_state(status[3]);
      if (last_cmd_error_sensor_ != nullptr)
        last_cmd_error_sensor_->publish_state(status[4]);
      if (last_cmd_error_text_sensor_ != nullptr)
        last_cmd_error_text_sensor_->publish_state(last_cmd_error_to_cstr(status[4]));
      if (fault_detail_sensor_ != nullptr)
        fault_detail_sensor_->publish_state(status[5]);
      if (fault_detail_text_sensor_ != nullptr)
        fault_detail_text_sensor_->publish_state(fault_detail_to_cstr(status[5]));
      if (current_faults_sensor_ != nullptr)
        current_faults_sensor_->publish_state(u16_(status, 6));
      if (current_faults_text_sensor_ != nullptr)
        current_faults_text_sensor_->publish_state(
          bitmask_to_names(u16_(status, 6), FAULT_NAMES, 16)
        );
      if (occurred_faults_sensor_ != nullptr)
        occurred_faults_sensor_->publish_state(u16_(status, 8));
      if (occurred_faults_text_sensor_ != nullptr)
        occurred_faults_text_sensor_->publish_state(
          bitmask_to_names(u16_(status, 8), FAULT_NAMES, 16)
        );
      if (status_flags_sensor_ != nullptr)
        status_flags_sensor_->publish_state(u16_(status, 10));
      if (status_flags_text_sensor_ != nullptr)
        status_flags_text_sensor_->publish_state(
          bitmask_to_names(u16_(status, 10), STATUS_FLAG_NAMES, 8)
        );
      if (watchdog_ms_left_sensor_ != nullptr)
        watchdog_ms_left_sensor_->publish_state(u16_(status, 12) / 1000.0f);
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t tel[32]{0};
    if (this->read_register_(REG_TELEMETRY, tel, sizeof(tel))) {
      // Field offsets follow TELEMETRY ordering in i2c_interface.md.
      if (seq_sensor_ != nullptr)
        seq_sensor_->publish_state(tel[0]);
      if (esc_state_sensor_ != nullptr)
        esc_state_sensor_->publish_state(tel[1]);
      if (esc_state_text_sensor_ != nullptr)
        esc_state_text_sensor_->publish_state(esc_state_to_cstr(tel[1]));
      if (mc_state_sensor_ != nullptr)
        mc_state_sensor_->publish_state(tel[2]);
      if (mc_state_text_sensor_ != nullptr)
        mc_state_text_sensor_->publish_state(mc_state_to_cstr(tel[2]));
      if (last_cmd_error_sensor_ != nullptr)
        last_cmd_error_sensor_->publish_state(tel[3]);
      if (last_cmd_error_text_sensor_ != nullptr)
        last_cmd_error_text_sensor_->publish_state(last_cmd_error_to_cstr(tel[3]));
      if (fault_detail_sensor_ != nullptr)
        fault_detail_sensor_->publish_state(tel[27]);
      if (fault_detail_text_sensor_ != nullptr)
        fault_detail_text_sensor_->publish_state(fault_detail_to_cstr(tel[27]));
      if (status_flags_sensor_ != nullptr)
        status_flags_sensor_->publish_state(u16_(tel, 4));
      if (status_flags_text_sensor_ != nullptr)
        status_flags_text_sensor_->publish_state(bitmask_to_names(u16_(tel, 4), STATUS_FLAG_NAMES, 8));
      if (current_faults_sensor_ != nullptr)
        current_faults_sensor_->publish_state(u16_(tel, 6));
      if (current_faults_text_sensor_ != nullptr)
        current_faults_text_sensor_->publish_state(
          bitmask_to_names(u16_(tel, 6), FAULT_NAMES, 16)
        );
      if (vbus_mv_sensor_ != nullptr)
        vbus_mv_sensor_->publish_state(u32_(tel, 8) / 1000.0f);
      if (ibus_ma_sensor_ != nullptr)
        ibus_ma_sensor_->publish_state(i32_(tel, 12) / 1000.0f);
      if (speed_dhz_sensor_ != nullptr)
        speed_dhz_sensor_->publish_state(i32_(tel, 16));
      if (duty_centi_pct_sensor_ != nullptr)
        duty_centi_pct_sensor_->publish_state(i16_(tel, 20));
      if (temp_mc_sensor_ != nullptr)
        temp_mc_sensor_->publish_state(i32_(tel, 22) / 1000.0f);
      if (last_cmd_seq_sensor_ != nullptr)
        last_cmd_seq_sensor_->publish_state(tel[26]);
      if (uptime_s_sensor_ != nullptr)
        uptime_s_sensor_->publish_state(u32_(tel, 28));
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
  ESP_LOGCONFIG(TAG, "  Speed ramp target dHz: %d", static_cast<int>(speed_ramp_target_dhz_));
  ESP_LOGCONFIG(TAG, "  Speed ramp time ms: %d", static_cast<int>(speed_ramp_time_ms_));
}

}  // namespace esc_higher
}  // namespace esphome
