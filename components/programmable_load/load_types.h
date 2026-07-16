#pragma once

#include "programmable_load_core.h"

namespace esphome {
namespace programmable_load {

using ::programmable_load_core::ABSOLUTE_MAXIMUM_VOLTAGE_V;
using ::programmable_load_core::CALIBRATION_VERSION;
using ::programmable_load_core::Calibration;
using ::programmable_load_core::CalibrationSource;
using ::programmable_load_core::ChargerCommand;
using ::programmable_load_core::ChargerMeasurement;
using ::programmable_load_core::ChargerState;
using ::programmable_load_core::Fault;
using ::programmable_load_core::FaultFlags;
using ::programmable_load_core::FaultPolicy;
using ::programmable_load_core::HardwareLimits;
using ::programmable_load_core::Limits;
using ::programmable_load_core::LinearCalibration;
using ::programmable_load_core::Measurement;
using ::programmable_load_core::OperationLock;
using ::programmable_load_core::OperationOwner;
using ::programmable_load_core::OutputCalibration;
using ::programmable_load_core::ProcedureContext;
using ::programmable_load_core::ProcedureResult;
using ::programmable_load_core::ProcedureStatus;
using ::programmable_load_core::State;
using ::programmable_load_core::StopReason;
using ::programmable_load_core::calibration_source_to_string;
using ::programmable_load_core::fault_flag;
using ::programmable_load_core::fault_to_string;
using ::programmable_load_core::format_faults;
using ::programmable_load_core::has_fault;
using ::programmable_load_core::normalize_hardware_maximum_voltage;
using ::programmable_load_core::state_to_string;

}  // namespace programmable_load
}  // namespace esphome
