#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace esc_higher {

static const char* i2c_error_to_cstr(i2c::ErrorCode err) {
  switch (err) {
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
    default:
      return "unknown";
  }
}

static const char* esc_state_to_cstr(uint8_t v) {
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

static const char* last_cmd_error_to_cstr(uint8_t v) {
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
    case 7:
      return "not_idle";
    case 8:
      return "not_fault_over";
    case 9:
      return "faults_still_active";
    case 10:
      return "latched_faults_present";
    case 17:
      return "unknown_register";
    default:
      return "unknown";
  }
}

static const char* fault_detail_to_cstr(uint8_t v) {
  switch (v) {
    case 0:
      return "none";
    case 1:
      return "overvoltage";
    case 2:
      return "undervoltage";
    case 4:
      return "overtemperature";
    case 5:
      return "startup_failed";
    case 6:
      return "speed_feedback_fault";
    case 7:
      return "overcurrent";
    case 8:
      return "software_error";
    case 9:
      return "driver_protection_fault";
    default:
      return "unknown";
  }
}

static const char* mc_state_to_cstr(uint8_t v) {
  switch (v) {
    case 0:
      return "idle";
    case 4:
      return "start";
    case 6:
      return "run";
    case 8:
      return "stop";
    case 10:
      return "fault_now";
    case 11:
      return "fault_over";
    case 12:
      return "iclwait";
    case 19:
      return "switch_over";
    case 20:
      return "wait_stop_motor";
    case 21:
      return "otf_detection";
    case 22:
      return "otf_brake";
    default:
      return "unknown";
  }
}

static const char* bringup_state_to_cstr(uint8_t v) {
  switch (v) {
    case 0:
      return "idle";
    case 1:
      return "running";
    case 2:
      return "passed";
    case 3:
      return "failed";
    case 4:
      return "aborted";
    default:
      return "unknown";
  }
}

static const char* bringup_result_to_cstr(uint8_t v) {
  switch (v) {
    case 0:
      return "none";
    case 1:
      return "busy";
    case 2:
      return "requires_idle";
    case 3:
      return "active_fault_present";
    case 4:
      return "latched_fault_present";
    case 5:
      return "gd_ready_low";
    case 6:
      return "vbus_too_low";
    case 7:
      return "vbus_too_high";
    case 8:
      return "pwm_handle_missing";
    case 9:
      return "adc_value_implausible";
    case 10:
      return "current_offset_too_large";
    case 11:
      return "temperature_implausible";
    case 12:
      return "mc_state_unexpected";
    case 13:
      return "mcsdk_api_call_failed";
    case 14:
      return "timeout";
    case 15:
      return "aborted_by_host";
    case 16:
      return "unsupported_test";
    case 18:
    return "motor_did_not_spin";
    case 19:
      return "no_differential_pwm";
    case 20:
    return "current_limit_exceeded";
    case 21:
    return "dry_run_complete";
    case 22:
    return "output_disabled";
    case 23:
    return "current_not_valid";
    case 24:
    return "offset_calib_start_failed";
    case 25:
    return "offset_calib_timeout";
    case 26:
    return "offset_calib_not_ready";
    case 17:
      return "passed";
    default:
      return "unknown";
  }
}

static const char* bringup_test_id_to_cstr(uint8_t v) {
  switch (v) {
    case 0:
      return "none";
    case 101:
      return "full_spin_sequence";
    case 102:
      return "bridge_static_vector_test";
    case 103:
      return "forced_timer_diff_pwm";
    default:
      return "unknown";
  }
}

static const char* opcode_to_cstr(uint8_t v) {
  switch (v) {
    case 0x00:
      return "NOP";
    case 0x01:
      return "START";
    case 0x02:
      return "STOP";
    case 0x03:
      return "CLEAR_FAULTS";
    case 0x04:
      return "SET_SPEED_RAMP";
    case 0x05:
      return "ESTOP";
    case 0x07:
      return "SET_WATCHDOG";
    case 0x09:
      return "RUN_BRINGUP_TEST";
    default:
      return "unknown";
  }
}

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

struct FaultMapEntry {
  uint16_t bit;
  const char* name;
};

static const FaultMapEntry FAULT_MAP[] = {
  {0x0002, "overvoltage"},
  {0x0004, "undervoltage"},
  {0x0008, "overtemperature"},
  {0x0010, "startup_failed"},
  {0x0020, "speed_feedback_fault"},
  {0x0040, "overcurrent"},
  {0x0080, "software_error"},
  {0x0400, "driver_protection"},
};

static std::string bitmask_to_names(uint16_t v, const char* const* names, size_t count) {
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

static std::string fault_bitmask_to_names(uint16_t v) {
  if (v == 0)
    return "none";
  std::string out;
  uint16_t known = 0;
  for (size_t i = 0; i < (sizeof(FAULT_MAP) / sizeof(FAULT_MAP[0])); i++) {
    if ((v & FAULT_MAP[i].bit) == 0)
      continue;
    known |= FAULT_MAP[i].bit;
    if (!out.empty())
      out += "|";
    out += FAULT_MAP[i].name;
  }
  if ((v & static_cast<uint16_t>(~known)) != 0) {
    if (!out.empty())
      out += "|";
    out += "unknown_bits";
  }
  return out.empty() ? "unknown_bits" : out;
}

static const char* debug_event_id_to_cstr(uint16_t event_id) {
  switch (event_id) {
    case 0x0100:
      return "BRINGUP_START";
    case 0x0101:
      return "TEST102_CONFIG";
    case 0x0102:
      return "TEST102_VECTOR_BEGIN";
    case 0x0103:
      return "TEST102_PWMC_BEFORE";
    case 0x0104:
      return "TEST102_PWMC_AFTER";
    case 0x0105:
      return "TEST102_TIM_BEFORE";
    case 0x0106:
      return "TEST102_TIM_AFTER";
    case 0x0107:
      return "TEST102_VOLTAGE_COMMAND";
    case 0x0108:
      return "TEST102_SETPHASE_RESULT";
    case 0x0109:
      return "TEST102_CURRENT_SNAPSHOT";
    case 0x010A:
      return "TEST102_VECTOR_RESULT";
    case 0x010B:
      return "BRINGUP_FAIL";
    case 0x010C:
      return "BRINGUP_ABORT";
    case 0x010D:
      return "BRINGUP_PASS";
    case 0x010E:
      return "MCSDK_SNAPSHOT";
    case 0x010F:
      return "CURRENT_SNAPSHOT";
    case 0x0110:
    return "TEST102_OFFSET_CALIB_STATE";
    case 0x0111:
    return "TEST102_OFFSET_CALIB_START";
    case 0x0112:
    return "TEST102_OFFSET_CALIB_START_RESULT";
    default:
      return nullptr;
  }
}

}  // namespace esc_higher
}  // namespace esphome
