#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace component_common {

enum class PayloadWidth : uint8_t {
  NONE = 0,
  U8 = 1,
  U16 = 2,
  U32 = 4,
  VARIABLE = 0xFF,
};

template<typename CommandId> struct CommandInfo {
  CommandId id{};
  const char *name{nullptr};
  uint16_t code{0};
  PayloadWidth request_width{PayloadWidth::NONE};
  PayloadWidth response_width{PayloadWidth::NONE};
};

template<typename CommandId>
constexpr size_t command_id_index(CommandId id) {
  static_assert(std::is_enum_v<CommandId>, "CommandId must be an enum");
  return static_cast<size_t>(id);
}

template<typename CommandId, size_t N>
constexpr bool command_definitions_have_all_ids_once(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = command_id_index(definition.id);
    if (index >= N || seen[index] || definition.name == nullptr || definition.name[0] == '\0') {
      return false;
    }
    seen[index] = true;
  }
  for (const bool present : seen) {
    if (!present) return false;
  }
  return true;
}

template<typename CommandId, size_t N>
constexpr bool command_definitions_have_unique_codes(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].code == definitions[other].code) return false;
    }
  }
  return true;
}

template<typename CommandId, size_t N>
constexpr std::array<CommandInfo<CommandId>, N> index_command_info_by_id(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  std::array<CommandInfo<CommandId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = command_id_index(definition.id);
    if (index < N) indexed[index] = definition;
  }
  return indexed;
}

template<typename CommandId, size_t N>
constexpr const CommandInfo<CommandId> &command_info(
    const std::array<CommandInfo<CommandId>, N> &indexed, CommandId id) {
  return indexed[command_id_index(id)];
}

}  // namespace component_common
