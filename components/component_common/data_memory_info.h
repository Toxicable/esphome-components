#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "register_info.h"

namespace component_common {

template<typename DataMemoryId> struct DataMemoryInfo {
  DataMemoryId id{};
  const char *name{nullptr};
  uint16_t address{0};
  RegisterWidth width{RegisterWidth::U8};
};

template<typename DataMemoryId>
constexpr size_t data_memory_id_index(DataMemoryId id) {
  static_assert(std::is_enum_v<DataMemoryId>, "DataMemoryId must be an enum");
  return static_cast<size_t>(id);
}

template<typename DataMemoryId, size_t N>
constexpr bool data_memory_definitions_have_all_ids_once(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = data_memory_id_index(definition.id);
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

template<typename DataMemoryId, size_t N>
constexpr bool data_memory_definitions_have_unique_addresses(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].address == definitions[other].address) return false;
    }
  }
  return true;
}

template<typename DataMemoryId, size_t N>
constexpr std::array<DataMemoryInfo<DataMemoryId>, N> index_data_memory_info_by_id(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  std::array<DataMemoryInfo<DataMemoryId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = data_memory_id_index(definition.id);
    if (index < N) indexed[index] = definition;
  }
  return indexed;
}

template<typename DataMemoryId, size_t N>
constexpr const DataMemoryInfo<DataMemoryId> &data_memory_info(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &indexed, DataMemoryId id) {
  return indexed[data_memory_id_index(id)];
}

}  // namespace component_common
