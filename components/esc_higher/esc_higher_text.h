#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace esc_higher {

// I2C error codes
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

// ESC state machine
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

// Last command error codes
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
    default:
      return "unknown";
  }
}

// Capability bitmask names (6 bits)
// MCSDK STM state (motor controller state machine)
static const char* mc_state_to_cstr(uint8_t v) {
  switch (v) {
    case 0: return "init";
    case 1: return "run";
    case 2: return "ramp";
    case 3: return "fault";
    default: return "unknown";
  }
}

static const char* const CAP_NAMES[] = {
  "speed_command", "duty_command", "current_meas", "temp_meas", "reverse", "brake",
};

// Status flag bitmask names (8 bits)
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

// Convert a bitmask to a pipe-separated string of named bits
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

}  // namespace esc_higher
}  // namespace esphome
