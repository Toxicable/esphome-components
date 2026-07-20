#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/bit_field.h"
#include "../component_common/register_manifest.h"

namespace bq25628_core {

// Single source of truth for every BQ25628E register in datasheet Table 8-5.
// Addresses not listed here are reserved and must not be accessed.
enum class RegisterId : uint8_t {
  CHARGE_CURRENT_LIMIT,
  CHARGE_VOLTAGE_LIMIT,
  INPUT_CURRENT_LIMIT,
  INPUT_VOLTAGE_LIMIT,
  MINIMAL_SYSTEM_VOLTAGE,
  PRECHARGE_CONTROL,
  TERMINATION_CONTROL,
  CHARGE_CONTROL,
  CHARGE_TIMER_CONTROL,
  CHARGER_CONTROL_0,
  CHARGER_CONTROL_1,
  CHARGER_CONTROL_2,
  CHARGER_CONTROL_3,
  NTC_CONTROL_0,
  NTC_CONTROL_1,
  NTC_CONTROL_2,
  CHARGER_STATUS_0,
  CHARGER_STATUS_1,
  FAULT_STATUS_0,
  CHARGER_FLAG_0,
  CHARGER_FLAG_1,
  FAULT_FLAG_0,
  CHARGER_MASK_0,
  CHARGER_MASK_1,
  FAULT_MASK_0,
  ADC_CONTROL,
  ADC_FUNCTION_DISABLE_0,
  IBUS_ADC,
  IBAT_ADC,
  VBUS_ADC,
  VPMID_ADC,
  VBAT_ADC,
  VSYS_ADC,
  TS_ADC,
  TDIE_ADC,
  PART_INFORMATION,
  COUNT,
};

enum class RegisterWidth : uint8_t { U8 = 1, U16 = 2 };

using component_common::RegisterManifestEntry;
using component_common::RegisterMasks;

struct RegisterInfo {
  RegisterId id{RegisterId::COUNT};
  const char *name{nullptr};
  uint8_t address{0};
  RegisterWidth width{RegisterWidth::U8};
  RegisterMasks masks{};
};

static constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);

