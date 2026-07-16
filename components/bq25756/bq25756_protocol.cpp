#include "bq25756_protocol.h"

#include <cmath>

namespace bq25756_core {


::component_common::ChargerState decode_charger_state(uint8_t status1) {
  using State = ::component_common::ChargerState;
  switch (status1 & REG21_CHARGE_STAT_MASK) {
    case 0: return State::NOT_CHARGING;
    case 1: return State::TRICKLE;
    case 2: return State::PRECHARGE;
    case 3: return State::FAST_CC;
    case 4: return State::TAPER_CV;
    case 6: return State::TOPOFF;
    case 7: return State::TERMINATION_DONE;
    default: return State::UNKNOWN;
  }
}

bool charger_fault_active(const Status &status) {
  return (status.status1 & REG21_WATCHDOG_STAT_MASK) != 0 ||
         (status.status3 & REG23_REVERSE_STAT_MASK) != 0 ||
         (status.status3 & REG23_CV_TIMER_STAT_MASK) != 0 ||
         (status.fault & REG24_ACTIVE_FAULT_MASK) != 0;
}

::component_common::ChargerSnapshot make_charger_snapshot(
    const Status &status, const Measurements &measurements,
    const ControlStates &controls, uint32_t sequence, uint32_t timestamp_ms) {
  ::component_common::ChargerSnapshot snapshot{};
  snapshot.sequence = sequence;
  snapshot.timestamp_ms = timestamp_ms;
  snapshot.current_a = measurements.ibat_ma / 1000.0f;
  snapshot.voltage_v = measurements.vbat_mv / 1000.0f;
  snapshot.state = decode_charger_state(status.status1);
  snapshot.status_flags = static_cast<uint32_t>(status.status1) |
                          (static_cast<uint32_t>(status.status2) << 8) |
                          (static_cast<uint32_t>(status.status3) << 16);
  snapshot.fault_flags = status.fault;
  snapshot.enabled = controls.charge_enabled;
  snapshot.power_good =
      (status.status2 & REG22_POWER_GOOD_STAT_MASK) != 0;
  snapshot.fault_active = charger_fault_active(status);
  snapshot.valid = std::isfinite(snapshot.current_a) &&
                   std::isfinite(snapshot.voltage_v);
  return snapshot;
}

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
  return 1504.0f + static_cast<float>(fields::ChargeVoltageLimit::decode(reg00_raw)) * 2.0f;
}

uint16_t encode_charge_voltage_limit_mv(uint16_t mv) {
  const uint16_t code = static_cast<uint16_t>((mv - 1504) / 2);
  return fields::ChargeVoltageLimit::encode(code);
}

uint16_t encode_charge_current_limit_ma(uint16_t ma) {
  const uint16_t code = static_cast<uint16_t>(ma / 50);
  return fields::ChargeCurrentLimit::encode(code);
}

uint16_t encode_input_current_dpm_limit_ma(uint16_t ma) {
  const uint16_t code = static_cast<uint16_t>(ma / 50);
  return fields::InputCurrentLimit::encode(code);
}

uint16_t encode_input_voltage_dpm_limit_mv(uint16_t mv) {
  const uint16_t code = static_cast<uint16_t>(mv / 20);
  return fields::InputVoltageLimit::encode(code);
}

}  // namespace bq25756_core
