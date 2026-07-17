#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/bit_field.h"
#include "../component_common/register_manifest.h"

namespace bq25756_core {

// Single source of truth for register-level device facts: identifiers, names,
// addresses, widths, bit ownership, masks, and fields. Encoding, decoding,
// physical scaling, and typed snapshots remain in protocol.h/.cpp.

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

// These definitions may be reordered without changing RegisterId lookup.
static constexpr std::array<RegisterInfo, REGISTER_COUNT> REGISTER_DEFINITIONS{{
    {
        .id = RegisterId::CHARGE_VOLTAGE_LIMIT,
        .name = "charge_voltage_limit",
        .address = 0x00,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x001F, .reserved = 0xFFE0},
    },
    {
        .id = RegisterId::CHARGE_CURRENT_LIMIT,
        .name = "charge_current_limit",
        .address = 0x02,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x07FC, .reserved = 0xF803},
    },
    {
        .id = RegisterId::INPUT_CURRENT_DPM_LIMIT,
        .name = "input_current_dpm_limit",
        .address = 0x06,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x07FC, .reserved = 0xF803},
    },
    {
        .id = RegisterId::INPUT_VOLTAGE_DPM_LIMIT,
        .name = "input_voltage_dpm_limit",
        .address = 0x08,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x3FFC, .reserved = 0xC003},
    },
    {
        .id = RegisterId::REVERSE_INPUT_CURRENT_LIMIT,
        .name = "reverse_input_current_limit",
        .address = 0x0A,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x07FC, .reserved = 0xF803},
    },
    {
        .id = RegisterId::REVERSE_INPUT_VOLTAGE_LIMIT,
        .name = "reverse_input_voltage_limit",
        .address = 0x0C,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x3FFC, .reserved = 0xC003},
    },
    {
        .id = RegisterId::PRECHARGE_CURRENT_LIMIT,
        .name = "precharge_current_limit",
        .address = 0x10,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x03FC, .reserved = 0xFC03},
    },
    {
        .id = RegisterId::TERMINATION_CURRENT_LIMIT,
        .name = "termination_current_limit",
        .address = 0x12,
        .width = RegisterWidth::U16,
        .masks = {.configuration = 0x03FC, .reserved = 0xFC03},
    },
    {
        .id = RegisterId::PRECHARGE_TERMINATION_CONTROL,
        .name = "precharge_termination_control",
        .address = 0x14,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x0F, .reserved = 0xF0},
    },
    {
        .id = RegisterId::TIMER_CONTROL,
        .name = "timer_control",
        .address = 0x15,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xFF},
    },
    {
        .id = RegisterId::THREE_STAGE_CHARGE_CONTROL,
        .name = "three_stage_charge_control",
        .address = 0x16,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x0F, .reserved = 0xF0},
    },
    {
        .id = RegisterId::CHARGER_CONTROL,
        .name = "charger_control",
        .address = 0x17,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xD8, .runtime = 0x07, .command = 0x20},
    },
    {
        .id = RegisterId::PIN_CONTROL,
        .name = "pin_control",
        .address = 0x18,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xFF},
    },
    {
        .id = RegisterId::POWER_PATH_CONTROL,
        .name = "power_path_control",
        .address = 0x19,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x20, .runtime = 0x41, .command = 0x80, .reserved = 0x1E},
    },
    {
        .id = RegisterId::MPPT_CONTROL,
        .name = "mppt_control",
        .address = 0x1A,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x67, .command = 0x80, .reserved = 0x18},
    },
    {
        .id = RegisterId::TS_CHARGING_THRESHOLD_CONTROL,
        .name = "ts_charging_threshold_control",
        .address = 0x1B,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xFF},
    },
    {
        .id = RegisterId::TS_CHARGING_REGION_CONTROL,
        .name = "ts_charging_region_control",
        .address = 0x1C,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x7F, .reserved = 0x80},
    },
    {
        .id = RegisterId::TS_REVERSE_THRESHOLD_CONTROL,
        .name = "ts_reverse_threshold_control",
        .address = 0x1D,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xE0, .reserved = 0x1F},
    },
    {
        .id = RegisterId::REVERSE_UNDERVOLTAGE_CONTROL,
        .name = "reverse_undervoltage_control",
        .address = 0x1E,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x20, .reserved = 0xDF},
    },
    {
        .id = RegisterId::VAC_MPP_DETECTED,
        .name = "vac_mpp_detected",
        .address = 0x1F,
        .width = RegisterWidth::U16,
        .masks = {.status = 0x3FFC, .reserved = 0xC003},
    },
    {
        .id = RegisterId::CHARGER_STATUS_1,
        .name = "charger_status_1",
        .address = 0x21,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xEF, .reserved = 0x10},
    },
    {
        .id = RegisterId::CHARGER_STATUS_2,
        .name = "charger_status_2",
        .address = 0x22,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xF3, .reserved = 0x0C},
    },
    {
        .id = RegisterId::CHARGER_STATUS_3,
        .name = "charger_status_3",
        .address = 0x23,
        .width = RegisterWidth::U8,
        .masks = {.status = 0x3C, .reserved = 0xC3},
    },
    {
        .id = RegisterId::FAULT_STATUS,
        .name = "fault_status",
        .address = 0x24,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xFE, .reserved = 0x01},
    },
    {
        .id = RegisterId::CHARGER_FLAG_1,
        .name = "charger_flag_1",
        .address = 0x25,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xFF},
    },
    {
        .id = RegisterId::CHARGER_FLAG_2,
        .name = "charger_flag_2",
        .address = 0x26,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xFF},
    },
    {
        .id = RegisterId::FAULT_FLAG,
        .name = "fault_flag",
        .address = 0x27,
        .width = RegisterWidth::U8,
        .masks = {.status = 0xFF},
    },
    {
        .id = RegisterId::CHARGER_MASK_1,
        .name = "charger_mask_1",
        .address = 0x28,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xEB, .reserved = 0x14},
    },
    {
        .id = RegisterId::CHARGER_MASK_2,
        .name = "charger_mask_2",
        .address = 0x29,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x9B, .reserved = 0x64},
    },
    {
        .id = RegisterId::FAULT_MASK,
        .name = "fault_mask",
        .address = 0x2A,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xFE, .reserved = 0x01},
    },
    {
        .id = RegisterId::ADC_CONTROL,
        .name = "adc_control",
        .address = 0x2B,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xF8, .command = 0x04, .reserved = 0x03},
    },
    {
        .id = RegisterId::ADC_CHANNEL_CONTROL,
        .name = "adc_channel_control",
        .address = 0x2C,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xF6, .reserved = 0x09},
    },
    {
        .id = RegisterId::IAC_ADC,
        .name = "iac_adc",
        .address = 0x2D,
        .width = RegisterWidth::U16,
        .masks = {.status = 0xFFFF},
    },
    {
        .id = RegisterId::IBAT_ADC,
        .name = "ibat_adc",
        .address = 0x2F,
        .width = RegisterWidth::U16,
        .masks = {.status = 0xFFFF},
    },
    {
        .id = RegisterId::VAC_ADC,
        .name = "vac_adc",
        .address = 0x31,
        .width = RegisterWidth::U16,
        .masks = {.status = 0xFFFF},
    },
    {
        .id = RegisterId::VBAT_ADC,
        .name = "vbat_adc",
        .address = 0x33,
        .width = RegisterWidth::U16,
        .masks = {.status = 0xFFFF},
    },
    {
        .id = RegisterId::TS_ADC,
        .name = "ts_adc",
        .address = 0x37,
        .width = RegisterWidth::U16,
        .masks = {.status = 0x03FF, .reserved = 0xFC00},
    },
    {
        .id = RegisterId::VFB_ADC,
        .name = "vfb_adc",
        .address = 0x39,
        .width = RegisterWidth::U16,
        .masks = {.status = 0x07FF, .reserved = 0xF800},
    },
    {
        .id = RegisterId::GATE_DRIVER_STRENGTH_CONTROL,
        .name = "gate_driver_strength_control",
        .address = 0x3B,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xFF},
    },
    {
        .id = RegisterId::GATE_DRIVER_DEAD_TIME_CONTROL,
        .name = "gate_driver_dead_time_control",
        .address = 0x3C,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0x0F, .reserved = 0xF0},
    },
    {
        .id = RegisterId::PART_INFORMATION,
        .name = "part_information",
        .address = 0x3D,
        .width = RegisterWidth::U8,
        .masks = {.status = 0x7F, .reserved = 0x80},
    },
    {
        .id = RegisterId::REVERSE_BATTERY_DISCHARGE_CURRENT,
        .name = "reverse_battery_discharge_current",
        .address = 0x62,
        .width = RegisterWidth::U8,
        .masks = {.configuration = 0xC2, .reserved = 0x3D},
    },
}};

