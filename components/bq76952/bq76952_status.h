#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "../component_common/status.h"

namespace bq76952_core {

enum class OperatingState : uint8_t {
  UNKNOWN = 0,
  NORMAL,
  SLEEP,
  DEEP_SLEEP,
  CONFIG_UPDATE,
  SHUTDOWN_PENDING,
};

enum class Fault : uint8_t {
  NONE = 0,
  PERMANENT_FAILURE,
  DISCHARGE_SHORT_CIRCUIT,
  CELL_OVERVOLTAGE,
  CELL_UNDERVOLTAGE,
  DISCHARGE_SEVERE_OVERCURRENT,
  CHARGE_OVERCURRENT,
  DISCHARGE_OVERCURRENT,
  DISCHARGE_SUSTAINED_OVERCURRENT,
  TEMPERATURE,
  PRECHARGE_TIMEOUT,
};

enum FaultFlags : uint32_t {
  FAULT_NONE = 0,
  FAULT_CELL_UNDERVOLTAGE = 1U << 0,
  FAULT_CELL_OVERVOLTAGE = 1U << 1,
  FAULT_CHARGE_OVERCURRENT = 1U << 2,
  FAULT_DISCHARGE_OVERCURRENT = 1U << 3,
  FAULT_DISCHARGE_SEVERE_OVERCURRENT = 1U << 4,
  FAULT_DISCHARGE_SUSTAINED_OVERCURRENT = 1U << 5,
  FAULT_DISCHARGE_SHORT_CIRCUIT = 1U << 6,
  FAULT_TEMPERATURE = 1U << 7,
  FAULT_PRECHARGE_TIMEOUT = 1U << 8,
  FAULT_PERMANENT_FAILURE = 1U << 9,
};

using FaultSnapshot = component_common::FaultSnapshot<Fault, uint32_t>;

struct Snapshot {
  component_common::LifecycleState lifecycle{component_common::LifecycleState::DISCONNECTED};
  OperatingState operating_state{OperatingState::UNKNOWN};
  FaultSnapshot faults{};
  bool output_enabled{false};

  std::array<int16_t, 16> cell_voltage_mv{};
  uint8_t cell_count{0};
  int32_t stack_voltage_mv{0};
  int32_t pack_voltage_mv{0};
  int32_t load_detect_voltage_mv{0};
  float current_a{0.0f};
  float state_of_charge_percent{0.0f};
  float learned_capacity_ah{std::numeric_limits<float>::quiet_NaN()};
  float die_temperature_c{0.0f};
  std::array<float, 3> thermistor_temperature_c{};
};

OperatingState decode_operating_state(uint16_t control_status, uint16_t battery_status);
uint32_t decode_fault_flags(uint16_t battery_status, uint8_t safety_a, uint8_t safety_b, uint8_t safety_c);
Fault primary_fault(uint32_t active_flags);
FaultSnapshot make_fault_snapshot(uint32_t active_flags, uint32_t latched_flags = 0);
const char *operating_state_to_string(OperatingState state);
const char *fault_to_string(Fault fault);
size_t format_fault_flags(uint32_t active_flags, char *buffer, size_t buffer_size);

}  // namespace bq76952_core
