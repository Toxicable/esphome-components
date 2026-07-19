#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/block_register_info.h"
#include "../component_common/command_info.h"

namespace esc_higher_core {
namespace registers {

using component_common::PayloadWidth;

inline constexpr int32_t COMMAND_WATCHDOG_TIMEOUT_MS = 500;

enum class RegisterId : uint8_t {
  ID,
  STATUS,
  COMMAND,
  TELEMETRY,
  BRINGUP,
  DEBUG_TELEMETRY,
  DEBUG_INFO,
  DEBUG_READ,
  DEBUG_CTRL,
  CONFIG_DATA,
  DIAGNOSTIC_90,
  DIAGNOSTIC_91,
  DIAGNOSTIC_92,
  CONFIG_STATUS,
  BOARD_CONFIG,
  COUNT,
};

enum class CommandId : uint8_t {
  START,
  STOP,
  CLEAR_FAULTS,
  SET_SPEED_RAMP,
  ESTOP,
  SET_WATCHDOG,
  RUN_BRINGUP_TEST,
  CONFIG_BEGIN,
  CONFIG_WRITE_CHUNK,
  CONFIG_VALIDATE,
  CONFIG_COMMIT,
  CONFIG_READ,
  CONFIG_ERASE,
  COUNT,
};

using RegisterInfo = component_common::BlockRegisterInfo<RegisterId>;
inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);
inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::ID, .name = "id", .address = 0x00, .write_size = 0, .read_size = 8},
    {.id = RegisterId::STATUS, .name = "status", .address = 0x10, .write_size = 0, .read_size = 16},
    {.id = RegisterId::COMMAND, .name = "command", .address = 0x20, .write_size = 16, .read_size = 0},
    {.id = RegisterId::TELEMETRY, .name = "telemetry", .address = 0x30, .write_size = 0, .read_size = 48},
    {.id = RegisterId::BRINGUP, .name = "bringup", .address = 0x40, .write_size = 0, .read_size = 64},
    {.id = RegisterId::DEBUG_TELEMETRY, .name = "debug_telemetry", .address = 0x50, .write_size = 0, .read_size = 32},
    {.id = RegisterId::DEBUG_INFO, .name = "debug_info", .address = 0x70, .write_size = 0, .read_size = 16},
    {.id = RegisterId::DEBUG_READ, .name = "debug_read", .address = 0x71, .write_size = 3,
     .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::DEBUG_CTRL, .name = "debug_ctrl", .address = 0x72,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::CONFIG_DATA, .name = "config_data", .address = 0x80,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::DIAGNOSTIC_90, .name = "diagnostic_90", .address = 0x90,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::DIAGNOSTIC_91, .name = "diagnostic_91", .address = 0x91,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::DIAGNOSTIC_92, .name = "diagnostic_92", .address = 0x92,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
    {.id = RegisterId::CONFIG_STATUS, .name = "config_status", .address = 0x93, .write_size = 0, .read_size = 49},
    {.id = RegisterId::BOARD_CONFIG, .name = "board_config", .address = 0xA0,
     .write_size = component_common::VARIABLE_PAYLOAD_SIZE, .read_size = component_common::VARIABLE_PAYLOAD_SIZE},
}};
static_assert(component_common::block_register_definitions_have_all_ids_once(REGISTER_DEFINITIONS));
static_assert(component_common::block_register_definitions_have_unique_addresses(REGISTER_DEFINITIONS));
inline constexpr auto REGISTER_INFO = component_common::index_block_register_info_by_id(REGISTER_DEFINITIONS);
constexpr const RegisterInfo &register_info(RegisterId id) {
  return component_common::block_register_info(REGISTER_INFO, id);
}
constexpr uint8_t register_address(RegisterId id) {
  return static_cast<uint8_t>(register_info(id).address);
}

using CommandInfo = component_common::CommandInfo<CommandId>;
inline constexpr size_t COMMAND_COUNT = static_cast<size_t>(CommandId::COUNT);
inline constexpr std::array<CommandInfo, COMMAND_COUNT> COMMAND_DEFINITIONS{{
    {.id = CommandId::START, .name = "start", .code = 0x01, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::STOP, .name = "stop", .code = 0x02, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CLEAR_FAULTS, .name = "clear_faults", .code = 0x03, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::SET_SPEED_RAMP, .name = "set_speed_ramp", .code = 0x04, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::ESTOP, .name = "estop", .code = 0x05, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::SET_WATCHDOG, .name = "set_watchdog", .code = 0x07, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::RUN_BRINGUP_TEST, .name = "run_bringup_test", .code = 0x09, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_BEGIN, .name = "config_begin", .code = 0x10, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_WRITE_CHUNK, .name = "config_write_chunk", .code = 0x11, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_VALIDATE, .name = "config_validate", .code = 0x12, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_COMMIT, .name = "config_commit", .code = 0x13, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_READ, .name = "config_read", .code = 0x14, .request_width = PayloadWidth::VARIABLE},
    {.id = CommandId::CONFIG_ERASE, .name = "config_erase", .code = 0x15, .request_width = PayloadWidth::VARIABLE},
}};
static_assert(component_common::command_definitions_have_all_ids_once(COMMAND_DEFINITIONS));
static_assert(component_common::command_definitions_have_unique_codes(COMMAND_DEFINITIONS));
inline constexpr auto COMMAND_INFO = component_common::index_command_info_by_id(COMMAND_DEFINITIONS);
constexpr const CommandInfo &command_info(CommandId id) {
  return component_common::command_info(COMMAND_INFO, id);
}
constexpr uint8_t command_code(CommandId id) {
  return static_cast<uint8_t>(command_info(id).code);
}

}  // namespace registers
}  // namespace esc_higher_core