// All writable fields are configuration except self-clearing commands. Status
// and ADC fields are read-only telemetry. Reserved bits are explicit so a
// manifest check catches omissions and accidental overlaps.
static constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {.id = RegisterId::CHARGE_CURRENT_LIMIT, .name = "charge_current_limit", .address = 0x02, .width = RegisterWidth::U16, .masks = {.configuration = 0x07E0, .reserved = 0xF81F}},
    {.id = RegisterId::CHARGE_VOLTAGE_LIMIT, .name = "charge_voltage_limit", .address = 0x04, .width = RegisterWidth::U16, .masks = {.configuration = 0x0FF8, .reserved = 0xF007}},
    {.id = RegisterId::INPUT_CURRENT_LIMIT, .name = "input_current_limit", .address = 0x06, .width = RegisterWidth::U16, .masks = {.configuration = 0x0FF0, .reserved = 0xF00F}},
    {.id = RegisterId::INPUT_VOLTAGE_LIMIT, .name = "input_voltage_limit", .address = 0x08, .width = RegisterWidth::U16, .masks = {.configuration = 0x3FE0, .reserved = 0xC01F}},
    {.id = RegisterId::MINIMAL_SYSTEM_VOLTAGE, .name = "minimal_system_voltage", .address = 0x0E, .width = RegisterWidth::U16, .masks = {.configuration = 0x0FC0, .reserved = 0xF03F}},
    {.id = RegisterId::PRECHARGE_CONTROL, .name = "precharge_control", .address = 0x10, .width = RegisterWidth::U8, .masks = {.configuration = 0xF8, .reserved = 0x07}},
    {.id = RegisterId::TERMINATION_CONTROL, .name = "termination_control", .address = 0x12, .width = RegisterWidth::U8, .masks = {.configuration = 0xFC, .reserved = 0x03}},
    {.id = RegisterId::CHARGE_CONTROL, .name = "charge_control", .address = 0x14, .width = RegisterWidth::U8, .masks = {.configuration = 0xFF}},
    {.id = RegisterId::CHARGE_TIMER_CONTROL, .name = "charge_timer_control", .address = 0x15, .width = RegisterWidth::U8, .masks = {.configuration = 0x8F, .reserved = 0x70}},
    {.id = RegisterId::CHARGER_CONTROL_0, .name = "charger_control_0", .address = 0x16, .width = RegisterWidth::U8, .masks = {.configuration = 0xFB, .command = 0x04}},
    {.id = RegisterId::CHARGER_CONTROL_1, .name = "charger_control_1", .address = 0x17, .width = RegisterWidth::U8, .masks = {.configuration = 0x7D, .command = 0x80, .reserved = 0x02}},
    {.id = RegisterId::CHARGER_CONTROL_2, .name = "charger_control_2", .address = 0x18, .width = RegisterWidth::U8, .masks = {.configuration = 0x1F, .reserved = 0xE0}},
    {.id = RegisterId::CHARGER_CONTROL_3, .name = "charger_control_3", .address = 0x19, .width = RegisterWidth::U8, .masks = {.configuration = 0xE7, .reserved = 0x18}},
    {.id = RegisterId::NTC_CONTROL_0, .name = "ntc_control_0", .address = 0x1A, .width = RegisterWidth::U8, .masks = {.configuration = 0x8F, .reserved = 0x70}},
    {.id = RegisterId::NTC_CONTROL_1, .name = "ntc_control_1", .address = 0x1B, .width = RegisterWidth::U8, .masks = {.configuration = 0xFF}},
    {.id = RegisterId::NTC_CONTROL_2, .name = "ntc_control_2", .address = 0x1C, .width = RegisterWidth::U8, .masks = {.configuration = 0x7F, .reserved = 0x80}},
    {.id = RegisterId::CHARGER_STATUS_0, .name = "charger_status_0", .address = 0x1D, .width = RegisterWidth::U8, .masks = {.status = 0x7F, .reserved = 0x80}},
    {.id = RegisterId::CHARGER_STATUS_1, .name = "charger_status_1", .address = 0x1E, .width = RegisterWidth::U8, .masks = {.status = 0x1F, .reserved = 0xE0}},
    {.id = RegisterId::FAULT_STATUS_0, .name = "fault_status_0", .address = 0x1F, .width = RegisterWidth::U8, .masks = {.status = 0xEF, .reserved = 0x10}},
    {.id = RegisterId::CHARGER_FLAG_0, .name = "charger_flag_0", .address = 0x20, .width = RegisterWidth::U8, .masks = {.status = 0x7F, .reserved = 0x80}},
    {.id = RegisterId::CHARGER_FLAG_1, .name = "charger_flag_1", .address = 0x21, .width = RegisterWidth::U8, .masks = {.status = 0x09, .reserved = 0xF6}},
    {.id = RegisterId::FAULT_FLAG_0, .name = "fault_flag_0", .address = 0x22, .width = RegisterWidth::U8, .masks = {.status = 0xE9, .reserved = 0x16}},
    {.id = RegisterId::CHARGER_MASK_0, .name = "charger_mask_0", .address = 0x23, .width = RegisterWidth::U8, .masks = {.configuration = 0x7F, .reserved = 0x80}},
    {.id = RegisterId::CHARGER_MASK_1, .name = "charger_mask_1", .address = 0x24, .width = RegisterWidth::U8, .masks = {.configuration = 0x09, .reserved = 0xF6}},
    {.id = RegisterId::FAULT_MASK_0, .name = "fault_mask_0", .address = 0x25, .width = RegisterWidth::U8, .masks = {.configuration = 0xE9, .reserved = 0x16}},
    {.id = RegisterId::ADC_CONTROL, .name = "adc_control", .address = 0x26, .width = RegisterWidth::U8, .masks = {.configuration = 0xF8, .command = 0x04, .reserved = 0x03}},
    {.id = RegisterId::ADC_FUNCTION_DISABLE_0, .name = "adc_function_disable_0", .address = 0x27, .width = RegisterWidth::U8, .masks = {.configuration = 0xFF}},
    {.id = RegisterId::IBUS_ADC, .name = "ibus_adc", .address = 0x28, .width = RegisterWidth::U16, .masks = {.status = 0xFFFE, .reserved = 0x0001}},
    {.id = RegisterId::IBAT_ADC, .name = "ibat_adc", .address = 0x2A, .width = RegisterWidth::U16, .masks = {.status = 0xFFFE, .reserved = 0x0001}},
    {.id = RegisterId::VBUS_ADC, .name = "vbus_adc", .address = 0x2C, .width = RegisterWidth::U16, .masks = {.status = 0x3FFE, .reserved = 0xC001}},
    {.id = RegisterId::VPMID_ADC, .name = "vpmid_adc", .address = 0x2E, .width = RegisterWidth::U16, .masks = {.status = 0x3FFE, .reserved = 0xC001}},
    {.id = RegisterId::VBAT_ADC, .name = "vbat_adc", .address = 0x30, .width = RegisterWidth::U16, .masks = {.status = 0x1FFE, .reserved = 0xE001}},
    {.id = RegisterId::VSYS_ADC, .name = "vsys_adc", .address = 0x32, .width = RegisterWidth::U16, .masks = {.status = 0x1FFE, .reserved = 0xE001}},
    {.id = RegisterId::TS_ADC, .name = "ts_adc", .address = 0x34, .width = RegisterWidth::U16, .masks = {.status = 0x0FFF, .reserved = 0xF000}},
    {.id = RegisterId::TDIE_ADC, .name = "tdie_adc", .address = 0x36, .width = RegisterWidth::U16, .masks = {.status = 0x0FFF, .reserved = 0xF000}},
    {.id = RegisterId::PART_INFORMATION, .name = "part_information", .address = 0x38, .width = RegisterWidth::U8, .masks = {.status = 0x3F, .reserved = 0xC0}},
}};

