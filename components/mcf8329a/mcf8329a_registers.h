#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/register_info.h"

namespace mcf8329a_core {
namespace regs {

enum class RegisterId : uint8_t {
  CONTROLLER_FAULT_STATUS,
  GATE_DRIVER_FAULT_STATUS,
  ALGO_STATUS,
  ALGO_STATUS_MPET,
  ALGORITHM_STATE,
  MTR_PARAMS,
  ALGO_CTRL1,
  ALGO_DEBUG1,
  ALGO_DEBUG2,
  PIN_CONFIG,
  PERI_CONFIG1,
  MOTOR_STARTUP1,
  MOTOR_STARTUP2,
  CLOSED_LOOP2,
  CLOSED_LOOP3,
  CLOSED_LOOP4,
  GD_CONFIG1,
  GD_CONFIG2,
  FAULT_CONFIG1,
  FAULT_CONFIG2,
  INT_ALGO_1,
  INT_ALGO_2,
  VM_VOLTAGE,
  FG_SPEED_FDBK,
  SPEED_REF_OPEN_LOOP,
  SPEED_FDBK,
  COUNT,
};

using RegisterInfo = component_common::RegisterInfo<RegisterId>;
using component_common::RegisterWidth;

inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);

inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::CONTROLLER_FAULT_STATUS, .name = "controller_fault_status", .address = 0x00E2, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::GATE_DRIVER_FAULT_STATUS, .name = "gate_driver_fault_status", .address = 0x00E0, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_STATUS, .name = "algo_status", .address = 0x00E4, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_STATUS_MPET, .name = "algo_status_mpet", .address = 0x00E8, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGORITHM_STATE, .name = "algorithm_state", .address = 0x0196, .width = RegisterWidth::U16, .masks = {.status = 0xFFFF}},
    {.id = RegisterId::MTR_PARAMS, .name = "motor_parameters", .address = 0x00E6, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::ALGO_CTRL1, .name = "algorithm_control_1", .address = 0x00EA, .width = RegisterWidth::U32},
    {.id = RegisterId::ALGO_DEBUG1, .name = "algorithm_debug_1", .address = 0x00EC, .width = RegisterWidth::U32},
    {.id = RegisterId::ALGO_DEBUG2, .name = "algorithm_debug_2", .address = 0x00EE, .width = RegisterWidth::U32},
    {.id = RegisterId::PIN_CONFIG, .name = "pin_config", .address = 0x00A4, .width = RegisterWidth::U32},
    {.id = RegisterId::PERI_CONFIG1, .name = "peripheral_config_1", .address = 0x00AA, .width = RegisterWidth::U32},
    {.id = RegisterId::MOTOR_STARTUP1, .name = "motor_startup_1", .address = 0x0084, .width = RegisterWidth::U32},
    {.id = RegisterId::MOTOR_STARTUP2, .name = "motor_startup_2", .address = 0x0086, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP2, .name = "closed_loop_2", .address = 0x008A, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP3, .name = "closed_loop_3", .address = 0x008C, .width = RegisterWidth::U32},
    {.id = RegisterId::CLOSED_LOOP4, .name = "closed_loop_4", .address = 0x008E, .width = RegisterWidth::U32},
    {.id = RegisterId::GD_CONFIG1, .name = "gate_driver_config_1", .address = 0x00AC, .width = RegisterWidth::U32},
    {.id = RegisterId::GD_CONFIG2, .name = "gate_driver_config_2", .address = 0x00AE, .width = RegisterWidth::U32},
    {.id = RegisterId::FAULT_CONFIG1, .name = "fault_config_1", .address = 0x0090, .width = RegisterWidth::U32},
    {.id = RegisterId::FAULT_CONFIG2, .name = "fault_config_2", .address = 0x0092, .width = RegisterWidth::U32},
    {.id = RegisterId::INT_ALGO_1, .name = "internal_algorithm_1", .address = 0x00A0, .width = RegisterWidth::U32},
    {.id = RegisterId::INT_ALGO_2, .name = "internal_algorithm_2", .address = 0x00A2, .width = RegisterWidth::U32},
    {.id = RegisterId::VM_VOLTAGE, .name = "vm_voltage", .address = 0x045C, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::FG_SPEED_FDBK, .name = "fg_speed_feedback", .address = 0x019C, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::SPEED_REF_OPEN_LOOP, .name = "open_loop_speed_reference", .address = 0x0532, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
    {.id = RegisterId::SPEED_FDBK, .name = "speed_feedback", .address = 0x076E, .width = RegisterWidth::U32, .masks = {.status = 0xFFFFFFFF}},
}};

static_assert(component_common::register_definitions_have_all_ids_once(REGISTER_DEFINITIONS),
              "MCF8329A register definitions must contain every RegisterId exactly once");
static_assert(component_common::register_definitions_have_unique_addresses(REGISTER_DEFINITIONS),
              "MCF8329A register definitions must have unique addresses");

inline constexpr auto REGISTER_INFO =
    component_common::index_register_info_by_id(REGISTER_DEFINITIONS);

constexpr const RegisterInfo &register_info(RegisterId id) {
  return component_common::register_info(REGISTER_INFO, id);
}

constexpr uint16_t register_address(RegisterId id) {
  return register_info(id).address;
}


}  // namespace regs
}  // namespace mcf8329a_core
