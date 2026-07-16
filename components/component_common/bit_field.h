#pragma once

#include <type_traits>

namespace component_common {
namespace detail {

template<typename Storage> constexpr unsigned trailing_zero_count(Storage value) {
  unsigned count = 0;
  while ((value & Storage{1}) == Storage{0}) {
    value = static_cast<Storage>(value >> 1);
    ++count;
  }
  return count;
}

template<typename Storage> constexpr bool is_contiguous_mask(Storage mask) {
  if (mask == Storage{0}) {
    return false;
  }
  const Storage normalized = static_cast<Storage>(mask >> trailing_zero_count(mask));
  return (normalized & static_cast<Storage>(normalized + Storage{1})) == Storage{0};
}

}  // namespace detail

template<typename Storage, Storage Mask> struct RegisterField {
  static_assert(std::is_integral<Storage>::value, "RegisterField storage must be integral");
  static_assert(std::is_unsigned<Storage>::value, "RegisterField storage must be unsigned");
  static_assert(Mask != Storage{0}, "RegisterField mask must not be zero");
  static_assert(detail::is_contiguous_mask(Mask), "RegisterField mask must be contiguous");

  static constexpr unsigned SHIFT = detail::trailing_zero_count(Mask);
  static constexpr Storage VALUE_MASK = static_cast<Storage>(Mask >> SHIFT);

  static constexpr bool fits(Storage value) {
    return (value & static_cast<Storage>(~VALUE_MASK)) == Storage{0};
  }

  static constexpr Storage decode(Storage register_value) {
    return static_cast<Storage>((register_value & Mask) >> SHIFT);
  }

  static constexpr Storage encode(Storage field_value) {
    return static_cast<Storage>((field_value << SHIFT) & Mask);
  }

  static constexpr Storage replace(Storage register_value, Storage field_value) {
    return static_cast<Storage>((register_value & static_cast<Storage>(~Mask)) | encode(field_value));
  }
};

template<typename Storage, typename Value>
constexpr Storage replace_masked(Storage register_value, Storage mask, Value value_bits) {
  static_assert(std::is_integral<Storage>::value, "replace_masked storage must be integral");
  static_assert(std::is_unsigned<Storage>::value, "replace_masked storage must be unsigned");
  static_assert(std::is_integral<Value>::value, "replace_masked value must be integral");
  const Storage stored_value = static_cast<Storage>(value_bits);
  return static_cast<Storage>((register_value & static_cast<Storage>(~mask)) | (stored_value & mask));
}

template<typename Storage> constexpr bool any_set(Storage register_value, Storage mask) {
  static_assert(std::is_integral<Storage>::value, "any_set storage must be integral");
  static_assert(std::is_unsigned<Storage>::value, "any_set storage must be unsigned");
  return (register_value & mask) != Storage{0};
}

}  // namespace component_common
