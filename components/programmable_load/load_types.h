#pragma once

#include <cstdint>

namespace esphome {
namespace programmable_load {

enum class State : uint8_t {
  IDLE = 0,
  RUNNING,
  FAULT,
};

enum class Fault : uint8_t {
  NONE = 0,
  CURRENT_MEASUREMENT_UNAVAILABLE,
  VOLTAGE_MEASUREMENT_UNAVAILABLE,
  REQUIRED_TEMPERATURE_UNAVAILABLE,
  INPUT_UNDERVOLTAGE,
  INPUT_OVERVOLTAGE,
  HARDWARE_OVERVOLTAGE,
  OVERCURRENT,
  OVERPOWER,
  OVERTEMPERATURE,
  CONTROL_ERROR,
  PROCEDURE_ERROR,
};

enum class StopReason : uint8_t {
  USER_REQUEST = 0,
  COMPLETED,
  FAULTED,
};

enum class ProcedureStatus : uint8_t {
  RUNNING = 0,
  COMPLETE,
  FAILED,
};

struct Measurement {
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

struct HardwareLimits {
  // Absolute electrical limit of the hardware design. This is checked even
  // when the configured operating limit is absent, invalid, or higher.
  float maximum_voltage_v{0.0f};
};

struct Limits {
  float maximum_current_a{0.0f};
  float minimum_voltage_v{0.0f};
  float maximum_voltage_v{0.0f};
  float maximum_power_w{0.0f};
  float maximum_temperature_c{0.0f};
};

struct FaultPolicy {
  bool auto_clear{false};
  uint32_t clear_delay_ms{2000};
};

struct ProcedureResult {
  ProcedureStatus status{ProcedureStatus::RUNNING};
  float requested_current_a{0.0f};
  Fault fault{Fault::NONE};
};

const char *state_to_string(State state);
const char *fault_to_string(Fault fault);

}  // namespace programmable_load
}  // namespace esphome
