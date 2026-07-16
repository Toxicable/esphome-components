#include "programmable_load_core.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace programmable_load_core {

namespace {

bool finite_positive(float value) {
  return std::isfinite(value) && value > 0.0f;
}

void append_fault(const char *name, char *buffer, std::size_t size,
                  std::size_t &used, bool &first) {
  if (size == 0 || used >= size - 1) return;
  const int written = std::snprintf(buffer + used, size - used, "%s%s",
                                    first ? "" : ",", name);
  if (written <= 0) return;
  const std::size_t count = static_cast<std::size_t>(written);
  used += std::min(count, size - used - 1);
  first = false;
}

}  // namespace

bool OperationLock::acquire_manual() {
  if (this->owner_ != OperationOwner::NONE) return false;
  this->owner_ = OperationOwner::MANUAL;
  this->procedure_ = nullptr;
  return true;
}

bool OperationLock::acquire_procedure(const void *procedure) {
  if (procedure == nullptr || this->owner_ != OperationOwner::NONE) return false;
  this->owner_ = OperationOwner::PROCEDURE;
  this->procedure_ = procedure;
  return true;
}

bool OperationLock::owns_procedure(const void *procedure) const {
  return procedure != nullptr && this->owner_ == OperationOwner::PROCEDURE &&
         this->procedure_ == procedure;
}

bool OperationLock::release_manual() {
  if (!this->owns_manual()) return false;
  this->force_release();
  return true;
}

bool OperationLock::release_procedure(const void *procedure) {
  if (!this->owns_procedure(procedure)) return false;
  this->force_release();
  return true;
}

void OperationLock::force_release() {
  this->owner_ = OperationOwner::NONE;
  this->procedure_ = nullptr;
}

const char *state_to_string(State state) {
  switch (state) {
    case State::IDLE: return "idle";
    case State::RUNNING: return "running";
    case State::FAULT: return "fault";
    default: return "unknown";
  }
}

const char *fault_to_string(Fault fault) {
  switch (fault) {
    case Fault::NONE: return "none";
    case Fault::CURRENT_MEASUREMENT_UNAVAILABLE:
      return "current_measurement_unavailable";
    case Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE:
      return "voltage_measurement_unavailable";
    case Fault::REQUIRED_TEMPERATURE_UNAVAILABLE:
      return "required_temperature_unavailable";
    case Fault::INPUT_UNDERVOLTAGE: return "input_undervoltage";
    case Fault::INPUT_OVERVOLTAGE: return "input_overvoltage";
    case Fault::HARDWARE_OVERVOLTAGE: return "hardware_overvoltage";
    case Fault::OVERCURRENT: return "overcurrent";
    case Fault::OVERPOWER: return "overpower";
    case Fault::OVERTEMPERATURE: return "overtemperature";
    case Fault::CHARGER_UNAVAILABLE: return "charger_unavailable";
    case Fault::CHARGER_FAULT: return "charger_fault";
    case Fault::CHARGER_CONTROL_ERROR: return "charger_control_error";
    case Fault::CHARGE_TIMEOUT: return "charge_timeout";
    case Fault::CONTROL_ERROR: return "control_error";
    case Fault::PROCEDURE_ERROR: return "procedure_error";
    default: return "unknown";
  }
}

const char *calibration_source_to_string(CalibrationSource source) {
  switch (source) {
    case CalibrationSource::CONFIGURED: return "configured";
    case CalibrationSource::RESTORED: return "restored";
    case CalibrationSource::APPLIED: return "applied";
    default: return "unknown";
  }
}

std::size_t format_faults(FaultFlags flags, char *buffer, std::size_t size) {
  if (size == 0) return 0;
  buffer[0] = '\0';
  if (flags == 0u) {
    const int written = std::snprintf(buffer, size, "none");
    return written <= 0 ? 0u : std::min(static_cast<std::size_t>(written), size - 1);
  }

  std::size_t used = 0;
  bool first = true;
  for (uint8_t value = static_cast<uint8_t>(Fault::CURRENT_MEASUREMENT_UNAVAILABLE);
       value <= static_cast<uint8_t>(Fault::PROCEDURE_ERROR); value++) {
    const auto fault = static_cast<Fault>(value);
    if (has_fault(flags, fault)) {
      append_fault(fault_to_string(fault), buffer, size, used, first);
    }
  }
  return used;
}

float normalize_hardware_maximum_voltage(float voltage_v) {
  if (!finite_positive(voltage_v) ||
      voltage_v > ABSOLUTE_MAXIMUM_VOLTAGE_V) {
    return ABSOLUTE_MAXIMUM_VOLTAGE_V;
  }
  return voltage_v;
}

bool calibration_valid(const Calibration &calibration) {
  return calibration.version == CALIBRATION_VERSION &&
         finite_positive(calibration.current.scale) &&
         std::isfinite(calibration.current.offset) &&
         finite_positive(calibration.voltage.scale) &&
         std::isfinite(calibration.voltage.offset) &&
         std::isfinite(calibration.output.zero_level) &&
         calibration.output.zero_level >= 0.0f &&
         calibration.output.zero_level < 1.0f &&
         finite_positive(calibration.output.full_scale_current_a);
}

