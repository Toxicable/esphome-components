#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/bit_field.h"
#include "../component_common/register_manifest.h"

namespace bq25756_core {

// Single source of truth for register-level device facts: addresses, widths,
// bit ownership, masks, and fields. Encoding, decoding, physical scaling, and
// typed snapshots remain in protocol.h/.cpp.

enum class RegisterId : uint8_t {
  CHARGE_VOLTAGE_LIMIT,
  CHARGE_CURRENT_LIMIT,
  INPUT_CURRENT_DPM_LIMIT,
  INPUT_VOLTAGE_DPM_LIMIT,
  REVERSE_INPUT_CURRENT_LIMIT,
  REVERSE_INPUT_VOLTAGE_LIMIT,
  PRECHARGE_CURRENT_LIMIT,
  TERMINATION_CURRENT_LIMIT,
  PRECHARGE_TERMINATION_CONTROL,
  TIMER_CONTROL,
  THREE_STAGE_CHARGE_CONTROL,
  CHARGER_CONTROL,
  PIN_CONTROL,
  POWER_PATH_CONTROL,
  MPPT_CONTROL,
  TS_CHARGING_THRESHOLD_CONTROL,
  TS_CHARGING_REGION_CONTROL,
  TS_REVERSE_THRESHOLD_CONTROL,
  REVERSE_UNDERVOLTAGE_CONTROL,
  VAC_MPP_DETECTED,
  CHARGER_STATUS_1,
  CHARGER_STATUS_2,
  CHARGER_STATUS_3,
  FAULT_STATUS,
  CHARGER_FLAG_1,
  CHARGER_FLAG_2,
  FAULT_FLAG,
  CHARGER_MASK_1,
  CHARGER_MASK_2,
  FAULT_MASK,
  ADC_CONTROL,
  ADC_CHANNEL_CONTROL,
  IAC_ADC,
  IBAT_ADC,
  VAC_ADC,
  VBAT_ADC,
  TS_ADC,
  VFB_ADC,
  GATE_DRIVER_STRENGTH_CONTROL,
  GATE_DRIVER_DEAD_TIME_CONTROL,
  PART_INFORMATION,
  REVERSE_BATTERY_DISCHARGE_CURRENT,
  COUNT,
};

enum class RegisterWidth : uint8_t {
  U8 = 1,
  U16 = 2,
};

struct RegisterCatalogEntry {
  uint8_t address;
  RegisterWidth width;
};

static constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);

static constexpr std::array<RegisterCatalogEntry, REGISTER_COUNT> REGISTER_CATALOG{{
    {0x00, RegisterWidth::U16}, {0x02, RegisterWidth::U16},
    {0x06, RegisterWidth::U16}, {0x08, RegisterWidth::U16},
    {0x0A, RegisterWidth::U16}, {0x0C, RegisterWidth::U16},
    {0x10, RegisterWidth::U16}, {0x12, RegisterWidth::U16},
    {0x14, RegisterWidth::U8},  {0x15, RegisterWidth::U8},
    {0x16, RegisterWidth::U8},  {0x17, RegisterWidth::U8},
    {0x18, RegisterWidth::U8},  {0x19, RegisterWidth::U8},
    {0x1A, RegisterWidth::U8},  {0x1B, RegisterWidth::U8},
    {0x1C, RegisterWidth::U8},  {0x1D, RegisterWidth::U8},
    {0x1E, RegisterWidth::U8},  {0x1F, RegisterWidth::U16},
    {0x21, RegisterWidth::U8},  {0x22, RegisterWidth::U8},
    {0x23, RegisterWidth::U8},  {0x24, RegisterWidth::U8},
    {0x25, RegisterWidth::U8},  {0x26, RegisterWidth::U8},
    {0x27, RegisterWidth::U8},  {0x28, RegisterWidth::U8},
    {0x29, RegisterWidth::U8},  {0x2A, RegisterWidth::U8},
    {0x2B, RegisterWidth::U8},  {0x2C, RegisterWidth::U8},
    {0x2D, RegisterWidth::U16}, {0x2F, RegisterWidth::U16},
    {0x31, RegisterWidth::U16}, {0x33, RegisterWidth::U16},
    {0x37, RegisterWidth::U16}, {0x39, RegisterWidth::U16},
    {0x3B, RegisterWidth::U8},  {0x3C, RegisterWidth::U8},
    {0x3D, RegisterWidth::U8},  {0x62, RegisterWidth::U8},
}};

constexpr const RegisterCatalogEntry &register_catalog_entry(RegisterId id) {
  return REGISTER_CATALOG[static_cast<size_t>(id)];
}

