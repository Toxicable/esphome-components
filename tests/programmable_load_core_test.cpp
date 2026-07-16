#include <cassert>
#include <cmath>
#include <cstring>

#include "../components/programmable_load/programmable_load_core.h"

namespace core = programmable_load_core;

namespace {

void test_operation_lock() {
  core::OperationLock lock;
  int procedure_a = 1;
  int procedure_b = 2;

  assert(lock.owner() == core::OperationOwner::NONE);
  assert(lock.acquire_manual());
  assert(lock.owns_manual());
  assert(!lock.acquire_manual());
  assert(!lock.acquire_procedure(&procedure_a));
  assert(!lock.release_procedure(&procedure_a));
  assert(lock.release_manual());
  assert(lock.owner() == core::OperationOwner::NONE);

  assert(lock.acquire_procedure(&procedure_a));
  assert(lock.owns_procedure(&procedure_a));
  assert(!lock.owns_procedure(&procedure_b));
  assert(!lock.acquire_manual());
  assert(!lock.release_procedure(&procedure_b));
  assert(lock.release_procedure(&procedure_a));
  assert(lock.owner() == core::OperationOwner::NONE);

  assert(!lock.acquire_procedure(nullptr));
  lock.force_release();
  assert(lock.owner() == core::OperationOwner::NONE);
}

void test_fault_formatting() {
  char buffer[256];
  assert(core::format_faults(0u, buffer, sizeof(buffer)) == 4u);
  assert(std::strcmp(buffer, "none") == 0);

  core::FaultFlags flags = 0u;
  flags |= core::fault_flag(core::Fault::INPUT_OVERVOLTAGE);
  flags |= core::fault_flag(core::Fault::OVERCURRENT);
  flags |= core::fault_flag(core::Fault::OVERTEMPERATURE);
  core::format_faults(flags, buffer, sizeof(buffer));
  assert(std::strcmp(buffer,
                     "input_overvoltage,overcurrent,overtemperature") == 0);
}

core::Calibration valid_calibration() {
  core::Calibration calibration{};
  calibration.current.scale = 1.0f;
  calibration.current.offset = 0.0f;
  calibration.voltage.scale = 1.0f;
  calibration.voltage.offset = 0.0f;
  calibration.output.zero_level = 0.02f;
  calibration.output.full_scale_current_a = 50.0f;
  return calibration;
}

void test_hardware_voltage_normalization() {
  assert(core::normalize_hardware_maximum_voltage(48.0f) == 48.0f);
  assert(core::normalize_hardware_maximum_voltage(75.0f) == 75.0f);
  assert(core::normalize_hardware_maximum_voltage(76.0f) == 75.0f);
  assert(core::normalize_hardware_maximum_voltage(-1.0f) == 75.0f);
  assert(core::normalize_hardware_maximum_voltage(NAN) == 75.0f);
}

void test_calibration_and_current_limit() {
  auto calibration = valid_calibration();
  assert(core::calibration_valid(calibration));
  assert(std::strcmp(core::calibration_source_to_string(
                         core::CalibrationSource::CONFIGURED),
                     "configured") == 0);
  assert(std::strcmp(core::calibration_source_to_string(
                         core::CalibrationSource::RESTORED),
                     "restored") == 0);
  assert(std::strcmp(core::calibration_source_to_string(
                         core::CalibrationSource::APPLIED),
                     "applied") == 0);

  calibration.output.zero_level = 1.0f;
  assert(!core::calibration_valid(calibration));
  calibration = valid_calibration();
  calibration.output.full_scale_current_a = 0.0f;
  assert(!core::calibration_valid(calibration));

  calibration = valid_calibration();
  core::Measurement measurement{};
  measurement.voltage_valid = true;
  measurement.voltage_v = 20.0f;
  core::Limits limits{};
  limits.maximum_current_a = 40.0f;
  limits.maximum_power_w = 500.0f;

  const float limit = core::effective_current_limit(measurement, limits,
                                                    calibration);
  assert(std::fabs(limit - 25.0f) < 0.0001f);

  measurement.voltage_valid = false;
  assert(core::effective_current_limit(measurement, limits, calibration) ==
         0.0f);
}

void test_multi_fault_detection_and_clear_conditions() {
  core::Measurement measurement{};
  measurement.current_valid = true;
  measurement.voltage_valid = true;
  measurement.temperature_valid = true;
  measurement.current_a = 12.0f;
  measurement.voltage_v = 80.0f;
  measurement.power_w = 960.0f;
  measurement.maximum_temperature_c = 110.0f;

  core::HardwareLimits hardware{};
  hardware.maximum_voltage_v = 75.0f;
  core::Limits limits{};
  limits.maximum_current_a = 10.0f;
  limits.minimum_voltage_v = 10.0f;
  limits.maximum_voltage_v = 60.0f;
  limits.maximum_power_w = 500.0f;
  limits.maximum_temperature_c = 100.0f;

  const core::FaultFlags faults = core::detect_safety_faults(
      measurement, hardware, limits, 0.01f, false, true);
  assert(core::has_fault(faults, core::Fault::HARDWARE_OVERVOLTAGE));
  assert(core::has_fault(faults, core::Fault::INPUT_OVERVOLTAGE));
  assert(core::has_fault(faults, core::Fault::OVERCURRENT));
  assert(core::has_fault(faults, core::Fault::OVERPOWER));
  assert(core::has_fault(faults, core::Fault::OVERTEMPERATURE));
  assert(core::has_fault(faults, core::Fault::CHARGER_CONTROL_ERROR));
  assert(!core::has_fault(faults, core::Fault::INPUT_UNDERVOLTAGE));

  assert(core::fault_conditions_active(
      faults, measurement, hardware, limits, 0.01f, false, true, true, true,
      false));

  measurement.current_a = 0.0f;
  measurement.voltage_v = 50.0f;
  measurement.power_w = 0.0f;
  measurement.maximum_temperature_c = 25.0f;
  assert(!core::fault_conditions_active(
      faults, measurement, hardware, limits, 0.01f, false, false, true, true,
      false));

  const core::FaultFlags unavailable =
      core::fault_flag(core::Fault::CURRENT_MEASUREMENT_UNAVAILABLE) |
      core::fault_flag(core::Fault::VOLTAGE_MEASUREMENT_UNAVAILABLE);
  measurement.current_valid = false;
  measurement.voltage_valid = false;
  assert(core::fault_conditions_active(
      unavailable, measurement, hardware, limits, 0.01f, false, false, false,
      false, false));
}

}  // namespace

int main() {
  test_operation_lock();
  test_fault_formatting();
  test_hardware_voltage_normalization();
  test_calibration_and_current_limit();
  test_multi_fault_detection_and_clear_conditions();
  return 0;
}
