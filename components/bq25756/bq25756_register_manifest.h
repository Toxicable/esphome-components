#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bq25756_protocol.h"
#include "../component_common/register_manifest.h"

namespace bq25756_core {

using component_common::RegisterImageEntry;
using component_common::RegisterManifestEntry;
using component_common::command_bits;
using component_common::configuration_bits;
using component_common::reserved_bits;
using component_common::runtime_bits;
using component_common::status_bits;

template<typename... Masks>
constexpr RegisterManifestEntry reg(const char *name, RegisterId id, uint8_t width,
                                    Masks... masks) {
  return component_common::make_register_manifest_entry(
      name, register_address(id), width, masks...);
}

static constexpr std::array<RegisterManifestEntry, REGISTER_COUNT> REGISTER_MANIFEST{{
    reg("charge_voltage_limit", RegisterId::CHARGE_VOLTAGE_LIMIT, 2,
        configuration_bits(0x001F), reserved_bits(0xFFE0)),
    reg("charge_current_limit", RegisterId::CHARGE_CURRENT_LIMIT, 2,
        configuration_bits(0x07FC), reserved_bits(0xF803)),
    reg("input_current_dpm_limit", RegisterId::INPUT_CURRENT_DPM_LIMIT, 2,
        configuration_bits(0x07FC), reserved_bits(0xF803)),
    reg("input_voltage_dpm_limit", RegisterId::INPUT_VOLTAGE_DPM_LIMIT, 2,
        configuration_bits(0x3FFC), reserved_bits(0xC003)),
    reg("reverse_input_current_limit", RegisterId::REVERSE_INPUT_CURRENT_LIMIT, 2,
        configuration_bits(0x07FC), reserved_bits(0xF803)),
    reg("reverse_input_voltage_limit", RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT, 2,
        configuration_bits(0x3FFC), reserved_bits(0xC003)),
    reg("precharge_current_limit", RegisterId::PRECHARGE_CURRENT_LIMIT, 2,
        configuration_bits(0x03FC), reserved_bits(0xFC03)),
    reg("termination_current_limit", RegisterId::TERMINATION_CURRENT_LIMIT, 2,
        configuration_bits(0x03FC), reserved_bits(0xFC03)),
    reg("precharge_termination_control", RegisterId::PRECHARGE_TERMINATION_CONTROL, 1,
        configuration_bits(0x0F), reserved_bits(0xF0)),
    reg("timer_control", RegisterId::TIMER_CONTROL, 1,
        configuration_bits(0xFF)),
    reg("three_stage_charge_control", RegisterId::THREE_STAGE_CHARGE_CONTROL, 1,
        configuration_bits(0x0F), reserved_bits(0xF0)),
    reg("charger_control", RegisterId::CHARGER_CONTROL, 1,
        configuration_bits(0xD8), runtime_bits(0x07), command_bits(0x20)),
    reg("pin_control", RegisterId::PIN_CONTROL, 1,
        configuration_bits(0xFF)),
    reg("power_path_control", RegisterId::POWER_PATH_CONTROL, 1,
        configuration_bits(0x20), runtime_bits(0x41), command_bits(0x80),
        reserved_bits(0x1E)),
    reg("mppt_control", RegisterId::MPPT_CONTROL, 1,
        configuration_bits(0x67), command_bits(0x80), reserved_bits(0x18)),
    reg("ts_charging_threshold_control", RegisterId::TS_CHARGING_THRESHOLD_CONTROL, 1,
        configuration_bits(0xFF)),
    reg("ts_charging_region_control", RegisterId::TS_CHARGING_REGION_CONTROL, 1,
        configuration_bits(0x7F), reserved_bits(0x80)),
    reg("ts_reverse_threshold_control", RegisterId::TS_REVERSE_THRESHOLD_CONTROL, 1,
        configuration_bits(0xE0), reserved_bits(0x1F)),
    reg("reverse_undervoltage_control", RegisterId::REVERSE_UNDERVOLTAGE_CONTROL, 1,
        configuration_bits(0x20), reserved_bits(0xDF)),
    reg("vac_mpp_detected", RegisterId::VAC_MPP_DETECTED, 2,
        status_bits(0x3FFC), reserved_bits(0xC003)),
    reg("charger_status_1", RegisterId::CHARGER_STATUS_1, 1,
        status_bits(0xEF), reserved_bits(0x10)),
    reg("charger_status_2", RegisterId::CHARGER_STATUS_2, 1,
        status_bits(0xF3), reserved_bits(0x0C)),
    reg("charger_status_3", RegisterId::CHARGER_STATUS_3, 1,
        status_bits(0x3C), reserved_bits(0xC3)),
    reg("fault_status", RegisterId::FAULT_STATUS, 1,
        status_bits(0xFE), reserved_bits(0x01)),
    reg("charger_flag_1", RegisterId::CHARGER_FLAG_1, 1,
        status_bits(0xFF)),
    reg("charger_flag_2", RegisterId::CHARGER_FLAG_2, 1,
        status_bits(0xFF)),
    reg("fault_flag", RegisterId::FAULT_FLAG, 1,
        status_bits(0xFF)),
    reg("charger_mask_1", RegisterId::CHARGER_MASK_1, 1,
        configuration_bits(0xEB), reserved_bits(0x14)),
    reg("charger_mask_2", RegisterId::CHARGER_MASK_2, 1,
        configuration_bits(0x9B), reserved_bits(0x64)),
    reg("fault_mask", RegisterId::FAULT_MASK, 1,
        configuration_bits(0xFE), reserved_bits(0x01)),
    reg("adc_control", RegisterId::ADC_CONTROL, 1,
        configuration_bits(0xF8), command_bits(0x04), reserved_bits(0x03)),
    reg("adc_channel_control", RegisterId::ADC_CHANNEL_CONTROL, 1,
        configuration_bits(0xF6), reserved_bits(0x09)),
    reg("iac_adc", RegisterId::IAC_ADC, 2,
        status_bits(0xFFFF)),
    reg("ibat_adc", RegisterId::IBAT_ADC, 2,
        status_bits(0xFFFF)),
    reg("vac_adc", RegisterId::VAC_ADC, 2,
        status_bits(0xFFFF)),
    reg("vbat_adc", RegisterId::VBAT_ADC, 2,
        status_bits(0xFFFF)),
    reg("ts_adc", RegisterId::TS_ADC, 2,
        status_bits(0x03FF), reserved_bits(0xFC00)),
    reg("vfb_adc", RegisterId::VFB_ADC, 2,
        status_bits(0x07FF), reserved_bits(0xF800)),
    reg("gate_driver_strength_control", RegisterId::GATE_DRIVER_STRENGTH_CONTROL, 1,
        configuration_bits(0xFF)),
    reg("gate_driver_dead_time_control", RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL, 1,
        configuration_bits(0x0F), reserved_bits(0xF0)),
    reg("part_information", RegisterId::PART_INFORMATION, 1,
        status_bits(0x7F), reserved_bits(0x80)),
    reg("reverse_battery_discharge_current", RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT, 1,
        configuration_bits(0xC2), reserved_bits(0x3D)),
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

struct Bq25756Configuration {
  uint16_t charge_voltage_limit{0x0010};
  uint16_t charge_current_limit{0x0640};
  uint16_t input_current_dpm_limit{0x0640};
  uint16_t input_voltage_dpm_limit{0x0348};
  uint16_t reverse_input_current_limit{0x0640};
  uint16_t reverse_input_voltage_limit{0x03E8};
  uint16_t precharge_current_limit{0x0140};
  uint16_t termination_current_limit{0x00A0};
  uint8_t precharge_termination_control{0x0F};
  uint8_t timer_control{0x1D};
  uint8_t three_stage_charge_control{0x00};
  uint8_t charger_control{0xC8};
  uint8_t pin_control{0xC0};
  uint8_t power_path_control{0x20};
  uint8_t mppt_control{0x20};
  uint8_t ts_charging_threshold_control{0x96};
  uint8_t ts_charging_region_control{0x57};
  uint8_t ts_reverse_threshold_control{0x40};
  uint8_t reverse_undervoltage_control{0x00};
  uint8_t charger_mask_1{0x00};
  uint8_t charger_mask_2{0x00};
  uint8_t fault_mask{0x00};
  uint8_t adc_control{REG2B_ADC_CONTINUOUS_15_BIT};
  uint8_t adc_channel_control{0x00};
  uint8_t gate_driver_strength_control{0x00};
  uint8_t gate_driver_dead_time_control{0x00};
  uint8_t reverse_battery_discharge_current{0x02};
};

static constexpr size_t CONFIGURATION_REGISTER_COUNT = 27;
using Bq25756ConfigurationImage =
    std::array<RegisterImageEntry, CONFIGURATION_REGISTER_COUNT>;

constexpr const RegisterManifestEntry &manifest_entry(RegisterId id) {
  return REGISTER_MANIFEST[static_cast<size_t>(id)];
}

constexpr RegisterImageEntry image_entry(RegisterId id, uint32_t value) {
  const auto &definition = manifest_entry(id);
  return {definition.name, definition.address, definition.width, value,
          definition.configuration_mask, definition.command_mask};
}

constexpr Bq25756ConfigurationImage make_configuration_image(
    const Bq25756Configuration &config) {
  return {{
      image_entry(RegisterId::CHARGE_VOLTAGE_LIMIT, config.charge_voltage_limit),
      image_entry(RegisterId::CHARGE_CURRENT_LIMIT, config.charge_current_limit),
      image_entry(RegisterId::INPUT_CURRENT_DPM_LIMIT, config.input_current_dpm_limit),
      image_entry(RegisterId::INPUT_VOLTAGE_DPM_LIMIT, config.input_voltage_dpm_limit),
      image_entry(RegisterId::REVERSE_INPUT_CURRENT_LIMIT, config.reverse_input_current_limit),
      image_entry(RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT, config.reverse_input_voltage_limit),
      image_entry(RegisterId::PRECHARGE_CURRENT_LIMIT, config.precharge_current_limit),
      image_entry(RegisterId::TERMINATION_CURRENT_LIMIT, config.termination_current_limit),
      image_entry(RegisterId::PRECHARGE_TERMINATION_CONTROL, config.precharge_termination_control),
      image_entry(RegisterId::TIMER_CONTROL, config.timer_control),
      image_entry(RegisterId::THREE_STAGE_CHARGE_CONTROL, config.three_stage_charge_control),
      image_entry(RegisterId::CHARGER_CONTROL, config.charger_control),
      image_entry(RegisterId::PIN_CONTROL, config.pin_control),
      image_entry(RegisterId::POWER_PATH_CONTROL, config.power_path_control),
      image_entry(RegisterId::MPPT_CONTROL, config.mppt_control),
      image_entry(RegisterId::TS_CHARGING_THRESHOLD_CONTROL, config.ts_charging_threshold_control),
      image_entry(RegisterId::TS_CHARGING_REGION_CONTROL, config.ts_charging_region_control),
      image_entry(RegisterId::TS_REVERSE_THRESHOLD_CONTROL, config.ts_reverse_threshold_control),
      image_entry(RegisterId::REVERSE_UNDERVOLTAGE_CONTROL, config.reverse_undervoltage_control),
      image_entry(RegisterId::CHARGER_MASK_1, config.charger_mask_1),
      image_entry(RegisterId::CHARGER_MASK_2, config.charger_mask_2),
      image_entry(RegisterId::FAULT_MASK, config.fault_mask),
      image_entry(RegisterId::ADC_CONTROL, config.adc_control),
      image_entry(RegisterId::ADC_CHANNEL_CONTROL, config.adc_channel_control),
      image_entry(RegisterId::GATE_DRIVER_STRENGTH_CONTROL, config.gate_driver_strength_control),
      image_entry(RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL, config.gate_driver_dead_time_control),
      image_entry(RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT, config.reverse_battery_discharge_current),
  }};
}

static constexpr auto DEFAULT_CONFIGURATION_IMAGE =
    make_configuration_image(Bq25756Configuration{});
static_assert(component_common::configuration_image_layout_complete(
                  REGISTER_MANIFEST, DEFAULT_CONFIGURATION_IMAGE),
              "BQ25756 configuration image must own every configurable register");

}  // namespace bq25756_core
