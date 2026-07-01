#include "bq25756_protocol.h"

namespace bq25756_core {

const char *charge_status_to_string(uint8_t charge_status) {
  switch (charge_status) {
    case 0:
      return "not_charging";
    case 1:
      return "trickle";
    case 2:
      return "precharge";
    case 3:
      return "fast_cc";
    case 4:
      return "taper_cv";
    case 5:
      return "reserved_5";
    case 6:
      return "topoff";
    case 7:
      return "termination_done";
    default:
      return "unknown";
  }
}

const char *ts_status_to_string(uint8_t ts_status) {
  switch (ts_status) {
    case 0:
      return "normal";
    case 1:
      return "warm";
    case 2:
      return "cool";
    case 3:
      return "cold";
    case 4:
      return "hot";
    default:
      return "reserved";
  }
}

const char *mppt_status_to_string(uint8_t mppt_status) {
  switch (mppt_status) {
    case 0:
      return "disabled";
    case 1:
      return "enabled_idle";
    case 2:
      return "sweeping";
    case 3:
      return "max_power_detected";
    default:
      return "unknown";
  }
}

Measurements decode_measurements(const Reg16Value &iac, const Reg16Value &ibat, const Reg16Value &vac,
                                  const Reg16Value &vbat, const Reg16Value &ts, const Reg16Value &vfb) {
  Measurements measurements;
  measurements.iac = iac;
  measurements.ibat = ibat;
  measurements.vac = vac;
  measurements.vbat = vbat;
  measurements.ts = ts;
  measurements.vfb = vfb;
  measurements.iac_ma = static_cast<float>(static_cast<int16_t>(iac.raw_le)) * IAC_CURRENT_LSB_MA;
  measurements.ibat_ma = static_cast<float>(static_cast<int16_t>(ibat.raw_le)) * IBAT_CURRENT_LSB_MA;
  measurements.vac_mv = static_cast<float>(vac.raw_le) * VOLTAGE_LSB_MV;
  measurements.vbat_mv = static_cast<float>(vbat.raw_le) * VOLTAGE_LSB_MV;
  measurements.ts_percent = static_cast<float>(ts.raw_le) * TS_PERCENT_LSB;
  measurements.vfb_mv = static_cast<float>(vfb.raw_le);
  return measurements;
}

float vfb_reg_target_mv(uint16_t reg00_raw) {
  return 1504.0f + static_cast<float>(reg00_raw & REG00_VFB_REG_MASK) * 2.0f;
}

uint16_t encode_charge_voltage_limit_mv(uint16_t mv) {
  const uint16_t code = static_cast<uint16_t>((mv - 1504) / 2);
  return static_cast<uint16_t>(code & REG00_VFB_REG_MASK);
}

uint16_t encode_charge_current_limit_ma(uint16_t ma) {
  const uint16_t code = static_cast<uint16_t>(ma / 50);
  return static_cast<uint16_t>((code << 2) & REG02_ICHG_REG_MASK);
}

uint16_t encode_input_current_dpm_limit_ma(uint16_t ma) {
  const uint16_t code = static_cast<uint16_t>(ma / 50);
  return static_cast<uint16_t>((code << 2) & REG06_IAC_DPM_MASK);
}

uint16_t encode_input_voltage_dpm_limit_mv(uint16_t mv) {
  const uint16_t code = static_cast<uint16_t>(mv / 20);
  return static_cast<uint16_t>((code << 2) & REG08_VAC_DPM_MASK);
}

}  // namespace bq25756_core
