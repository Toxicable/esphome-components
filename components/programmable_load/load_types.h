#pragma once

#include <cstdint>

namespace esphome {
namespace programmable_load {

enum class LoadState : uint8_t {
  IDLE = 0,
  RUNNING,
  PAUSED,
  COMPLETE,
  FAULT,
};

enum class LoadFault : uint8_t {
  NONE = 0,
  CURRENT_MEASUREMENT_UNAVAILABLE,
  VOLTAGE_MEASUREMENT_UNAVAILABLE,
  REQUIRED_TEMPERATURE_UNAVAILABLE,
  INPUT_UNDERVOLTAGE,
  INPUT_OVERVOLTAGE,
  OVERCURRENT,
  OVERPOWER,
  OVERTEMPERATURE,
  CONTROL_ERROR,
  PROCEDURE_ERROR,
};

enum class LoadStopReason : uint8_t {
  USER_REQUEST = 0,
  COMPLETED,
  FAULTED,
  REPLACED,
};

struct LoadMeasurement {
  uint32_t sequence{0};
  uint32_t timestamp_ms{0};
  float current_a{0.0f};
  float voltage_v{0.0f};
  float power_w{0.0f};
  float maximum_temperature_c{0.0f};
  bool current_valid{false};
  bool voltage_valid{false};
  bool temperature_valid{false};
};

struct LoadLimits {
  float maximum_current_a{0.0f};
  float minimum_voltage_v{0.0f};
  float maximum_voltage_v{0.0f};
  float maximum_power_w{0.0f};
  float maximum_temperature_c{0.0f};
};

struct LoadFaultPolicy {
  bool auto_clear{false};
  uint32_t clear_delay_ms{2000};
};

const char *load_state_to_string(LoadState state);
const char *load_fault_to_string(LoadFault fault);

}  // namespace programmable_load
}  // namespace esphome
