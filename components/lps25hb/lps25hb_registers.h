#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/register_info.h"

namespace lps25hb_core {
namespace registers {

using component_common::RegisterWidth;

enum class RegisterId : uint8_t {
  WHO_AM_I,
  CTRL_REG1,
  CTRL_REG2,
  STATUS,
  PRESS_OUT_XL,
  PRESS_OUT_L,
  PRESS_OUT_H,
  TEMP_OUT_L,
  TEMP_OUT_H,
  COUNT,
};

using RegisterInfo = component_common::RegisterInfo<RegisterId>;
inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);
inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::WHO_AM_I, .name = "who_am_i", .address = 0x0F, .width = RegisterWidth::U8},
    {.id = RegisterId::CTRL_REG1, .name = "ctrl_reg1", .address = 0x20, .width = RegisterWidth::U8},
    {.id = RegisterId::CTRL_REG2, .name = "ctrl_reg2", .address = 0x21, .width = RegisterWidth::U8},
    {.id = RegisterId::STATUS, .name = "status", .address = 0x27, .width = RegisterWidth::U8},
    {.id = RegisterId::PRESS_OUT_XL, .name = "press_out_xl", .address = 0x28, .width = RegisterWidth::U8},
    {.id = RegisterId::PRESS_OUT_L, .name = "press_out_l", .address = 0x29, .width = RegisterWidth::U8},
    {.id = RegisterId::PRESS_OUT_H, .name = "press_out_h", .address = 0x2A, .width = RegisterWidth::U8},
    {.id = RegisterId::TEMP_OUT_L, .name = "temp_out_l", .address = 0x2B, .width = RegisterWidth::U8},
    {.id = RegisterId::TEMP_OUT_H, .name = "temp_out_h", .address = 0x2C, .width = RegisterWidth::U8},
}};
static_assert(component_common::register_definitions_have_all_ids_once(REGISTER_DEFINITIONS));
static_assert(component_common::register_definitions_have_unique_addresses(REGISTER_DEFINITIONS));
inline constexpr auto REGISTER_INFO = component_common::index_register_info_by_id(REGISTER_DEFINITIONS);
constexpr const RegisterInfo &register_info(RegisterId id) {
  return component_common::register_info(REGISTER_INFO, id);
}
constexpr uint8_t register_address(RegisterId id) {
  return static_cast<uint8_t>(register_info(id).address);
}

inline constexpr uint8_t WHO_AM_I_EXPECTED = 0xBD;
inline constexpr uint8_t CTRL1_PD = 0x80;
inline constexpr uint8_t CTRL1_BDU = 0x04;
inline constexpr uint8_t CTRL2_ONE_SHOT = 0x01;
inline constexpr uint8_t STATUS_PRESSURE_READY = 0x02;
inline constexpr uint8_t STATUS_TEMPERATURE_READY = 0x01;

}  // namespace registers
}  // namespace lps25hb_core
