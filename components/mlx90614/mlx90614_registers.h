#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/register_info.h"

namespace mlx90614_core {
namespace registers {

using component_common::RegisterWidth;

enum class RegisterId : uint8_t {
  AMBIENT_TEMPERATURE,
  OBJECT1_TEMPERATURE,
  OBJECT2_TEMPERATURE,
  COUNT,
};

using RegisterInfo = component_common::RegisterInfo<RegisterId>;
inline constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);
inline constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::AMBIENT_TEMPERATURE, .name = "ambient_temperature", .address = 0x06,
     .width = RegisterWidth::U16},
    {.id = RegisterId::OBJECT1_TEMPERATURE, .name = "object1_temperature", .address = 0x07,
     .width = RegisterWidth::U16},
    {.id = RegisterId::OBJECT2_TEMPERATURE, .name = "object2_temperature", .address = 0x08,
     .width = RegisterWidth::U16},
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

}  // namespace registers
}  // namespace mlx90614_core
