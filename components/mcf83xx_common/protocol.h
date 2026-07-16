#pragma once

#include <array>
#include <cstdint>

#include "../component_common/byte_order.h"

namespace mcf83xx_common {

enum class RegisterWidth : uint8_t {
  BITS_16 = 0,
  BITS_32,
};

constexpr uint32_t build_control_word(bool is_read, uint16_t offset, RegisterWidth width) {
  const uint32_t data_length = width == RegisterWidth::BITS_32 ? 1U : 0U;
  return ((is_read ? 1U : 0U) << 23U) | (data_length << 20U) |
         (static_cast<uint32_t>(offset) & 0x0FFFU);
}

inline std::array<uint8_t, 3> make_control_frame(bool is_read, uint16_t offset, RegisterWidth width) {
  const uint32_t control_word = build_control_word(is_read, offset, width);
  return {
      static_cast<uint8_t>((control_word >> 16U) & 0xFFU),
      static_cast<uint8_t>((control_word >> 8U) & 0xFFU),
      static_cast<uint8_t>(control_word & 0xFFU),
  };
}

inline std::array<uint8_t, 7> make_write32_frame(uint16_t offset, uint32_t value) {
  const auto control = make_control_frame(false, offset, RegisterWidth::BITS_32);
  std::array<uint8_t, 7> frame{control[0], control[1], control[2], 0, 0, 0, 0};
  component_common::store_le<uint32_t>(value, frame.data() + 3U);
  return frame;
}

inline uint16_t decode_read16(const uint8_t *data) {
  return component_common::load_le<uint16_t>(data);
}

inline uint32_t decode_read32(const uint8_t *data) {
  return component_common::load_le<uint32_t>(data);
}

}  // namespace mcf83xx_common
