#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace bq25756_core {

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

static constexpr size_t REGISTER_COUNT = static_cast<size_t>(RegisterId::COUNT);

static constexpr std::array<uint8_t, REGISTER_COUNT> REGISTER_ADDRESSES{{
    0x00, 0x02, 0x06, 0x08, 0x0A, 0x0C, 0x10, 0x12, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x21, 0x22,
    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
    0x2F, 0x31, 0x33, 0x37, 0x39, 0x3B, 0x3C, 0x3D, 0x62,
}};

constexpr uint8_t register_address(RegisterId id) {
  return REGISTER_ADDRESSES[static_cast<size_t>(id)];
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

}  // namespace bq25756_core