constexpr uint8_t register_address(RegisterId id) {
  return register_catalog_entry(id).address;
}

constexpr uint8_t register_width(RegisterId id) {
  return static_cast<uint8_t>(register_catalog_entry(id).width);
}

static constexpr uint8_t REG00_CHARGE_VOLTAGE_LIMIT = register_address(RegisterId::CHARGE_VOLTAGE_LIMIT);
static constexpr uint8_t REG02_CHARGE_CURRENT_LIMIT = register_address(RegisterId::CHARGE_CURRENT_LIMIT);
static constexpr uint8_t REG06_INPUT_CURRENT_DPM_LIMIT = register_address(RegisterId::INPUT_CURRENT_DPM_LIMIT);
static constexpr uint8_t REG08_INPUT_VOLTAGE_DPM_LIMIT = register_address(RegisterId::INPUT_VOLTAGE_DPM_LIMIT);
static constexpr uint8_t REG0A_REVERSE_INPUT_CURRENT_LIMIT = register_address(RegisterId::REVERSE_INPUT_CURRENT_LIMIT);
static constexpr uint8_t REG0C_REVERSE_INPUT_VOLTAGE_LIMIT = register_address(RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT);
static constexpr uint8_t REG10_PRECHARGE_CURRENT_LIMIT = register_address(RegisterId::PRECHARGE_CURRENT_LIMIT);
static constexpr uint8_t REG12_TERMINATION_CURRENT_LIMIT = register_address(RegisterId::TERMINATION_CURRENT_LIMIT);
static constexpr uint8_t REG14_PRECHARGE_TERMINATION_CONTROL = register_address(RegisterId::PRECHARGE_TERMINATION_CONTROL);
static constexpr uint8_t REG15_TIMER_CONTROL = register_address(RegisterId::TIMER_CONTROL);
static constexpr uint8_t REG16_THREE_STAGE_CHARGE_CONTROL = register_address(RegisterId::THREE_STAGE_CHARGE_CONTROL);
static constexpr uint8_t REG17_CHARGER_CONTROL = register_address(RegisterId::CHARGER_CONTROL);
static constexpr uint8_t REG18_PIN_CONTROL = register_address(RegisterId::PIN_CONTROL);
static constexpr uint8_t REG19_POWER_PATH_CONTROL = register_address(RegisterId::POWER_PATH_CONTROL);
static constexpr uint8_t REG1A_MPPT_CONTROL = register_address(RegisterId::MPPT_CONTROL);
static constexpr uint8_t REG1B_TS_CHARGING_THRESHOLD_CONTROL = register_address(RegisterId::TS_CHARGING_THRESHOLD_CONTROL);
static constexpr uint8_t REG1C_TS_CHARGING_REGION_CONTROL = register_address(RegisterId::TS_CHARGING_REGION_CONTROL);
static constexpr uint8_t REG1D_TS_REVERSE_THRESHOLD_CONTROL = register_address(RegisterId::TS_REVERSE_THRESHOLD_CONTROL);
static constexpr uint8_t REG1E_REVERSE_UNDERVOLTAGE_CONTROL = register_address(RegisterId::REVERSE_UNDERVOLTAGE_CONTROL);
static constexpr uint8_t REG1F_VAC_MPP_DETECTED = register_address(RegisterId::VAC_MPP_DETECTED);
static constexpr uint8_t REG21_CHARGER_STATUS_1 = register_address(RegisterId::CHARGER_STATUS_1);
static constexpr uint8_t REG22_CHARGER_STATUS_2 = register_address(RegisterId::CHARGER_STATUS_2);
static constexpr uint8_t REG23_CHARGER_STATUS_3 = register_address(RegisterId::CHARGER_STATUS_3);
static constexpr uint8_t REG24_FAULT_STATUS = register_address(RegisterId::FAULT_STATUS);
static constexpr uint8_t REG25_CHARGER_FLAG_1 = register_address(RegisterId::CHARGER_FLAG_1);
static constexpr uint8_t REG26_CHARGER_FLAG_2 = register_address(RegisterId::CHARGER_FLAG_2);
static constexpr uint8_t REG27_FAULT_FLAG = register_address(RegisterId::FAULT_FLAG);
static constexpr uint8_t REG28_CHARGER_MASK_1 = register_address(RegisterId::CHARGER_MASK_1);
static constexpr uint8_t REG29_CHARGER_MASK_2 = register_address(RegisterId::CHARGER_MASK_2);
static constexpr uint8_t REG2A_FAULT_MASK = register_address(RegisterId::FAULT_MASK);
static constexpr uint8_t REG2B_ADC_CONTROL = register_address(RegisterId::ADC_CONTROL);
static constexpr uint8_t REG2C_ADC_CHANNEL_CONTROL = register_address(RegisterId::ADC_CHANNEL_CONTROL);
static constexpr uint8_t REG2D_IAC_ADC = register_address(RegisterId::IAC_ADC);
static constexpr uint8_t REG2F_IBAT_ADC = register_address(RegisterId::IBAT_ADC);
static constexpr uint8_t REG31_VAC_ADC = register_address(RegisterId::VAC_ADC);
static constexpr uint8_t REG33_VBAT_ADC = register_address(RegisterId::VBAT_ADC);
static constexpr uint8_t REG37_TS_ADC = register_address(RegisterId::TS_ADC);
static constexpr uint8_t REG39_VFB_ADC = register_address(RegisterId::VFB_ADC);
static constexpr uint8_t REG3B_GATE_DRIVER_STRENGTH_CONTROL = register_address(RegisterId::GATE_DRIVER_STRENGTH_CONTROL);
static constexpr uint8_t REG3C_GATE_DRIVER_DEAD_TIME_CONTROL = register_address(RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL);
static constexpr uint8_t REG3D_PART_INFORMATION = register_address(RegisterId::PART_INFORMATION);
static constexpr uint8_t REG62_REVERSE_BATTERY_DISCHARGE_CURRENT = register_address(RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT);

