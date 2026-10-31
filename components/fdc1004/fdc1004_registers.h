#pragma once

#include <cstdint>

namespace esphome::fdc1004 {

// TI FDC1004 SNOSCY5C, sections 6.5.3 and 6.6.
enum class Register : uint8_t {
  MEAS1_MSB = 0x00,
  CONF_MEAS1 = 0x08,
  FDC_CONF = 0x0C,
  MANUFACTURER_ID = 0xFE,
  DEVICE_ID = 0xFF,
};

static constexpr uint16_t EXPECTED_MANUFACTURER_ID = 0x5449;
static constexpr uint16_t EXPECTED_DEVICE_ID = 0x1004;
static constexpr uint16_t FDC_CONF_SETTINGS_MASK = 0x0DF0;
static constexpr float CAPDAC_STEP_PF = 3.125f;
static constexpr float MEAS_LSB_PF = 1.0f / 524288.0f;  // 2^19

}  // namespace esphome::fdc1004
