#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "register_manifest.h"

namespace component_common {

enum class RegisterWidth : uint8_t {
  U8 = 1,
  U16 = 2,
  U32 = 4,
};

template<typename RegisterId> struct RegisterInfo {
  RegisterId id{};
  const char *name{nullptr};
  uint16_t address{0};
  RegisterWidth width{RegisterWidth::U8};
  RegisterMasks masks{};
};

template<typename RegisterId>
constexpr size_t register_id_index(RegisterId id) {
  static_assert(std::is_enum_v<RegisterId>, "RegisterId must be an enum");
  return static_cast<size_t>(id);
}

constexpr uint8_t register_width_bytes(RegisterWidth width) {
  return static_cast<uint8_t>(width);
}

template<typename RegisterId, size_t N>
constexpr bool register_definitions_have_all_ids_once(
    const std::array<RegisterInfo<RegisterId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = register_id_index(definition.id);
    if (index >= N || seen[index]) {
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

template<typename RegisterId, size_t N>
constexpr std::array<RegisterInfo<RegisterId>, N> index_register_info_by_id(
    const std::array<RegisterInfo<RegisterId>, N> &definitions) {
  std::array<RegisterInfo<RegisterId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = register_id_index(definition.id);
    if (index < N) {
      indexed[index] = definition;
    }
  }
  return indexed;
}

template<typename RegisterId, size_t N>
constexpr const RegisterInfo<RegisterId> &register_info(
    const std::array<RegisterInfo<RegisterId>, N> &indexed,
    RegisterId id) {
  return indexed[register_id_index(id)];
}

template<typename RegisterId>
constexpr RegisterManifestEntry make_register_manifest_entry(
    const RegisterInfo<RegisterId> &info) {
  return component_common::make_register_manifest_entry(
      info.name, info.address, register_width_bytes(info.width), info.masks);
}

template<typename RegisterId, size_t N>
constexpr std::array<RegisterManifestEntry, N> make_register_manifest(
    const std::array<RegisterInfo<RegisterId>, N> &indexed) {
  std::array<RegisterManifestEntry, N> manifest{};
  for (size_t index = 0; index < N; index++) {
    manifest[index] = make_register_manifest_entry(indexed[index]);
  }
  return manifest;
}

template<typename RegisterId, size_t N>
constexpr size_t configuration_register_count(
    const std::array<RegisterInfo<RegisterId>, N> &indexed) {
  size_t count = 0;
  for (const auto &info : indexed) {
    if (info.masks.configuration != 0) {
      count++;
    }
  }
  return count;
}

}  // namespace component_common
