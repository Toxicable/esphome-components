#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bq25756_register_manifest.h"

namespace bq25756_core {

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
    std::array<component_common::RegisterImageEntry, CONFIGURATION_REGISTER_COUNT>;

constexpr const component_common::RegisterManifestEntry &manifest_entry(RegisterId id) {
  return REGISTER_MANIFEST[static_cast<size_t>(id)];
}

constexpr component_common::RegisterImageEntry image_entry(RegisterId id, uint32_t value) {
  const auto &definition = manifest_entry(id);
  return {
      .name = definition.name,
      .address = definition.address,
      .width = definition.width,
      .value = value,
      .mask = definition.configuration_mask,
      .command_mask = definition.command_mask,
  };
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
