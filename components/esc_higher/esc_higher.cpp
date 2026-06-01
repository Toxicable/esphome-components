#include "esc_higher.h"

#include <string>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
static const char* const CAP_NAMES[] = {
  "speed_command", "duty_command", "current_meas", "temp_meas", "reverse", "brake",
};
static const char* const STATUS_FLAG_NAMES[] = {
  "fault_present",
  "running",
  "watchdog_expired",
  "undervoltage",
  "overvoltage",
  "overtemperature",
  "overcurrent",
  "speed_feedback_unreliable",
};

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
void ESCHigherSetSpeedRampButton::press_action() {
  this->parent_->set_speed_ramp();
}

// TODO: put mapping functins like this in another file
const char* ESCHigherComponent::esc_state_to_cstr_(uint8_t v) {
  switch (v) {
    case 0:
      return "boot";
    case 1:
      return "idle";
    case 2:
      return "running";
    case 3:
      return "stopping";
    case 4:
      return "fault";
    default:
      return "unknown";
  }
}

const char* ESCHigherComponent::last_cmd_error_to_cstr_(uint8_t v) {
  switch (v) {
    case 0:
      return "ok";
    case 1:
      return "unknown_opcode";
    case 2:
      return "invalid_state";
    case 3:
      return "parameter_out_of_range";
    case 4:
      return "motor_fault_active";
    case 5:
      return "busy";
    case 6:
      return "bad_length";
    default:
      return "unknown";
  }
}

std::string ESCHigherComponent::bitmask_to_names_(uint16_t v, const char* const* names, size_t count) {
  if (v == 0)
    return "none";
  std::string out;
  for (size_t i = 0; i < count; i++) {
    if ((v & (1U << i)) == 0)
      continue;
    if (!out.empty())
      out += "|";
    out += names[i];
  }
  if (out.empty())
    out = "unknown_bits";
  return out;
}

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
}

bool ESCHigherComponent::read_register_(uint8_t reg, uint8_t* out, size_t len) {
  const i2c::ErrorCode err = this->write_read(&reg, 1, out, len);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Read reg 0x%02X failed (err=%d)", reg, static_cast<int>(err));
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

  const i2c::ErrorCode err = this->write(tx, sizeof(tx));
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Write command opcode 0x%02X failed (err=%d)", opcode, static_cast<int>(err));
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
        capabilities_text_sensor_->publish_state(bitmask_to_names_(u16_(id, 6), CAP_NAMES, 6));
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
        esc_state_text_sensor_->publish_state(esc_state_to_cstr_(status[1]));
      if (mc_state_sensor_ != nullptr)
        mc_state_sensor_->publish_state(status[2]);
      if (last_cmd_seq_sensor_ != nullptr)
        last_cmd_seq_sensor_->publish_state(status[3]);
      if (last_cmd_error_sensor_ != nullptr)
        last_cmd_error_sensor_->publish_state(status[4]);
      if (last_cmd_error_text_sensor_ != nullptr)
        last_cmd_error_text_sensor_->publish_state(last_cmd_error_to_cstr_(status[4]));
      if (current_faults_sensor_ != nullptr)
        current_faults_sensor_->publish_state(u16_(status, 6));
      if (current_faults_text_sensor_ != nullptr)
        current_faults_text_sensor_->publish_state(
          std::string("0x") + str_sprintf("%04X", u16_(status, 6))
        );
      if (occurred_faults_sensor_ != nullptr)
        occurred_faults_sensor_->publish_state(u16_(status, 8));
      if (occurred_faults_text_sensor_ != nullptr)
        occurred_faults_text_sensor_->publish_state(
          std::string("0x") + str_sprintf("%04X", u16_(status, 8))
        );
      if (status_flags_sensor_ != nullptr)
        status_flags_sensor_->publish_state(u16_(status, 10));
      if (status_flags_text_sensor_ != nullptr)
        status_flags_text_sensor_->publish_state(
          bitmask_to_names_(u16_(status, 10), STATUS_FLAG_NAMES, 8)
        );
      if (watchdog_ms_left_sensor_ != nullptr)
        watchdog_ms_left_sensor_->publish_state(u16_(status, 12));
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
        esc_state_text_sensor_->publish_state(esc_state_to_cstr_(tel[1]));
      if (mc_state_sensor_ != nullptr)
        mc_state_sensor_->publish_state(tel[2]);
      if (last_cmd_error_sensor_ != nullptr)
        last_cmd_error_sensor_->publish_state(tel[3]);
      if (last_cmd_error_text_sensor_ != nullptr)
        last_cmd_error_text_sensor_->publish_state(last_cmd_error_to_cstr_(tel[3]));
      if (status_flags_sensor_ != nullptr)
        status_flags_sensor_->publish_state(u16_(tel, 4));
      if (status_flags_text_sensor_ != nullptr)
        status_flags_text_sensor_->publish_state(bitmask_to_names_(u16_(tel, 4), STATUS_FLAG_NAMES, 8));
      if (current_faults_sensor_ != nullptr)
        current_faults_sensor_->publish_state(u16_(tel, 6));
      if (current_faults_text_sensor_ != nullptr)
        current_faults_text_sensor_->publish_state(
          std::string("0x") + str_sprintf("%04X", u16_(tel, 6))
        );
      if (vbus_mv_sensor_ != nullptr)
        vbus_mv_sensor_->publish_state(u32_(tel, 8));
      if (ibus_ma_sensor_ != nullptr)
        ibus_ma_sensor_->publish_state(i32_(tel, 12));
      if (speed_dhz_sensor_ != nullptr)
        speed_dhz_sensor_->publish_state(i32_(tel, 16));
      if (duty_centi_pct_sensor_ != nullptr)
        duty_centi_pct_sensor_->publish_state(i16_(tel, 20));
      if (temp_mc_sensor_ != nullptr)
        temp_mc_sensor_->publish_state(i32_(tel, 22));
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