static constexpr uint16_t REG00_VFB_REG_MASK = 0x001F;
static constexpr uint16_t REG02_ICHG_REG_MASK = 0x07FC;
static constexpr uint16_t REG06_IAC_DPM_MASK = 0x07FC;
static constexpr uint16_t REG08_VAC_DPM_MASK = 0x3FFC;

static constexpr uint8_t REG15_WATCHDOG_MASK = 0x30;
static constexpr uint8_t REG17_WD_RST_MASK = 0x20;
static constexpr uint8_t REG17_DIS_CE_PIN_MASK = 0x10;
static constexpr uint8_t REG17_EN_HIZ_MASK = 0x04;
static constexpr uint8_t REG17_EN_CHG_MASK = 0x01;
static constexpr uint8_t REG18_EN_ICHG_PIN_MASK = 0x80;
static constexpr uint8_t REG18_EN_ILIM_HIZ_PIN_MASK = 0x40;
static constexpr uint8_t REG19_EN_PFM_MASK = 0x20;
static constexpr uint8_t REG19_EN_REV_MASK = 0x01;

static constexpr uint8_t REG21_IAC_DPM_STAT_MASK = 0x40;
static constexpr uint8_t REG21_VAC_DPM_STAT_MASK = 0x20;
static constexpr uint8_t REG21_WATCHDOG_STAT_MASK = 0x08;
static constexpr uint8_t REG21_CHARGE_STAT_MASK = 0x07;
static constexpr uint8_t REG22_POWER_GOOD_STAT_MASK = 0x80;
static constexpr uint8_t REG22_TS_STAT_MASK = 0x70;
static constexpr uint8_t REG22_MPPT_STAT_MASK = 0x03;
static constexpr uint8_t REG23_CV_TIMER_STAT_MASK = 0x08;
static constexpr uint8_t REG23_REVERSE_STAT_MASK = 0x04;
static constexpr uint8_t REG24_VAC_UV_FAULT_MASK = 0x80;
static constexpr uint8_t REG24_VAC_OV_FAULT_MASK = 0x40;
static constexpr uint8_t REG24_IBAT_OCP_FAULT_MASK = 0x20;
static constexpr uint8_t REG24_VBAT_OV_FAULT_MASK = 0x10;
static constexpr uint8_t REG24_THERMAL_SHUTDOWN_FAULT_MASK = 0x08;
static constexpr uint8_t REG24_CHARGE_TIMER_FAULT_MASK = 0x04;
static constexpr uint8_t REG24_DRIVER_SUPPLY_FAULT_MASK = 0x02;
static constexpr uint8_t REG24_ACTIVE_FAULT_MASK =
    REG24_VAC_UV_FAULT_MASK | REG24_VAC_OV_FAULT_MASK |
    REG24_IBAT_OCP_FAULT_MASK | REG24_VBAT_OV_FAULT_MASK |
    REG24_THERMAL_SHUTDOWN_FAULT_MASK |
    REG24_CHARGE_TIMER_FAULT_MASK | REG24_DRIVER_SUPPLY_FAULT_MASK;

