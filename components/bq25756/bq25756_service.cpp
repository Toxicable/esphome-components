#include "bq25756_service.h"

namespace bq25756_core {

bool Bq25756Service::read_byte(uint8_t reg, uint8_t &value) {
  return this->read_bytes(reg, &value, 1);
}

bool Bq25756Service::read_bytes(uint8_t reg, uint8_t *data, size_t len) {
  return len == 0 || (this->bus_ != nullptr && this->bus_->read_registers(reg, data, len));
}

bool Bq25756Service::write_byte(uint8_t reg, uint8_t value) {
  return this->write_bytes(reg, &value, 1);
}

bool Bq25756Service::write_bytes(uint8_t reg, const uint8_t *data, size_t len) {
  return len == 0 || (this->bus_ != nullptr && this->bus_->write_registers(reg, data, len));
}

bool Bq25756Service::write_u16_le(uint8_t reg, uint16_t value) {
  const uint8_t raw[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  return this->write_bytes(reg, raw, sizeof(raw));
}

bool Bq25756Service::read_u16_le(uint8_t reg, Reg16Value &value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_bytes(reg, raw, sizeof(raw))) {
    return false;
  }
  value.lsb = raw[0];
  value.msb = raw[1];
  value.raw_le = static_cast<uint16_t>(raw[0]) | (static_cast<uint16_t>(raw[1]) << 8);
  return true;
}

bool Bq25756Service::update_register_bits(uint8_t reg, uint8_t mask, uint8_t value_bits) {
  uint8_t current = 0;
  if (!this->read_byte(reg, current)) {
    return false;
  }
  const uint8_t updated = static_cast<uint8_t>((current & ~mask) | (value_bits & mask));
  return updated == current || this->write_byte(reg, updated);
}

bool Bq25756Service::probe(uint8_t &part_info) {
  if (!this->read_byte(REG3D_PART_INFORMATION, part_info)) {
    return false;
  }
  return (part_info & PART_NUM_MASK) == BQ25756_PART_NUM_BITS;
}

bool Bq25756Service::set_charge_enabled(bool enabled) {
  return this->update_register_bits(REG17_CHARGER_CONTROL, REG17_EN_CHG_MASK, enabled ? REG17_EN_CHG_MASK : 0x00);
}

bool Bq25756Service::set_hiz_mode(bool enabled) {
  return this->update_register_bits(REG17_CHARGER_CONTROL, REG17_EN_HIZ_MASK, enabled ? REG17_EN_HIZ_MASK : 0x00);
}

bool Bq25756Service::set_reverse_mode(bool enabled) {
  return this->update_register_bits(REG19_POWER_PATH_CONTROL, REG19_EN_REV_MASK, enabled ? REG19_EN_REV_MASK : 0x00);
}

bool Bq25756Service::set_watchdog_code(uint8_t code) {
  if (code > 0x03) {
    return false;
  }
  return this->update_register_bits(REG15_TIMER_CONTROL, REG15_WATCHDOG_MASK, static_cast<uint8_t>(code << 4));
}

bool Bq25756Service::reset_watchdog() {
  return this->update_register_bits(REG17_CHARGER_CONTROL, REG17_WD_RST_MASK, REG17_WD_RST_MASK);
}

bool Bq25756Service::read_status(Status &status) {
  return this->read_byte(REG21_CHARGER_STATUS_1, status.status1) &&
         this->read_byte(REG22_CHARGER_STATUS_2, status.status2) &&
         this->read_byte(REG23_CHARGER_STATUS_3, status.status3) &&
         this->read_byte(REG24_FAULT_STATUS, status.fault);
}

bool Bq25756Service::read_measurements(Measurements &measurements, bool include_vfb) {
  Reg16Value iac{};
  Reg16Value ibat{};
  Reg16Value vac{};
  Reg16Value vbat{};
  Reg16Value ts{};
  Reg16Value vfb{};
  if (!this->read_u16_le(REG2D_IAC_ADC, iac) || !this->read_u16_le(REG2F_IBAT_ADC, ibat) ||
      !this->read_u16_le(REG31_VAC_ADC, vac) || !this->read_u16_le(REG33_VBAT_ADC, vbat) ||
      !this->read_u16_le(REG37_TS_ADC, ts) || (include_vfb && !this->read_u16_le(REG39_VFB_ADC, vfb))) {
    return false;
  }
  measurements = decode_measurements(iac, ibat, vac, vbat, ts, vfb);
  return true;
}

bool Bq25756Service::read_control_states(ControlStates &states) {
  uint8_t reg15 = 0;
  uint8_t reg17 = 0;
  uint8_t reg19 = 0;
  if (!this->read_byte(REG15_TIMER_CONTROL, reg15) || !this->read_byte(REG17_CHARGER_CONTROL, reg17) ||
      !this->read_byte(REG19_POWER_PATH_CONTROL, reg19)) {
    return false;
  }

  states.charge_enabled = (reg17 & REG17_EN_CHG_MASK) != 0;
  states.hiz_mode = (reg17 & REG17_EN_HIZ_MASK) != 0;
  states.reverse_mode = (reg19 & REG19_EN_REV_MASK) != 0;
  states.watchdog_code = static_cast<uint8_t>((reg15 & REG15_WATCHDOG_MASK) >> 4);
  return true;
}

bool Bq25756Service::ensure_adc_enabled(bool include_vfb, uint8_t &reg2b_old, uint8_t &reg2b_new, uint8_t &reg2c_old,
                                 uint8_t &reg2c_new) {
  if (!this->read_byte(REG2B_ADC_CONTROL, reg2b_old)) {
    return false;
  }

  reg2b_new = static_cast<uint8_t>((reg2b_old | REG2B_ADC_EN_MASK) & ~REG2B_ADC_RATE_MASK);
  if (reg2b_new != reg2b_old && !this->write_byte(REG2B_ADC_CONTROL, reg2b_new)) {
    return false;
  }

  if (!this->read_byte(REG2C_ADC_CHANNEL_CONTROL, reg2c_old)) {
    return false;
  }

  reg2c_new = static_cast<uint8_t>(
    reg2c_old &
    static_cast<uint8_t>(
      ~(REG2C_IAC_ADC_DIS_MASK | REG2C_IBAT_ADC_DIS_MASK | REG2C_VAC_ADC_DIS_MASK | REG2C_VBAT_ADC_DIS_MASK |
        REG2C_TS_ADC_DIS_MASK)
    )
  );
  reg2c_new = include_vfb ? static_cast<uint8_t>(reg2c_new & ~REG2C_VFB_ADC_DIS_MASK)
                          : static_cast<uint8_t>(reg2c_new | REG2C_VFB_ADC_DIS_MASK);

  return reg2c_new == reg2c_old || this->write_byte(REG2C_ADC_CHANNEL_CONTROL, reg2c_new);
}

bool Bq25756Service::apply_limits(bool has_charge_voltage_limit_mv, uint16_t charge_voltage_limit_mv,
                           bool has_charge_current_limit_ma, uint16_t charge_current_limit_ma,
                           bool has_input_current_dpm_limit_ma, uint16_t input_current_dpm_limit_ma,
                           bool has_input_voltage_dpm_limit_mv, uint16_t input_voltage_dpm_limit_mv) {
  if (has_charge_voltage_limit_mv &&
      !this->write_u16_le(REG00_CHARGE_VOLTAGE_LIMIT, encode_charge_voltage_limit_mv(charge_voltage_limit_mv))) {
    return false;
  }
  if (has_charge_current_limit_ma &&
      !this->write_u16_le(REG02_CHARGE_CURRENT_LIMIT, encode_charge_current_limit_ma(charge_current_limit_ma))) {
    return false;
  }
  if (has_input_current_dpm_limit_ma &&
      !this->write_u16_le(REG06_INPUT_CURRENT_DPM_LIMIT, encode_input_current_dpm_limit_ma(input_current_dpm_limit_ma))) {
    return false;
  }
  if (has_input_voltage_dpm_limit_mv &&
      !this->write_u16_le(REG08_INPUT_VOLTAGE_DPM_LIMIT, encode_input_voltage_dpm_limit_mv(input_voltage_dpm_limit_mv))) {
    return false;
  }
  return true;
}

bool Bq25756Service::apply_pin_overrides(bool disable_ce_pin, bool disable_ilim_hiz_pin, bool disable_ichg_pin) {
  if (disable_ce_pin &&
      !this->update_register_bits(REG17_CHARGER_CONTROL, REG17_DIS_CE_PIN_MASK, REG17_DIS_CE_PIN_MASK)) {
    return false;
  }
  if (disable_ilim_hiz_pin &&
      !this->update_register_bits(REG18_PIN_CONTROL, REG18_EN_ILIM_HIZ_PIN_MASK, 0x00)) {
    return false;
  }
  if (disable_ichg_pin && !this->update_register_bits(REG18_PIN_CONTROL, REG18_EN_ICHG_PIN_MASK, 0x00)) {
    return false;
  }
  return true;
}

bool Bq25756Service::read_charge_precheck(ChargePrecheckSnapshot &snapshot) {
  if (!this->read_byte(REG17_CHARGER_CONTROL, snapshot.reg17) ||
      !this->read_byte(REG19_POWER_PATH_CONTROL, snapshot.reg19) || !this->read_status(snapshot.status) ||
      !this->read_measurements(snapshot.measurements, false)) {
    return false;
  }

  snapshot.en_chg = (snapshot.reg17 & REG17_EN_CHG_MASK) != 0;
  snapshot.en_hiz = (snapshot.reg17 & REG17_EN_HIZ_MASK) != 0;
  snapshot.dis_ce_pin = (snapshot.reg17 & REG17_DIS_CE_PIN_MASK) != 0;
  snapshot.en_rev = (snapshot.reg19 & REG19_EN_REV_MASK) != 0;
  return true;
}

}  // namespace bq25756_core