constexpr bool register_definitions_have_all_ids_once() {
  for (size_t expected = 0; expected < REGISTER_COUNT; expected++) {
    size_t matches = 0;
    for (const auto &info : REGISTER_DEFINITIONS) {
      if (static_cast<size_t>(info.id) >= REGISTER_COUNT)
        return false;
      if (static_cast<size_t>(info.id) == expected)
        matches++;
    }
    if (matches != 1)
      return false;
  }
  return true;
}

constexpr std::array<RegisterInfo, REGISTER_COUNT> make_register_info_by_id() {
  std::array<RegisterInfo, REGISTER_COUNT> result{};
  for (const auto &info : REGISTER_DEFINITIONS)
    result[static_cast<size_t>(info.id)] = info;
  return result;
}

static constexpr auto REGISTER_INFO = make_register_info_by_id();

constexpr const RegisterInfo &register_info(RegisterId id) { return REGISTER_INFO[static_cast<size_t>(id)]; }
constexpr uint8_t register_address(RegisterId id) { return register_info(id).address; }

constexpr RegisterManifestEntry make_manifest_entry(const RegisterInfo &info) {
  return component_common::make_register_manifest_entry(info.name, info.address,
                                                         static_cast<uint8_t>(info.width), info.masks);
}

constexpr std::array<RegisterManifestEntry, REGISTER_COUNT> make_register_manifest() {
  std::array<RegisterManifestEntry, REGISTER_COUNT> manifest{};
  for (size_t index = 0; index < REGISTER_COUNT; index++)
    manifest[index] = make_manifest_entry(REGISTER_INFO[index]);
  return manifest;
}

static constexpr auto REGISTER_MANIFEST = make_register_manifest();

static_assert(register_definitions_have_all_ids_once(), "BQ25628 register IDs must be complete and unique");
static_assert(component_common::register_manifest_valid(REGISTER_MANIFEST), "BQ25628 register manifest must be complete");

namespace fields {
using AdcEnable = component_common::RegisterField<uint8_t, 0x80>;
using BatteryVoltageAdc = component_common::RegisterField<uint16_t, 0x1FFE>;
using PartNumber = component_common::RegisterField<uint8_t, 0x38>;
}  // namespace fields

}  // namespace bq25628_core