static constexpr uint8_t REG2B_ADC_EN_MASK = 0x80;
static constexpr uint8_t REG2B_ADC_RATE_MASK = 0x40;
static constexpr uint8_t REG2B_ADC_SAMPLE_MASK = 0x30;
static constexpr uint8_t REG2B_ADC_AVG_MASK = 0x08;
static constexpr uint8_t REG2B_ADC_AVG_INIT_MASK = 0x04;
static constexpr uint8_t REG2B_ADC_OWNED_MASK =
    REG2B_ADC_EN_MASK | REG2B_ADC_RATE_MASK | REG2B_ADC_SAMPLE_MASK |
    REG2B_ADC_AVG_MASK | REG2B_ADC_AVG_INIT_MASK;
static constexpr uint8_t REG2B_ADC_CONTINUOUS_15_BIT = REG2B_ADC_EN_MASK;
static constexpr uint8_t REG2B_ADC_PERSISTENT_MASK =
    REG2B_ADC_OWNED_MASK & static_cast<uint8_t>(~REG2B_ADC_AVG_INIT_MASK);
static constexpr uint8_t REG2C_IAC_ADC_DIS_MASK = 0x80;
static constexpr uint8_t REG2C_IBAT_ADC_DIS_MASK = 0x40;
static constexpr uint8_t REG2C_VAC_ADC_DIS_MASK = 0x20;
static constexpr uint8_t REG2C_VBAT_ADC_DIS_MASK = 0x10;
static constexpr uint8_t REG2C_TS_ADC_DIS_MASK = 0x04;
static constexpr uint8_t REG2C_VFB_ADC_DIS_MASK = 0x02;
static constexpr uint8_t REG2C_ADC_CHANNEL_OWNED_MASK =
    REG2C_IAC_ADC_DIS_MASK | REG2C_IBAT_ADC_DIS_MASK |
    REG2C_VAC_ADC_DIS_MASK | REG2C_VBAT_ADC_DIS_MASK |
    REG2C_TS_ADC_DIS_MASK | REG2C_VFB_ADC_DIS_MASK;

static constexpr uint8_t PART_NUM_MASK = 0x78;
static constexpr uint8_t BQ25756_PART_NUM_BITS = 0x10;

namespace fields {
using Watchdog = component_common::RegisterField<uint8_t, REG15_WATCHDOG_MASK>;
using PartNumber = component_common::RegisterField<uint8_t, PART_NUM_MASK>;
using ChargeVoltageLimit = component_common::RegisterField<uint16_t, REG00_VFB_REG_MASK>;
using ChargeCurrentLimit = component_common::RegisterField<uint16_t, REG02_ICHG_REG_MASK>;
using InputCurrentLimit = component_common::RegisterField<uint16_t, REG06_IAC_DPM_MASK>;
using InputVoltageLimit = component_common::RegisterField<uint16_t, REG08_VAC_DPM_MASK>;
}  // namespace fields

using component_common::RegisterManifestEntry;
using component_common::RegisterMasks;

constexpr RegisterManifestEntry reg(const char *name, RegisterId id,
                                    RegisterMasks masks) {
  return component_common::make_register_manifest_entry(
      name, register_address(id), register_width(id), masks);
}

