#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace component_common {

enum class OperationWidth : uint8_t {
  NONE = 0,
  U8 = 1,
  U16 = 2,
  U32 = 4,
  VARIABLE = 0xFF,
};

template<typename OperationId> struct OperationInfo {
  OperationId id{};
  const char *name{nullptr};
  uint16_t code{0};
  OperationWidth request_width{OperationWidth::NONE};
  OperationWidth response_width{OperationWidth::NONE};
};

template<typename OperationId>
constexpr size_t operation_id_index(OperationId id) {
  static_assert(std::is_enum_v<OperationId>, "OperationId must be an enum");
  return static_cast<size_t>(id);
}

template<typename OperationId, size_t N>
constexpr bool operation_definitions_have_all_ids_once(
    const std::array<OperationInfo<OperationId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = operation_id_index(definition.id);
    if (index >= N || seen[index] || definition.name == nullptr ||
        definition.name[0] == '\0') {
      return false;
    }
    seen[index] = true;
  }
  for (const bool present : seen) {
    if (!present) {
      return false;
    }
  }
  return true;
}

template<typename OperationId, size_t N>
constexpr bool operation_definitions_have_unique_codes(
    const std::array<OperationInfo<OperationId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].code == definitions[other].code) {
        return false;
      }
    }
  }
  return true;
}

template<typename OperationId, size_t N>
constexpr std::array<OperationInfo<OperationId>, N> index_operation_info_by_id(
    const std::array<OperationInfo<OperationId>, N> &definitions) {
  std::array<OperationInfo<OperationId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = operation_id_index(definition.id);
    if (index < N) {
      indexed[index] = definition;
    }
  }
  return indexed;
}

template<typename OperationId, size_t N>
constexpr const OperationInfo<OperationId> &operation_info(
    const std::array<OperationInfo<OperationId>, N> &indexed,
    OperationId id) {
  return indexed[operation_id_index(id)];
}

}  // namespace component_common
