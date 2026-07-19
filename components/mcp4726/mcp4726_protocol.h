#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/command_info.h"

namespace mcp4726_core {

using component_common::PayloadWidth;

enum class CommandId : uint8_t {
  WRITE_VOLATILE_MEMORY,
  COUNT,
};

using CommandInfo = component_common::CommandInfo<CommandId>;
inline constexpr size_t COMMAND_COUNT = static_cast<size_t>(CommandId::COUNT);
inline constexpr std::array<CommandInfo, COMMAND_COUNT> COMMAND_DEFINITIONS{{
    {.id = CommandId::WRITE_VOLATILE_MEMORY, .name = "write_volatile_memory", .code = 0x40,
     .request_width = PayloadWidth::U16, .response_width = PayloadWidth::NONE},
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

struct VolatileWriteConfig {
  uint8_t vref{0};
  uint8_t power_down{0};
  uint8_t gain{0};
};

constexpr std::array<uint8_t, 3> encode_volatile_write(VolatileWriteConfig config, uint16_t code) {
  if (code > 4095u) code = 4095u;
  const uint16_t data = static_cast<uint16_t>(code << 4);
  return {{
      static_cast<uint8_t>(command_code(CommandId::WRITE_VOLATILE_MEMORY) |
                           ((config.vref & 0x03u) << 3) |
                           ((config.power_down & 0x03u) << 1) |
                           (config.gain & 0x01u)),
      static_cast<uint8_t>(data >> 8),
      static_cast<uint8_t>(data & 0xFFu),
  }};
}

}  // namespace mcp4726_core