static constexpr std::array<RegisterManifestEntry, REGISTER_COUNT> REGISTER_MANIFEST{{
    reg("charge_voltage_limit", RegisterId::CHARGE_VOLTAGE_LIMIT,
        {.configuration = 0x001F, .reserved = 0xFFE0}),
    reg("charge_current_limit", RegisterId::CHARGE_CURRENT_LIMIT,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("input_current_dpm_limit", RegisterId::INPUT_CURRENT_DPM_LIMIT,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("input_voltage_dpm_limit", RegisterId::INPUT_VOLTAGE_DPM_LIMIT,
        {.configuration = 0x3FFC, .reserved = 0xC003}),
    reg("reverse_input_current_limit", RegisterId::REVERSE_INPUT_CURRENT_LIMIT,
        {.configuration = 0x07FC, .reserved = 0xF803}),
    reg("reverse_input_voltage_limit", RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT,
        {.configuration = 0x3FFC, .reserved = 0xC003}),
    reg("precharge_current_limit", RegisterId::PRECHARGE_CURRENT_LIMIT,
        {.configuration = 0x03FC, .reserved = 0xFC03}),
    reg("termination_current_limit", RegisterId::TERMINATION_CURRENT_LIMIT,
        {.configuration = 0x03FC, .reserved = 0xFC03}),
    reg("precharge_termination_control", RegisterId::PRECHARGE_TERMINATION_CONTROL,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("timer_control", RegisterId::TIMER_CONTROL,
        {.configuration = 0xFF}),
    reg("three_stage_charge_control", RegisterId::THREE_STAGE_CHARGE_CONTROL,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("charger_control", RegisterId::CHARGER_CONTROL,
        {.configuration = 0xD8, .runtime = 0x07, .command = 0x20}),
    reg("pin_control", RegisterId::PIN_CONTROL,
        {.configuration = 0xFF}),
    reg("power_path_control", RegisterId::POWER_PATH_CONTROL,
        {.configuration = 0x20, .runtime = 0x41, .command = 0x80, .reserved = 0x1E}),
    reg("mppt_control", RegisterId::MPPT_CONTROL,
        {.configuration = 0x67, .command = 0x80, .reserved = 0x18}),
    reg("ts_charging_threshold_control", RegisterId::TS_CHARGING_THRESHOLD_CONTROL,
        {.configuration = 0xFF}),
    reg("ts_charging_region_control", RegisterId::TS_CHARGING_REGION_CONTROL,
        {.configuration = 0x7F, .reserved = 0x80}),
    reg("ts_reverse_threshold_control", RegisterId::TS_REVERSE_THRESHOLD_CONTROL,
        {.configuration = 0xE0, .reserved = 0x1F}),
    reg("reverse_undervoltage_control", RegisterId::REVERSE_UNDERVOLTAGE_CONTROL,
        {.configuration = 0x20, .reserved = 0xDF}),
    reg("vac_mpp_detected", RegisterId::VAC_MPP_DETECTED,
        {.status = 0x3FFC, .reserved = 0xC003}),
    reg("charger_status_1", RegisterId::CHARGER_STATUS_1,
        {.status = 0xEF, .reserved = 0x10}),
    reg("charger_status_2", RegisterId::CHARGER_STATUS_2,
        {.status = 0xF3, .reserved = 0x0C}),
    reg("charger_status_3", RegisterId::CHARGER_STATUS_3,
        {.status = 0x3C, .reserved = 0xC3}),
    reg("fault_status", RegisterId::FAULT_STATUS,
        {.status = 0xFE, .reserved = 0x01}),
    reg("charger_flag_1", RegisterId::CHARGER_FLAG_1,
        {.status = 0xFF}),
    reg("charger_flag_2", RegisterId::CHARGER_FLAG_2,
        {.status = 0xFF}),
    reg("fault_flag", RegisterId::FAULT_FLAG,
        {.status = 0xFF}),
    reg("charger_mask_1", RegisterId::CHARGER_MASK_1,
        {.configuration = 0xEB, .reserved = 0x14}),
    reg("charger_mask_2", RegisterId::CHARGER_MASK_2,
        {.configuration = 0x9B, .reserved = 0x64}),
    reg("fault_mask", RegisterId::FAULT_MASK,
        {.configuration = 0xFE, .reserved = 0x01}),
    reg("adc_control", RegisterId::ADC_CONTROL,
        {.configuration = 0xF8, .command = 0x04, .reserved = 0x03}),
    reg("adc_channel_control", RegisterId::ADC_CHANNEL_CONTROL,
        {.configuration = 0xF6, .reserved = 0x09}),
    reg("iac_adc", RegisterId::IAC_ADC,
        {.status = 0xFFFF}),
    reg("ibat_adc", RegisterId::IBAT_ADC,
        {.status = 0xFFFF}),
    reg("vac_adc", RegisterId::VAC_ADC,
        {.status = 0xFFFF}),
    reg("vbat_adc", RegisterId::VBAT_ADC,
        {.status = 0xFFFF}),
    reg("ts_adc", RegisterId::TS_ADC,
        {.status = 0x03FF, .reserved = 0xFC00}),
    reg("vfb_adc", RegisterId::VFB_ADC,
        {.status = 0x07FF, .reserved = 0xF800}),
    reg("gate_driver_strength_control", RegisterId::GATE_DRIVER_STRENGTH_CONTROL,
        {.configuration = 0xFF}),
    reg("gate_driver_dead_time_control", RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL,
        {.configuration = 0x0F, .reserved = 0xF0}),
    reg("part_information", RegisterId::PART_INFORMATION,
        {.status = 0x7F, .reserved = 0x80}),
    reg("reverse_battery_discharge_current", RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT,
        {.configuration = 0xC2, .reserved = 0x3D}),
}};

constexpr size_t configuration_register_count() {
  size_t count = 0;
  for (const auto &entry : REGISTER_MANIFEST) {
    if (entry.configuration_mask != 0) {
      count++;
    }
  }
  return count;
}

static constexpr size_t CONFIGURATION_REGISTER_COUNT =
    configuration_register_count();

static_assert(component_common::register_manifest_valid(REGISTER_MANIFEST),
              "BQ25756 register manifest must classify every bit exactly once");

}  // namespace bq25756_core
