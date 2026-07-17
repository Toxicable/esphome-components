#pragma once

#include <cstdint>

#include "bq25756_registers.h"
#include "../component_common/charger.h"

namespace bq25756_core {

// Protocol owns interpretation of register values: physical scaling,
// encoders/decoders, typed measurements, status, and snapshots. Raw addresses,
// masks, and fields belong exclusively to bq25756_registers.h.

static constexpr float IAC_CURRENT_LSB_MA = 0.8f;
static constexpr float IBAT_CURRENT_LSB_MA = 2.0f;
static constexpr float VOLTAGE_LSB_MV = 2.0f;
static constexpr float TS_PERCENT_LSB = 0.09765625f;
static constexpr float VBAT_OV_RISING_MULTIPLIER = 1.04f;
static constexpr float VBAT_OV_FALLING_MULTIPLIER = 1.02f;

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
Measurements decode_measurements(const Reg16Value &iac, const Reg16Value &ibat,
                                 const Reg16Value &vac, const Reg16Value &vbat,
                                 const Reg16Value &ts, const Reg16Value &vfb);
float vfb_reg_target_mv(uint16_t reg00_raw);
uint16_t encode_charge_voltage_limit_mv(uint16_t mv);
uint16_t encode_charge_current_limit_ma(uint16_t ma);
uint16_t encode_input_current_dpm_limit_ma(uint16_t ma);
uint16_t encode_input_voltage_dpm_limit_mv(uint16_t mv);

}  // namespace bq25756_core
