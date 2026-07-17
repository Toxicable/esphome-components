#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/register_info.h"

namespace mcf8316d_core {
namespace regs {

enum class RegisterId : uint8_t {
  CONTROLLER_FAULT_STATUS,
  GATE_DRIVER_FAULT_STATUS,
  ALGO_STATUS,
  MTR_PARAMS,
  ALGO_STATUS_MPET,
  ALGO_CTRL1,
  ALGO_DEBUG1,
  ALGO_DEBUG2,
  ALGORITHM_STATE,
  ISD_CONFIG,
  REV_DRIVE_CONFIG,
  MOTOR_STARTUP1,
  MOTOR_STARTUP2,
  CLOSED_LOOP1,
  CLOSED_LOOP2,
  CLOSED_LOOP3,
  CLOSED_LOOP4,
  FAULT_CONFIG1,
  FAULT_CONFIG2,
  PIN_CONFIG,
  DEVICE_CONFIG1,
  DEVICE_CONFIG2,
  PERI_CONFIG1,
  GD_CONFIG1,
  GD_CONFIG2,
  CSA_GAIN_FEEDBACK,
  VOLTAGE_GAIN_FEEDBACK,
  VM_VOLTAGE,
  COUNT,
};

using RegisterInfo = component_common::RegisterInfo<RegisterId>;
using component_common::RegisterWidth;

inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);

inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::CONTROLLER_FAULT_STATUS, .name = "controller_fault_status", .address = 0x00E2, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::GATE_DRIVER_FAULT_STATUS, .name = "gate_driver_fault_status", .address = 0x00E0, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_STATUS, .name = "algo_status", .address = 0x00E4, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::MTR_PARAMS, .name = "motor_parameters", .address = 0x00E6, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_STATUS_MPET, .name = "algo_status_mpet", .address = 0x00E8, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_CTRL1, .name = "algorithm_control_1", .address = 0x00EA, .width = RegisterWidth::U32},
    {.id = RegisterId::ALGO_DEBUG1, .name = "algorithm_debug_1", .address = 0x00EC, .width = RegisterWidth::U32},
    {.id = RegisterId::ALGO_DEBUG2, .name = "algorithm_debug_2", .address = 0x00EE, .width = RegisterWidth::U32},
    {.id = RegisterId::ALGORITHM_STATE, .name = "algorithm_state", .address = 0x018E, .width = RegisterWidth::U16, .masks = {.status = 0xFFFF}},
    {.id = RegisterId::ISD_CONFIG, .name = "isd_config", .address = 0x0080, .width = RegisterWidth::U32},
    {.id = RegisterId::REV_DRIVE_CONFIG, .name = "reverse_drive_config", .address = 0x0082, .width = RegisterWidth::U32},
    {.id = RegisterId::MOTOR_STARTUP1, .name = "motor_startup_1", .address = 0x0084, .width = RegisterWidth::U32},
    {.id = RegisterId::MOTOR_STARTUP2, .name = "motor_startup_2", .address = 0x0086, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP1, .name = "closed_loop_1", .address = 0x0088, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP2, .name = "closed_loop_2", .address = 0x008A, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP3, .name = "closed_loop_3", .address = 0x008C, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP4, .name = "closed_loop_4", .address = 0x008E, .width = RegisterWidth::U32},
    {.id = RegisterId::FAULT_CONFIG1, .name = "fault_config_1", .address = 0x0090, .width = RegisterWidth::U32},
    {.id = RegisterId::FAULT_CONFIG2, .name = "fault_config_2", .address = 0x0092, .width = RegisterWidth::U32},
    {.id = RegisterId::PIN_CONFIG, .name = "pin_config", .address = 0x00A4, .width = RegisterWidth::U32},
    {.id = RegisterId::DEVICE_CONFIG1, .name = "device_config_1", .address = 0x00A6, .width = RegisterWidth::U32},
    {.id = RegisterId::DEVICE_CONFIG2, .name = "device_config_2", .address = 0x00A8, .width = RegisterWidth::U32},
    {.id = RegisterId::PERI_CONFIG1, .name = "peripheral_config_1", .address = 0x00AA, .width = RegisterWidth::U32},
    {.id = RegisterId::GD_CONFIG1, .name = "gate_driver_config_1", .address = 0x00AC, .width = RegisterWidth::U32},
    {.id = RegisterId::GD_CONFIG2, .name = "gate_driver_config_2", .address = 0x00AE, .width = RegisterWidth::U32},
    {.id = RegisterId::CSA_GAIN_FEEDBACK, .name = "csa_gain_feedback", .address = 0x046C, .width = RegisterWidth::U16, .masks = {.status = 0xFFFF}},
    {.id = RegisterId::VOLTAGE_GAIN_FEEDBACK, .name = "voltage_gain_feedback", .address = 0x0477, .width = RegisterWidth::U16, .masks = {.status = 0xFFFF}},
    {.id = RegisterId::VM_VOLTAGE, .name = "vm_voltage", .address = 0x047C, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
}};

