#include "mcf8316d_protocol.h"

namespace mcf8316d_core {

uint32_t build_control_word(bool is_read, uint16_t offset, bool is_32bit) {
  const uint32_t dlen = is_32bit ? 0x1u : 0x0u;
  return ((is_read ? 1u : 0u) << 23) | (dlen << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

const char *algorithm_state_to_string(uint16_t state) {
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

const char *brake_input_to_string(uint32_t brake_input_value) {
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

const char *direction_input_to_string(uint32_t direction_input_value) {
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

}  // namespace mcf8316d_core
