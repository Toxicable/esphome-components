#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace component_common {

struct RegisterMasks {
  uint32_t configuration{0};
  uint32_t runtime{0};
  uint32_t status{0};
  uint32_t command{0};
  uint32_t reserved{0};
};

struct RegisterManifestEntry {
  const char *name{nullptr};
  uint16_t address{0};
  uint8_t width{0};
  uint32_t configuration_mask{0};
  uint32_t runtime_mask{0};
  uint32_t status_mask{0};
  uint32_t command_mask{0};
  uint32_t reserved_mask{0};
};

constexpr RegisterManifestEntry make_register_manifest_entry(
    const char *name, uint16_t address, uint8_t width, RegisterMasks masks) {
  return {
      .name = name,
      .address = address,
      .width = width,
      .configuration_mask = masks.configuration,
      .runtime_mask = masks.runtime,
      .status_mask = masks.status,
      .command_mask = masks.command,
      .reserved_mask = masks.reserved,
  };
}

struct RegisterImageEntry {
  const char *name{nullptr};
  uint16_t address{0};
  uint8_t width{0};
  uint32_t value{0};
  uint32_t mask{0};
  uint32_t command_mask{0};
};

constexpr uint32_t register_width_mask(uint8_t width) {
  return width == 1 ? 0xFFUL : width == 2 ? 0xFFFFUL : width == 4 ? 0xFFFFFFFFUL : 0UL;
}

constexpr bool register_ranges_overlap(const RegisterManifestEntry &left,
                                       const RegisterManifestEntry &right) {
  const uint32_t left_end = static_cast<uint32_t>(left.address) + left.width;
  const uint32_t right_end = static_cast<uint32_t>(right.address) + right.width;
  return static_cast<uint32_t>(left.address) < right_end &&
         static_cast<uint32_t>(right.address) < left_end;
}

constexpr bool register_manifest_entry_valid(const RegisterManifestEntry &entry) {
  const uint32_t width_mask = register_width_mask(entry.width);
  if (entry.name == nullptr || entry.name[0] == '\0' || width_mask == 0) {
    return false;
  }

  const std::array<uint32_t, 5> masks{{entry.configuration_mask, entry.runtime_mask,
                                       entry.status_mask, entry.command_mask,
                                       entry.reserved_mask}};
  uint32_t classified = 0;
  for (size_t i = 0; i < masks.size(); i++) {
    if ((masks[i] & ~width_mask) != 0 || (classified & masks[i]) != 0) {
      return false;
    }
    classified |= masks[i];
  }
  return classified == width_mask;
}

template<size_t N>
constexpr bool register_manifest_valid(const std::array<RegisterManifestEntry, N> &manifest) {
  for (size_t i = 0; i < N; i++) {
    if (!register_manifest_entry_valid(manifest[i])) {
      return false;
    }
    for (size_t j = i + 1; j < N; j++) {
      if (register_ranges_overlap(manifest[i], manifest[j])) {
        return false;
      }
    }
  }
  return true;
}

constexpr bool register_image_entry_valid(const RegisterImageEntry &entry) {
  const uint32_t width_mask = register_width_mask(entry.width);
  return entry.name != nullptr && entry.name[0] != '\0' && width_mask != 0 &&
         entry.mask != 0 && (entry.mask & ~width_mask) == 0 &&
         (entry.command_mask & ~width_mask) == 0 &&
         (entry.mask & entry.command_mask) == 0;
}

template<size_t M, size_t I>
constexpr bool configuration_image_layout_complete(
    const std::array<RegisterManifestEntry, M> &manifest,
    const std::array<RegisterImageEntry, I> &image) {
  for (size_t image_index = 0; image_index < I; image_index++) {
    const auto &candidate = image[image_index];
    if (!register_image_entry_valid(candidate)) {
      return false;
    }

    size_t matches = 0;
    for (size_t manifest_index = 0; manifest_index < M; manifest_index++) {
      const auto &definition = manifest[manifest_index];
      if (definition.address == candidate.address) {
        matches++;
        if (definition.width != candidate.width ||
            definition.configuration_mask != candidate.mask ||
            definition.command_mask != candidate.command_mask) {
          return false;
        }
      }
    }
    if (matches != 1) {
      return false;
    }

    for (size_t other = image_index + 1; other < I; other++) {
      if (image[other].address == candidate.address) {
        return false;
      }
    }
  }

  for (const auto &definition : manifest) {
    size_t matches = 0;
    for (const auto &candidate : image) {
      if (candidate.address == definition.address) {
        matches++;
      }
    }
    if ((definition.configuration_mask != 0 && matches != 1) ||
        (definition.configuration_mask == 0 && matches != 0)) {
      return false;
    }
  }
  return true;
}

constexpr uint32_t merge_register_value(uint32_t actual, uint32_t desired,
                                        uint32_t mask) {
  return (actual & ~mask) | (desired & mask);
}

constexpr bool register_value_matches(uint32_t actual, uint32_t desired,
                                      uint32_t mask) {
  return (actual & mask) == (desired & mask);
}

constexpr uint32_t FNV1A_OFFSET_BASIS = 2166136261UL;
constexpr uint32_t FNV1A_PRIME = 16777619UL;

constexpr uint32_t fingerprint_append_byte(uint32_t fingerprint, uint8_t value) {
  return (fingerprint ^ value) * FNV1A_PRIME;
}

constexpr uint32_t fingerprint_append_u32(uint32_t fingerprint, uint32_t value) {
  for (uint8_t i = 0; i < 4; i++) {
    fingerprint = fingerprint_append_byte(
        fingerprint, static_cast<uint8_t>((value >> (i * 8U)) & 0xFFU));
  }
  return fingerprint;
}

constexpr uint32_t fingerprint_register_value(uint32_t fingerprint,
                                              const RegisterImageEntry &entry,
                                              uint32_t value) {
  fingerprint = fingerprint_append_byte(fingerprint,
                                        static_cast<uint8_t>(entry.address & 0xFFU));
  fingerprint = fingerprint_append_byte(
      fingerprint, static_cast<uint8_t>((entry.address >> 8U) & 0xFFU));
  fingerprint = fingerprint_append_byte(fingerprint, entry.width);
  fingerprint = fingerprint_append_u32(fingerprint, entry.mask);
  return fingerprint_append_u32(fingerprint, value & entry.mask);
}

template<size_t N>
constexpr uint32_t configuration_fingerprint(
    const std::array<RegisterImageEntry, N> &image) {
  uint32_t fingerprint = FNV1A_OFFSET_BASIS;
  for (const auto &entry : image) {
    fingerprint = fingerprint_register_value(fingerprint, entry, entry.value);
  }
  return fingerprint;
}

}  // namespace component_common