static_assert(component_common::register_definitions_have_all_ids_once(REGISTER_DEFINITIONS),
              "MCF8316D register definitions must contain every RegisterId exactly once");
static_assert(component_common::register_definitions_have_unique_addresses(REGISTER_DEFINITIONS),
              "MCF8316D register definitions must have unique addresses");

inline constexpr auto REGISTER_INFO =
    component_common::index_register_info_by_id(REGISTER_DEFINITIONS);

constexpr const RegisterInfo &register_info(RegisterId id) {
  return component_common::register_info(REGISTER_INFO, id);
}

constexpr uint16_t register_address(RegisterId id) {
  return register_info(id).address;
}

inline constexpr uint16_t REG_CONTROLLER_FAULT_STATUS = register_address(RegisterId::CONTROLLER_FAULT_STATUS);
inline constexpr uint16_t REG_GATE_DRIVER_FAULT_STATUS = register_address(RegisterId::GATE_DRIVER_FAULT_STATUS);
inline constexpr uint16_t REG_ALGO_STATUS = register_address(RegisterId::ALGO_STATUS);
inline constexpr uint16_t REG_MTR_PARAMS = register_address(RegisterId::MTR_PARAMS);
inline constexpr uint16_t REG_ALGO_STATUS_MPET = register_address(RegisterId::ALGO_STATUS_MPET);
inline constexpr uint16_t REG_ALGO_CTRL1 = register_address(RegisterId::ALGO_CTRL1);
inline constexpr uint16_t REG_ALGO_DEBUG1 = register_address(RegisterId::ALGO_DEBUG1);
inline constexpr uint16_t REG_ALGO_DEBUG2 = register_address(RegisterId::ALGO_DEBUG2);
inline constexpr uint16_t REG_ALGORITHM_STATE = register_address(RegisterId::ALGORITHM_STATE);
inline constexpr uint16_t REG_ISD_CONFIG = register_address(RegisterId::ISD_CONFIG);
inline constexpr uint16_t REG_REV_DRIVE_CONFIG = register_address(RegisterId::REV_DRIVE_CONFIG);
inline constexpr uint16_t REG_MOTOR_STARTUP1 = register_address(RegisterId::MOTOR_STARTUP1);
inline constexpr uint16_t REG_MOTOR_STARTUP2 = register_address(RegisterId::MOTOR_STARTUP2);
inline constexpr uint16_t REG_CLOSED_LOOP1 = register_address(RegisterId::CLOSED_LOOP1);
inline constexpr uint16_t REG_CLOSED_LOOP2 = register_address(RegisterId::CLOSED_LOOP2);
inline constexpr uint16_t REG_CLOSED_LOOP3 = register_address(RegisterId::CLOSED_LOOP3);
inline constexpr uint16_t REG_CLOSED_LOOP4 = register_address(RegisterId::CLOSED_LOOP4);
inline constexpr uint16_t REG_FAULT_CONFIG1 = register_address(RegisterId::FAULT_CONFIG1);
inline constexpr uint16_t REG_FAULT_CONFIG2 = register_address(RegisterId::FAULT_CONFIG2);
inline constexpr uint16_t REG_PIN_CONFIG = register_address(RegisterId::PIN_CONFIG);
inline constexpr uint16_t REG_DEVICE_CONFIG1 = register_address(RegisterId::DEVICE_CONFIG1);
inline constexpr uint16_t REG_DEVICE_CONFIG2 = register_address(RegisterId::DEVICE_CONFIG2);
inline constexpr uint16_t REG_PERI_CONFIG1 = register_address(RegisterId::PERI_CONFIG1);
inline constexpr uint16_t REG_GD_CONFIG1 = register_address(RegisterId::GD_CONFIG1);
inline constexpr uint16_t REG_GD_CONFIG2 = register_address(RegisterId::GD_CONFIG2);
inline constexpr uint16_t REG_CSA_GAIN_FEEDBACK = register_address(RegisterId::CSA_GAIN_FEEDBACK);
inline constexpr uint16_t REG_VOLTAGE_GAIN_FEEDBACK = register_address(RegisterId::VOLTAGE_GAIN_FEEDBACK);
inline constexpr uint16_t REG_VM_VOLTAGE = register_address(RegisterId::VM_VOLTAGE);

}  // namespace regs
}  // namespace mcf8316d_core
