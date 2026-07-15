#pragma once

#include <cstdint>

#include "../component_common/charger.h"

namespace esphome {
namespace programmable_load {

static constexpr float ABSOLUTE_MAXIMUM_VOLTAGE_V = 75.0f;

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
  CHARGER_UNAVAILABLE,
  CHARGER_FAULT,
  CHARGER_CONTROL_ERROR,
  CHARGE_TIMEOUT,
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

enum class ChargerCommand : uint8_t {
  DISABLE = 0,
  ENABLE,
};

struct Measurement {
  // Incremented whenever either electrical measurement publishes a new sample.
  // Procedures can use this to avoid counting the same conversion repeatedly.
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

using ChargerState = ::component_common::ChargerState;
using ChargerMeasurement = ::component_common::ChargerSnapshot;

struct ProcedureContext {
  Measurement load{};
  ChargerMeasurement charger{};
};

struct HardwareLimits {
  // Board-specific limit, additionally capped by
  // ABSOLUTE_MAXIMUM_VOLTAGE_V inside the core.
  float maximum_voltage_v{ABSOLUTE_MAXIMUM_VOLTAGE_V};
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
  ChargerCommand charger_command{ChargerCommand::DISABLE};
};

const char *state_to_string(State state);
const char *fault_to_string(Fault fault);

}  // namespace programmable_load
}  // namespace esphome