constexpr bool register_definitions_have_all_ids_once() {
  for (size_t expected = 0; expected < REGISTER_COUNT; expected++) {
    size_t matches = 0;
    for (const auto &info : REGISTER_DEFINITIONS) {
      if (static_cast<size_t>(info.id) >= REGISTER_COUNT) {
        return false;
      }
      if (static_cast<size_t>(info.id) == expected) {
        matches++;
      }
    }
    if (matches != 1) {
      return false;
    }
  }
  return true;
}

constexpr std::array<RegisterInfo, REGISTER_COUNT> make_register_info_by_id() {
  std::array<RegisterInfo, REGISTER_COUNT> result{};
  for (const auto &info : REGISTER_DEFINITIONS) {
    result[static_cast<size_t>(info.id)] = info;
  }
  return result;
}

static constexpr auto REGISTER_INFO = make_register_info_by_id();

constexpr const RegisterInfo &register_info(RegisterId id) {
  return REGISTER_INFO[static_cast<size_t>(id)];
}

constexpr uint8_t register_address(RegisterId id) {
  return register_info(id).address;
}

constexpr uint8_t register_width(RegisterId id) {
  return static_cast<uint8_t>(register_info(id).width);
}

constexpr RegisterManifestEntry make_manifest_entry(const RegisterInfo &info) {
  return component_common::make_register_manifest_entry(
      info.name, info.address, static_cast<uint8_t>(info.width), info.masks);
}

constexpr std::array<RegisterManifestEntry, REGISTER_COUNT> make_register_manifest() {
  std::array<RegisterManifestEntry, REGISTER_COUNT> manifest{};
  for (size_t index = 0; index < REGISTER_COUNT; index++) {
    manifest[index] = make_manifest_entry(REGISTER_INFO[index]);
  }
  return manifest;
}

static constexpr auto REGISTER_MANIFEST = make_register_manifest();

constexpr size_t configuration_register_count() {
  size_t count = 0;
  for (const auto &info : REGISTER_INFO) {
    if (info.masks.configuration != 0) {
      count++;
    }
  }
  return count;
}

static constexpr size_t CONFIGURATION_REGISTER_COUNT =
    configuration_register_count();

static_assert(register_definitions_have_all_ids_once(),
              "BQ25756 register definitions must contain every RegisterId exactly once");
static_assert(component_common::register_manifest_valid(REGISTER_MANIFEST),
              "BQ25756 register definitions must classify every bit exactly once and not overlap");

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

}  // namespace bq25756_core
