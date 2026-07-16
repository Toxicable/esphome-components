#include "bq76952_status.h"

#include <cstring>

#include "bq76952_registers.h"

namespace bq76952_core {

namespace {
namespace hw = registers;

bool append_text(char *buffer, size_t buffer_size, size_t &length, const char *text) {
  if (buffer == nullptr || buffer_size == 0 || text == nullptr) {
    return false;
  }
  const size_t text_length = std::strlen(text);
  const size_t separator_length = length == 0 ? 0 : 1;
  if (length + separator_length + text_length >= buffer_size) {
    buffer[buffer_size - 1] = '\0';
    return false;
  }
  if (separator_length != 0) {
    buffer[length++] = ',';
  }
  std::memcpy(buffer + length, text, text_length);
  length += text_length;
  buffer[length] = '\0';
  return true;
}

}  // namespace

OperatingState decode_operating_state(uint16_t control_status, uint16_t battery_status) {
  if ((battery_status & hw::bits::battery_status::CONFIG_UPDATE) != 0) {
    return OperatingState::CONFIG_UPDATE;
  }
  if ((control_status & hw::bits::control_status::DEEP_SLEEP) != 0) {
    return OperatingState::DEEP_SLEEP;
  }
  if ((battery_status & hw::bits::battery_status::SHUTDOWN_COMMAND) != 0) {
    return OperatingState::SHUTDOWN_PENDING;
  }
  if ((battery_status & hw::bits::battery_status::SLEEP) != 0) {
    return OperatingState::SLEEP;
  }
  return OperatingState::NORMAL;
}

uint32_t decode_faults(uint16_t battery_status, uint8_t safety_a, uint8_t safety_b, uint8_t safety_c) {
  uint32_t faults = FAULT_NONE;
  if ((safety_a & hw::bits::protection_a::CUV) != 0) faults |= FAULT_CELL_UNDERVOLTAGE;
  if ((safety_a & hw::bits::protection_a::COV) != 0) faults |= FAULT_CELL_OVERVOLTAGE;
  if ((safety_a & hw::bits::protection_a::OCC) != 0) faults |= FAULT_CHARGE_OVERCURRENT;
  if ((safety_a & hw::bits::protection_a::OCD1) != 0) faults |= FAULT_DISCHARGE_OVERCURRENT;
  if ((safety_a & hw::bits::protection_a::OCD2) != 0) faults |= FAULT_DISCHARGE_SEVERE_OVERCURRENT;
  if ((safety_a & hw::bits::protection_a::SCD) != 0) faults |= FAULT_DISCHARGE_SHORT_CIRCUIT;
  if ((safety_c & hw::bits::protection_c::OCD3) != 0) faults |= FAULT_DISCHARGE_SUSTAINED_OVERCURRENT;
  if ((safety_c & hw::bits::protection_c::PRECHARGE_TIMEOUT) != 0) faults |= FAULT_PRECHARGE_TIMEOUT;
  if ((safety_b & hw::bits::protection_b::ANY_TEMPERATURE) != 0) faults |= FAULT_TEMPERATURE;
  if ((battery_status & hw::bits::battery_status::PERMANENT_FAILURE) != 0) faults |= FAULT_PERMANENT_FAILURE;
  return faults;
}

const char *operating_state_to_string(OperatingState state) {
  switch (state) {
    case OperatingState::NORMAL:
      return "normal";
    case OperatingState::SLEEP:
      return "sleep";
    case OperatingState::DEEP_SLEEP:
      return "deep_sleep";
    case OperatingState::CONFIG_UPDATE:
      return "config_update";
    case OperatingState::SHUTDOWN_PENDING:
      return "shutdown_pending";
    default:
      return "unknown";
  }
}

size_t format_faults(uint32_t active_faults, char *buffer, size_t buffer_size) {
  if (buffer == nullptr || buffer_size == 0) {
    return 0;
  }
  buffer[0] = '\0';
  size_t length = 0;
  const struct {
    uint32_t flag;
    const char *name;
  } entries[] = {
      {FAULT_PERMANENT_FAILURE, "permanent_failure"},
      {FAULT_DISCHARGE_SHORT_CIRCUIT, "discharge_short_circuit"},
      {FAULT_CELL_OVERVOLTAGE, "cell_overvoltage"},
      {FAULT_CELL_UNDERVOLTAGE, "cell_undervoltage"},
      {FAULT_DISCHARGE_SEVERE_OVERCURRENT, "discharge_severe_overcurrent"},
      {FAULT_CHARGE_OVERCURRENT, "charge_overcurrent"},
      {FAULT_DISCHARGE_OVERCURRENT, "discharge_overcurrent"},
      {FAULT_DISCHARGE_SUSTAINED_OVERCURRENT, "discharge_sustained_overcurrent"},
      {FAULT_TEMPERATURE, "temperature"},
      {FAULT_PRECHARGE_TIMEOUT, "precharge_timeout"},
  };
  for (const auto &entry : entries) {
    if ((active_faults & entry.flag) != 0 && !append_text(buffer, buffer_size, length, entry.name)) {
      return length;
    }
  }
  if (length == 0) {
    append_text(buffer, buffer_size, length, "none");
  }
  return length;
}

}  // namespace bq76952_core
