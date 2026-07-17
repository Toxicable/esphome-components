#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bq25756_registers.h"
#include "../component_common/register_manifest.h"

namespace bq25756_core {

// The manifest classifies every bit from the complete register catalog. It does
// not contain desired settings; those live in bq25756_configuration.h.

using component_common::RegisterManifestEntry;
using component_common::RegisterMasks;

constexpr RegisterManifestEntry reg(const char *name, RegisterId id, uint8_t width,
                                    RegisterMasks masks) {
  return component_common::make_register_manifest_entry(
      name, register_address(id), width, masks);
}

static constexpr std::array<RegisterManifestEntry, REGISTER_COUNT> REGISTER_MANIFEST{{
    reg("charge_voltage_limit", RegisterId::CHARGE_VOLTAGE_LIMIT, 2,
        {.configuration = 0x001F, .reserved = 0xFFE0}),
    reg("charge_current_limit", RegisterId::CHARGE_CURRENT_LIMIT, 2,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("input_current_dpm_limit", RegisterId::INPUT_CURRENT_DPM_LIMIT, 2,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("input_voltage_dpm_limit", RegisterId::INPUT_VOLTAGE_DPM_LIMIT, 2,
        {.configuration = 0x3FFC, .reserved = 0xC003}),
    reg("reverse_input_current_limit", RegisterId::REVERSE_INPUT_CURRENT_LIMIT, 2,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("reverse_input_voltage_limit", RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT, 2,
        {.configuration = 0x3FFC, .reserved = 0xC003}),
    reg("precharge_current_limit", RegisterId::PRECHARGE_CURRENT_LIMIT, 2,
        {.configuration = 0x03FC, .reserved = 0xFC03}),
    reg("termination_current_limit", RegisterId::TERMINATION_CURRENT_LIMIT, 2,
        {.configuration = 0x03FC, .reserved = 0xFC03}),
    reg("precharge_termination_control", RegisterId::PRECHARGE_TERMINATION_CONTROL, 1,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("timer_control", RegisterId::TIMER_CONTROL, 1,
        {.configuration = 0xFF}),
    reg("three_stage_charge_control", RegisterId::THREE_STAGE_CHARGE_CONTROL, 1,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("charger_control", RegisterId::CHARGER_CONTROL, 1,
        {.configuration = 0xD8, .runtime = 0x07, .command = 0x20}),
    reg("pin_control", RegisterId::PIN_CONTROL, 1,
        {.configuration = 0xFF}),
    reg("power_path_control", RegisterId::POWER_PATH_CONTROL, 1,
        {.configuration = 0x20, .runtime = 0x41, .command = 0x80, .reserved = 0x1E}),
    reg("mppt_control", RegisterId::MPPT_CONTROL, 1,
        {.configuration = 0x67, .command = 0x80, .reserved = 0x18}),
    reg("ts_charging_threshold_control", RegisterId::TS_CHARGING_THRESHOLD_CONTROL, 1,
        {.configuration = 0xFF}),
    reg("ts_charging_region_control", RegisterId::TS_CHARGING_REGION_CONTROL, 1,
        {.configuration = 0x7F, .reserved = 0x80}),
    reg("ts_reverse_threshold_control", RegisterId::TS_REVERSE_THRESHOLD_CONTROL, 1,
        {.configuration = 0xE0, .reserved = 0x1F}),
    reg("reverse_undervoltage_control", RegisterId::REVERSE_UNDERVOLTAGE_CONTROL, 1,
        {.configuration = 0x20, .reserved = 0xDF}),
    reg("vac_mpp_detected", RegisterId::VAC_MPP_DETECTED, 2,
        {.status = 0x3FFC, .reserved = 0xC003}),
    reg("charger_status_1", RegisterId::CHARGER_STATUS_1, 1,
        {.status = 0xEF, .reserved = 0x10}),
    reg("charger_status_2", RegisterId::CHARGER_STATUS_2, 1,
        {.status = 0xF3, .reserved = 0x0C}),
    reg("charger_status_3", RegisterId::CHARGER_STATUS_3, 1,
        {.status = 0x3C, .reserved = 0xC3}),
    reg("fault_status", RegisterId::FAULT_STATUS, 1,
        {.status = 0xFE, .reserved = 0x01}),
    reg("charger_flag_1", RegisterId::CHARGER_FLAG_1, 1,
        {.status = 0xFF}),
    reg("charger_flag_2", RegisterId::CHARGER_FLAG_2, 1,
        {.status = 0xFF}),
    reg("fault_flag", RegisterId::FAULT_FLAG, 1,
        {.status = 0xFF}),
    reg("charger_mask_1", RegisterId::CHARGER_MASK_1, 1,
        {.configuration = 0xEB, .reserved = 0x14}),
    reg("charger_mask_2", RegisterId::CHARGER_MASK_2, 1,
        {.configuration = 0x9B, .reserved = 0x64}),
    reg("fault_mask", RegisterId::FAULT_MASK, 1,
        {.configuration = 0xFE, .reserved = 0x01}),
    reg("adc_control", RegisterId::ADC_CONTROL, 1,
        {.configuration = 0xF8, .command = 0x04, .reserved = 0x03}),
    reg("adc_channel_control", RegisterId::ADC_CHANNEL_CONTROL, 1,
        {.configuration = 0xF6, .reserved = 0x09}),
    reg("iac_adc", RegisterId::IAC_ADC, 2,
        {.status = 0xFFFF}),
    reg("ibat_adc", RegisterId::IBAT_ADC, 2,
        {.status = 0xFFFF}),
    reg("vac_adc", RegisterId::VAC_ADC, 2,
        {.status = 0xFFFF}),
    reg("vbat_adc", RegisterId::VBAT_ADC, 2,
        {.status = 0xFFFF}),
    reg("ts_adc", RegisterId::TS_ADC, 2,
        {.status = 0x03FF, .reserved = 0xFC00}),
    reg("vfb_adc", RegisterId::VFB_ADC, 2,
        {.status = 0x07FF, .reserved = 0xF800}),
    reg("gate_driver_strength_control", RegisterId::GATE_DRIVER_STRENGTH_CONTROL, 1,
        {.configuration = 0xFF}),
    reg("gate_driver_dead_time_control", RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL, 1,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("part_information", RegisterId::PART_INFORMATION, 1,
        {.status = 0x7F, .reserved = 0x80}),
    reg("reverse_battery_discharge_current", RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT, 1,
        {.configuration = 0xC2, .reserved = 0x3D}),
}};

constexpr bool register_manifest_matches_catalog() {
  for (size_t index = 0; index < REGISTER_COUNT; index++) {
    if (REGISTER_MANIFEST[index].address != REGISTER_ADDRESSES[index]) {
      return false;
    }
  }
  return true;
}

static_assert(component_common::register_manifest_valid(REGISTER_MANIFEST),
              "BQ25756 register manifest must classify every bit exactly once");
static_assert(register_manifest_matches_catalog(),
              "Every catalogued BQ25756 register must have exactly one manifest entry");

}  // namespace bq25756_core
