#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace component_common {

template<typename UInt> UInt load_le(const uint8_t *data) {
  static_assert(std::is_integral<UInt>::value, "load_le destination must be integral");
  static_assert(std::is_unsigned<UInt>::value, "load_le destination must be unsigned");

  UInt value = 0;
  for (size_t index = 0; index < sizeof(UInt); ++index) {
    value = static_cast<UInt>(value | (static_cast<UInt>(data[index]) << (index * 8U)));
  }
  return value;
}

template<typename UInt> UInt load_be(const uint8_t *data) {
  static_assert(std::is_integral<UInt>::value, "load_be destination must be integral");
  static_assert(std::is_unsigned<UInt>::value, "load_be destination must be unsigned");

  UInt value = 0;
  for (size_t index = 0; index < sizeof(UInt); ++index) {
    value = static_cast<UInt>((value << 8U) | static_cast<UInt>(data[index]));
  }
  return value;
}

template<typename UInt> void store_le(UInt value, uint8_t *data) {
  static_assert(std::is_integral<UInt>::value, "store_le source must be integral");
  static_assert(std::is_unsigned<UInt>::value, "store_le source must be unsigned");

  for (size_t index = 0; index < sizeof(UInt); ++index) {
    data[index] = static_cast<uint8_t>((value >> (index * 8U)) & static_cast<UInt>(0xFFU));
  }
}

template<typename UInt> void store_be(UInt value, uint8_t *data) {
  static_assert(std::is_integral<UInt>::value, "store_be source must be integral");
  static_assert(std::is_unsigned<UInt>::value, "store_be source must be unsigned");

  for (size_t index = 0; index < sizeof(UInt); ++index) {
    const size_t shift = (sizeof(UInt) - index - 1U) * 8U;
    data[index] = static_cast<uint8_t>((value >> shift) & static_cast<UInt>(0xFFU));
  }
}

}  // namespace component_common
