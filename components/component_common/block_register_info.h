#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace component_common {

inline constexpr uint16_t VARIABLE_PAYLOAD_SIZE = 0xFFFFu;

template<typename RegisterId> struct BlockRegisterInfo {
  RegisterId id{};
  const char *name{nullptr};
  uint16_t address{0};
  uint16_t write_size{0};
  uint16_t read_size{0};
};

template<typename RegisterId>
constexpr size_t block_register_id_index(RegisterId id) {
  static_assert(std::is_enum_v<RegisterId>, "RegisterId must be an enum");
  return static_cast<size_t>(id);
}

template<typename RegisterId, size_t N>
constexpr bool block_register_definitions_have_all_ids_once(
    const std::array<BlockRegisterInfo<RegisterId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = block_register_id_index(definition.id);
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

template<typename RegisterId, size_t N>
constexpr bool block_register_definitions_have_unique_addresses(
    const std::array<BlockRegisterInfo<RegisterId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].address == definitions[other].address) return false;
    }
  }
  return true;
}

template<typename RegisterId, size_t N>
constexpr std::array<BlockRegisterInfo<RegisterId>, N> index_block_register_info_by_id(
    const std::array<BlockRegisterInfo<RegisterId>, N> &definitions) {
  std::array<BlockRegisterInfo<RegisterId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = block_register_id_index(definition.id);
    if (index < N) indexed[index] = definition;
  }
  return indexed;
}

template<typename RegisterId, size_t N>
constexpr const BlockRegisterInfo<RegisterId> &block_register_info(
    const std::array<BlockRegisterInfo<RegisterId>, N> &indexed, RegisterId id) {
  return indexed[block_register_id_index(id)];
}

constexpr bool payload_size_matches(uint16_t expected, size_t actual) {
  return expected == VARIABLE_PAYLOAD_SIZE || expected == actual;
}

}  // namespace component_common
