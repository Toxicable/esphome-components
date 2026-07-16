#include "mcf8329a_protocol.h"

#include "mcf8329a_tables.h"

namespace mcf8329a_core {


float decode_vm_voltage(uint32_t raw) {
  return static_cast<float>(static_cast<double>(raw) * (60.0 / 134217728.0));
}

float decode_max_speed_hz(uint16_t code) {
  const uint16_t clamped = code & 0x3FFFu;
  if (clamped <= 9600u) {
    return static_cast<float>(clamped) / 6.0f;
  }
  return (static_cast<float>(clamped) / 4.0f) - 800.0f;
}

float decode_speed_hz(int32_t raw, float max_speed_hz) {
  return static_cast<float>(raw) * (1.0f / 134217728.0f) * max_speed_hz;
}

float decode_fg_speed_hz(uint32_t raw, float max_speed_hz) {
  return static_cast<float>(raw) * (1.0f / 134217728.0f) * max_speed_hz;
}

float decode_open_loop_accel_hz_per_s(uint8_t code) {
  return tables::OPEN_LOOP_ACCEL_HZ_PER_S[code & 0x0Fu];
}

float decode_open_to_closed_handoff_percent(uint8_t code) {
  return tables::OPEN_TO_CLOSED_HANDOFF_PERCENT[code & 0x1Fu];
}

const char *mode_to_string(uint8_t mode) {
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

const char *align_time_to_string(uint8_t code) {
  static const char *const kAlignTimeLabels[16] = {
    "10ms",  "50ms",   "100ms",  "200ms",  "300ms",  "400ms", "500ms", "750ms",
    "1000ms","1500ms", "2000ms", "3000ms", "4000ms", "5000ms","7500ms","10000ms",
  };
  return kAlignTimeLabels[code & 0x0Fu];
}

const char *brake_mode_to_string(uint8_t code) {
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

const char *brake_time_to_string(uint8_t code) {
  static const char *const kBrakeTimeLabels[16] = {
    "1ms",    "1ms",    "1ms",    "1ms",    "1ms",    "5ms",    "10ms",   "50ms",
    "100ms",  "250ms",  "500ms",  "1000ms", "2500ms", "5000ms", "10000ms","15000ms",
  };
  return kBrakeTimeLabels[code & 0x0Fu];
}

const char *algorithm_state_to_string(uint16_t state) {
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

const char *lock_mode_to_string(uint8_t mode) {
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

const char *lock_retry_time_to_string(uint8_t code) {
  static const char *const kLockRetryLabels[16] = {
    "300ms", "500ms", "1s",   "2s",   "3s",   "4s",   "5s",   "6s",
    "7s",    "8s",    "9s",   "10s",  "11s",  "12s",  "13s",  "14s",
  };
  return kLockRetryLabels[code & 0x0Fu];
}

const char *brake_input_to_string(uint32_t brake_input_value) {
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

}  // namespace mcf8329a_core
