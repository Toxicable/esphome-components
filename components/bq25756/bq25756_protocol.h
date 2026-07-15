#pragma once

#include <cstdint>

#include "../component_common/bit_field.h"
#include "../component_common/charger.h"

namespace bq25756_core {

static constexpr uint8_t REG00_CHARGE_VOLTAGE_LIMIT = 0x00;
static constexpr uint8_t REG02_CHARGE_CURRENT_LIMIT = 0x02;
static constexpr uint8_t REG06_INPUT_CURRENT_DPM_LIMIT = 0x06;
static constexpr uint8_t REG08_INPUT_VOLTAGE_DPM_LIMIT = 0x08;
static constexpr uint8_t REG15_TIMER_CONTROL = 0x15;
static constexpr uint8_t REG17_CHARGER_CONTROL = 0x17;
static constexpr uint8_t REG18_PIN_CONTROL = 0x18;
static constexpr uint8_t REG19_POWER_PATH_CONTROL = 0x19;
static constexpr uint8_t REG21_CHARGER_STATUS_1 = 0x21;
static constexpr uint8_t REG22_CHARGER_STATUS_2 = 0x22;
static constexpr uint8_t REG23_CHARGER_STATUS_3 = 0x23;
static constexpr uint8_t REG24_FAULT_STATUS = 0x24;
static constexpr uint8_t REG2B_ADC_CONTROL = 0x2B;
static constexpr uint8_t REG2C_ADC_CHANNEL_CONTROL = 0x2C;
static constexpr uint8_t REG2D_IAC_ADC = 0x2D;
static constexpr uint8_t REG2F_IBAT_ADC = 0x2F;
static constexpr uint8_t REG31_VAC_ADC = 0x31;
static constexpr uint8_t REG33_VBAT_ADC = 0x33;
static constexpr uint8_t REG37_TS_ADC = 0x37;
static constexpr uint8_t REG39_VFB_ADC = 0x39;
static constexpr uint8_t REG3D_PART_INFORMATION = 0x3D;

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
  REG2B_ADC_EN_MASK | REG2B_ADC_RATE_MASK | REG2B_ADC_SAMPLE_MASK | REG2B_ADC_AVG_MASK |
  REG2B_ADC_AVG_INIT_MASK;
static constexpr uint8_t REG2B_ADC_CONTINUOUS_15_BIT = REG2B_ADC_EN_MASK;
static constexpr uint8_t REG2C_IAC_ADC_DIS_MASK = 0x80;
static constexpr uint8_t REG2C_IBAT_ADC_DIS_MASK = 0x40;
static constexpr uint8_t REG2C_VAC_ADC_DIS_MASK = 0x20;
static constexpr uint8_t REG2C_VBAT_ADC_DIS_MASK = 0x10;
static constexpr uint8_t REG2C_TS_ADC_DIS_MASK = 0x04;
static constexpr uint8_t REG2C_VFB_ADC_DIS_MASK = 0x02;
static constexpr uint8_t REG2C_ADC_CHANNEL_OWNED_MASK =
  REG2C_IAC_ADC_DIS_MASK | REG2C_IBAT_ADC_DIS_MASK | REG2C_VAC_ADC_DIS_MASK |
  REG2C_VBAT_ADC_DIS_MASK | REG2C_TS_ADC_DIS_MASK | REG2C_VFB_ADC_DIS_MASK;

static constexpr uint8_t PART_NUM_MASK = 0x78;
static constexpr uint8_t BQ25756_PART_NUM_BITS = 0x10;

static constexpr float IAC_CURRENT_LSB_MA = 0.8f;
static constexpr float IBAT_CURRENT_LSB_MA = 2.0f;
static constexpr float VOLTAGE_LSB_MV = 2.0f;
static constexpr float TS_PERCENT_LSB = 0.09765625f;

static constexpr uint16_t REG00_VFB_REG_MASK = 0x001F;
static constexpr uint16_t REG02_ICHG_REG_MASK = 0x07FC;
static constexpr uint16_t REG06_IAC_DPM_MASK = 0x07FC;
static constexpr uint16_t REG08_VAC_DPM_MASK = 0x3FFC;
static constexpr float VBAT_OV_RISING_MULTIPLIER = 1.04f;
static constexpr float VBAT_OV_FALLING_MULTIPLIER = 1.02f;

namespace fields {
using Watchdog = component_common::RegisterField<uint8_t, REG15_WATCHDOG_MASK>;
using PartNumber = component_common::RegisterField<uint8_t, PART_NUM_MASK>;
using ChargeVoltageLimit = component_common::RegisterField<uint16_t, REG00_VFB_REG_MASK>;
using ChargeCurrentLimit = component_common::RegisterField<uint16_t, REG02_ICHG_REG_MASK>;
using InputCurrentLimit = component_common::RegisterField<uint16_t, REG06_IAC_DPM_MASK>;
using InputVoltageLimit = component_common::RegisterField<uint16_t, REG08_VAC_DPM_MASK>;
}  // namespace fields

struct Reg16Value {
  uint16_t raw_le{0};
  uint8_t lsb{0};
  uint8_t msb{0};
};

struct Status {
  uint8_t status1{0};
  uint8_t status2{0};
  uint8_t status3{0};
  uint8_t fault{0};
};

struct Measurements {
  Reg16Value iac{};
  Reg16Value ibat{};
  Reg16Value vac{};
  Reg16Value vbat{};
  Reg16Value ts{};
  Reg16Value vfb{};
  float iac_ma{0.0f};
  float ibat_ma{0.0f};
  float vac_mv{0.0f};
  float vbat_mv{0.0f};
  float ts_percent{0.0f};
  float vfb_mv{0.0f};
};

struct ControlStates {
  bool charge_enabled{false};
  bool hiz_mode{false};
  bool reverse_mode{false};
  uint8_t watchdog_code{0};
};

struct ChargePrecheckSnapshot {
  uint8_t reg17{0};
  uint8_t reg19{0};
  Status status{};
  Measurements measurements{};
  bool en_chg{false};
  bool en_hiz{false};
  bool dis_ce_pin{false};
  bool en_rev{false};
};

const char *charge_status_to_string(uint8_t charge_status);
const char *ts_status_to_string(uint8_t ts_status);
const char *mppt_status_to_string(uint8_t mppt_status);
::component_common::ChargerState decode_charger_state(uint8_t status1);
bool charger_fault_active(const Status &status);
::component_common::ChargerSnapshot make_charger_snapshot(
    const Status &status, const Measurements &measurements,
    const ControlStates &controls, uint32_t sequence, uint32_t timestamp_ms);
Measurements decode_measurements(const Reg16Value &iac, const Reg16Value &ibat, const Reg16Value &vac,
                                 const Reg16Value &vbat, const Reg16Value &ts, const Reg16Value &vfb);
float vfb_reg_target_mv(uint16_t reg00_raw);
uint16_t encode_charge_voltage_limit_mv(uint16_t mv);
uint16_t encode_charge_current_limit_ma(uint16_t ma);
uint16_t encode_input_current_dpm_limit_ma(uint16_t ma);
uint16_t encode_input_voltage_dpm_limit_mv(uint16_t mv);

}  // namespace bq25756_core
