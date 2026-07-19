#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/command_info.h"
#include "../component_common/register_info.h"

namespace husb238_core {
namespace registers {

using component_common::PayloadWidth;
using component_common::RegisterWidth;

enum class RegisterId : uint8_t {
  PD_STATUS0,
  PD_STATUS1,
  SOURCE_PDO_5V,
  SOURCE_PDO_9V,
  SOURCE_PDO_12V,
  SOURCE_PDO_15V,
  SOURCE_PDO_18V,
  SOURCE_PDO_20V,
  SELECTED_PDO,
  GO_COMMAND,
  COUNT,
};

enum class CommandId : uint8_t {
  REQUEST_SELECTED_PDO,
  GET_SOURCE_CAPABILITIES,
  HARD_RESET,
  COUNT,
};

using RegisterInfo = component_common::RegisterInfo<RegisterId>;
inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);
inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::PD_STATUS0, .name = "pd_status0", .address = 0x00, .width = RegisterWidth::U8},
    {.id = RegisterId::PD_STATUS1, .name = "pd_status1", .address = 0x01, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_5V, .name = "source_pdo_5v", .address = 0x02, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_9V, .name = "source_pdo_9v", .address = 0x03, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_12V, .name = "source_pdo_12v", .address = 0x04, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_15V, .name = "source_pdo_15v", .address = 0x05, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_18V, .name = "source_pdo_18v", .address = 0x06, .width = RegisterWidth::U8},
    {.id = RegisterId::SOURCE_PDO_20V, .name = "source_pdo_20v", .address = 0x07, .width = RegisterWidth::U8},
    {.id = RegisterId::SELECTED_PDO, .name = "selected_pdo", .address = 0x08, .width = RegisterWidth::U8},
    {.id = RegisterId::GO_COMMAND, .name = "go_command", .address = 0x09, .width = RegisterWidth::U8},
}};
static_assert(component_common::register_definitions_have_all_ids_once(REGISTER_DEFINITIONS));
static_assert(component_common::register_definitions_have_unique_addresses(REGISTER_DEFINITIONS));
inline constexpr auto REGISTER_INFO = component_common::index_register_info_by_id(REGISTER_DEFINITIONS);
constexpr const RegisterInfo &register_info(RegisterId id) {
  return component_common::register_info(REGISTER_INFO, id);
}
constexpr uint8_t register_address(RegisterId id) {
  return static_cast<uint8_t>(register_info(id).address);
}
constexpr RegisterId source_pdo_register(size_t index) {
  return static_cast<RegisterId>(static_cast<size_t>(RegisterId::SOURCE_PDO_5V) + index);
}

using CommandInfo = component_common::CommandInfo<CommandId>;
inline constexpr size_t COMMAND_COUNT = static_cast<size_t>(CommandId::COUNT);
inline constexpr std::array<CommandInfo, COMMAND_COUNT> COMMAND_DEFINITIONS{{
    {.id = CommandId::REQUEST_SELECTED_PDO, .name = "request_selected_pdo", .code = 0x01,
     .request_width = PayloadWidth::NONE, .response_width = PayloadWidth::NONE},
    {.id = CommandId::GET_SOURCE_CAPABILITIES, .name = "get_source_capabilities", .code = 0x04,
     .request_width = PayloadWidth::NONE, .response_width = PayloadWidth::NONE},
    {.id = CommandId::HARD_RESET, .name = "hard_reset", .code = 0x10,
     .request_width = PayloadWidth::NONE, .response_width = PayloadWidth::NONE},
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
}  // namespace husb238_core