float effective_current_limit(const Measurement &measurement,
                              const Limits &limits,
                              const Calibration &calibration) {
  if (!measurement.voltage_valid || measurement.voltage_v <= 0.0f ||
      !calibration_valid(calibration)) {
    return 0.0f;
  }
  float limit = std::min(limits.maximum_current_a,
                         calibration.output.full_scale_current_a);
  if (limits.maximum_power_w > 0.0f) {
    limit = std::min(limit,
                     limits.maximum_power_w / std::fabs(measurement.voltage_v));
  }
  return std::max(0.0f, limit);
}

FaultFlags detect_safety_faults(const Measurement &measurement,
                                const HardwareLimits &hardware_limits,
                                const Limits &limits, float current_deadband_a,
                                bool required_temperature_unavailable,
                                bool charger_control_mismatch) {
  FaultFlags faults = 0u;
  if (!measurement.current_valid)
    faults |= fault_flag(Fault::CURRENT_MEASUREMENT_UNAVAILABLE);
  if (!measurement.voltage_valid)
    faults |= fault_flag(Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE);
  if (required_temperature_unavailable)
    faults |= fault_flag(Fault::REQUIRED_TEMPERATURE_UNAVAILABLE);

  if (measurement.voltage_valid) {
    if (measurement.voltage_v > hardware_limits.maximum_voltage_v)
      faults |= fault_flag(Fault::HARDWARE_OVERVOLTAGE);
    if (measurement.voltage_v < limits.minimum_voltage_v)
      faults |= fault_flag(Fault::INPUT_UNDERVOLTAGE);
    if (measurement.voltage_v > limits.maximum_voltage_v)
      faults |= fault_flag(Fault::INPUT_OVERVOLTAGE);
  }
  if (measurement.current_valid &&
      std::fabs(measurement.current_a) >
          limits.maximum_current_a + current_deadband_a) {
    faults |= fault_flag(Fault::OVERCURRENT);
  }
  if (measurement.current_valid && measurement.voltage_valid &&
      std::fabs(measurement.power_w) > limits.maximum_power_w) {
    faults |= fault_flag(Fault::OVERPOWER);
  }
  if (std::isfinite(measurement.maximum_temperature_c) &&
      measurement.maximum_temperature_c > limits.maximum_temperature_c) {
    faults |= fault_flag(Fault::OVERTEMPERATURE);
  }
  if (charger_control_mismatch)
    faults |= fault_flag(Fault::CHARGER_CONTROL_ERROR);
  return faults;
}

bool fault_conditions_active(FaultFlags flags, const Measurement &measurement,
                             const HardwareLimits &hardware_limits,
                             const Limits &limits, float current_deadband_a,
                             bool required_temperature_unavailable,
                             bool charger_control_mismatch,
                             bool charger_configured,
                             bool charger_measurement_valid,
                             bool charger_fault_active) {
  if (flags == 0u) return false;
  if (has_fault(flags, Fault::CURRENT_MEASUREMENT_UNAVAILABLE) &&
      !measurement.current_valid)
    return true;
  if (has_fault(flags, Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE) &&
      !measurement.voltage_valid)
    return true;
  if (has_fault(flags, Fault::REQUIRED_TEMPERATURE_UNAVAILABLE) &&
      required_temperature_unavailable)
    return true;
  if (has_fault(flags, Fault::INPUT_UNDERVOLTAGE) &&
      (!measurement.voltage_valid ||
       measurement.voltage_v < limits.minimum_voltage_v))
    return true;
  if (has_fault(flags, Fault::INPUT_OVERVOLTAGE) &&
      (!measurement.voltage_valid ||
       measurement.voltage_v > limits.maximum_voltage_v))
    return true;
  if (has_fault(flags, Fault::HARDWARE_OVERVOLTAGE) &&
      (!measurement.voltage_valid ||
       measurement.voltage_v > hardware_limits.maximum_voltage_v))
    return true;
  if (has_fault(flags, Fault::OVERCURRENT) &&
      (!measurement.current_valid ||
       std::fabs(measurement.current_a) >
           limits.maximum_current_a + current_deadband_a))
    return true;
  if (has_fault(flags, Fault::OVERPOWER) &&
      (!measurement.current_valid || !measurement.voltage_valid ||
       std::fabs(measurement.power_w) > limits.maximum_power_w))
    return true;
  if (has_fault(flags, Fault::OVERTEMPERATURE) &&
      (required_temperature_unavailable ||
       (std::isfinite(measurement.maximum_temperature_c) &&
        measurement.maximum_temperature_c > limits.maximum_temperature_c)))
    return true;
  if (has_fault(flags, Fault::CHARGER_UNAVAILABLE) && charger_configured &&
      !charger_measurement_valid)
    return true;
  if (has_fault(flags, Fault::CHARGER_FAULT) && charger_measurement_valid &&
      charger_fault_active)
    return true;
  if (has_fault(flags, Fault::CHARGER_CONTROL_ERROR) &&
      charger_control_mismatch)
    return true;

  // Timeout, control and procedure failures are latched root causes without a
  // continuously observable condition. Once the output and charger are safe,
  // they may be cleared explicitly or by the configured auto-clear delay.
  return false;
}

}  // namespace programmable_load_core
