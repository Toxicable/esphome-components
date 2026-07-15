#include "bq25756_service.h"

namespace bq25756_core {

bool Bq25756Service::read_byte(uint8_t reg, uint8_t& value) {
  return this->read_bytes(reg, &value, 1);
}

bool Bq25756Service::read_bytes(uint8_t reg, uint8_t* data, size_t len) {
  return len == 0 || (this->bus_ != nullptr && this->bus_->read_registers(reg, data, len));
}

bool Bq25756Service::write_byte(uint8_t reg, uint8_t value) {
  return this->write_bytes(reg, &value, 1);
}

bool Bq25756Service::write_bytes(uint8_t reg, const uint8_t* data, size_t len) {
  return len == 0 || (this->bus_ != nullptr && this->bus_->write_registers(reg, data, len));
}

bool Bq25756Service::write_u16_le(uint8_t reg, uint16_t value) {
  const uint8_t raw[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  return this->write_bytes(reg, raw, sizeof(raw));
}

bool Bq25756Service::read_u16_le(uint8_t reg, Reg16Value& value) {
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

bool Bq25756Service::probe(uint8_t& part_info) {
  if (!this->read_byte(REG3D_PART_INFORMATION, part_info)) {
    return false;
  }
  return (part_info & PART_NUM_MASK) == BQ25756_PART_NUM_BITS;
}

bool Bq25756Service::set_charge_enabled(bool enabled) {
  return this->update_register_bits(
    REG17_CHARGER_CONTROL, REG17_EN_CHG_MASK, enabled ? REG17_EN_CHG_MASK : 0x00
  );
}

bool Bq25756Service::set_hiz_mode(bool enabled) {
  return this->update_register_bits(
    REG17_CHARGER_CONTROL, REG17_EN_HIZ_MASK, enabled ? REG17_EN_HIZ_MASK : 0x00
  );
}

bool Bq25756Service::set_pfm_enabled(bool enabled) {
  return this->update_register_bits(
    REG19_POWER_PATH_CONTROL, REG19_EN_PFM_MASK, enabled ? REG19_EN_PFM_MASK : 0x00
  );
}

bool Bq25756Service::set_reverse_mode(bool enabled) {
  return this->update_register_bits(
    REG19_POWER_PATH_CONTROL, REG19_EN_REV_MASK, enabled ? REG19_EN_REV_MASK : 0x00
  );
}

bool Bq25756Service::set_watchdog_code(uint8_t code) {
  if (code > 0x03) {
    return false;
  }
  return this->update_register_bits(
    REG15_TIMER_CONTROL, REG15_WATCHDOG_MASK, static_cast<uint8_t>(code << 4)
  );
}

bool Bq25756Service::reset_watchdog() {
  return this->update_register_bits(REG17_CHARGER_CONTROL, REG17_WD_RST_MASK, REG17_WD_RST_MASK);
}

bool Bq25756Service::read_status(Status& status) {
  return this->read_byte(REG21_CHARGER_STATUS_1, status.status1) &&
         this->read_byte(REG22_CHARGER_STATUS_2, status.status2) &&
         this->read_byte(REG23_CHARGER_STATUS_3, status.status3) &&
         this->read_byte(REG24_FAULT_STATUS, status.fault);
}

MeasurementReadResult Bq25756Service::read_measurements(
  Measurements& measurements, bool include_vfb, uint8_t requested_adc_config, AdcConfigurationState& adc_state
) {
  const AdcEnsureResult adc_result =
    this->ensure_adc_enabled(include_vfb, requested_adc_config, adc_state);
  if (adc_result == AdcEnsureResult::REPAIRED) {
    return MeasurementReadResult::CONFIGURATION_CHANGED;
  }
  if (adc_result == AdcEnsureResult::IO_ERROR) {
    return MeasurementReadResult::IO_ERROR;
  }
  if (adc_result == AdcEnsureResult::VERIFICATION_MISMATCH) {
    return MeasurementReadResult::CONFIGURATION_VERIFY_MISMATCH;
  }

  Reg16Value iac{};
  Reg16Value ibat{};
  Reg16Value vac{};
  Reg16Value vbat{};
  Reg16Value ts{};
  Reg16Value vfb{};
  if (!this->read_u16_le(REG2D_IAC_ADC, iac) || !this->read_u16_le(REG2F_IBAT_ADC, ibat) ||
      !this->read_u16_le(REG31_VAC_ADC, vac) || !this->read_u16_le(REG33_VBAT_ADC, vbat) ||
      !this->read_u16_le(REG37_TS_ADC, ts) || (include_vfb && !this->read_u16_le(REG39_VFB_ADC, vfb))) {
    return MeasurementReadResult::IO_ERROR;
  }
  measurements = decode_measurements(iac, ibat, vac, vbat, ts, vfb);
  return MeasurementReadResult::OK;
}

bool Bq25756Service::read_control_states(ControlStates& states) {
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

AdcEnsureResult Bq25756Service::ensure_adc_enabled(
  bool include_vfb, uint8_t requested_adc_config, AdcConfigurationState& adc_state
) {
  uint8_t current[2] = {0, 0};
  if (!this->read_bytes(REG2B_ADC_CONTROL, current, sizeof(current))) {
    return AdcEnsureResult::IO_ERROR;
  }

  adc_state.old_reg2b = current[0];
  adc_state.old_reg2c = current[1];

  adc_state.persistent_reg2b = static_cast<uint8_t>(requested_adc_config & REG2B_ADC_PERSISTENT_MASK);
  adc_state.requested_reg2c = static_cast<uint8_t>(adc_state.old_reg2c & ~REG2C_ADC_CHANNEL_OWNED_MASK);

  if (!include_vfb) {
    adc_state.requested_reg2c =
      static_cast<uint8_t>(adc_state.requested_reg2c | REG2C_VFB_ADC_DIS_MASK);
  }

  const bool reg2b_changed =
    (adc_state.old_reg2b & REG2B_ADC_PERSISTENT_MASK) != adc_state.persistent_reg2b;
  const bool reg2c_changed = adc_state.requested_reg2c != adc_state.old_reg2c;
  if (!reg2b_changed && !reg2c_changed) {
    adc_state.transient_reg2b = adc_state.persistent_reg2b;
    return AdcEnsureResult::OK;
  }

  adc_state.transient_reg2b = adc_state.persistent_reg2b;
  if ((adc_state.persistent_reg2b & REG2B_ADC_AVG_MASK) != 0) {
    adc_state.transient_reg2b =
      static_cast<uint8_t>(adc_state.transient_reg2b | REG2B_ADC_AVG_INIT_MASK);
  }
  if (!this->write_byte(REG2B_ADC_CONTROL, adc_state.transient_reg2b) ||
      (reg2c_changed && !this->write_byte(REG2C_ADC_CHANNEL_CONTROL, adc_state.requested_reg2c))) {
    return AdcEnsureResult::IO_ERROR;
  }

  uint8_t verify[2] = {0, 0};
  if (!this->read_bytes(REG2B_ADC_CONTROL, verify, sizeof(verify))) {
    return AdcEnsureResult::IO_ERROR;
  }

  if ((verify[0] & REG2B_ADC_PERSISTENT_MASK) != adc_state.persistent_reg2b ||
      verify[1] != adc_state.requested_reg2c) {
    return AdcEnsureResult::VERIFICATION_MISMATCH;
  }
  return AdcEnsureResult::REPAIRED;
}

bool Bq25756Service::apply_limits(
  bool has_charge_voltage_limit_mv,
  uint16_t charge_voltage_limit_mv,
  bool has_charge_current_limit_ma,
  uint16_t charge_current_limit_ma,
  bool has_input_current_dpm_limit_ma,
  uint16_t input_current_dpm_limit_ma,
  bool has_input_voltage_dpm_limit_mv,
  uint16_t input_voltage_dpm_limit_mv
) {
  if (has_charge_voltage_limit_mv && !this->write_u16_le(REG00_CHARGE_VOLTAGE_LIMIT, encode_charge_voltage_limit_mv(charge_voltage_limit_mv))) {
    return false;
  }
  if (has_charge_current_limit_ma && !this->write_u16_le(REG02_CHARGE_CURRENT_LIMIT, encode_charge_current_limit_ma(charge_current_limit_ma))) {
    return false;
  }
  if (has_input_current_dpm_limit_ma && !this->write_u16_le(REG06_INPUT_CURRENT_DPM_LIMIT, encode_input_current_dpm_limit_ma(input_current_dpm_limit_ma))) {
    return false;
  }
  if (has_input_voltage_dpm_limit_mv && !this->write_u16_le(REG08_INPUT_VOLTAGE_DPM_LIMIT, encode_input_voltage_dpm_limit_mv(input_voltage_dpm_limit_mv))) {
    return false;
  }
  return true;
}

bool Bq25756Service::apply_pin_overrides(
  bool disable_ce_pin, bool disable_ilim_hiz_pin, bool disable_ichg_pin
) {
  if (disable_ce_pin && !this->update_register_bits(REG17_CHARGER_CONTROL, REG17_DIS_CE_PIN_MASK, REG17_DIS_CE_PIN_MASK)) {
    return false;
  }
  if (disable_ilim_hiz_pin && !this->update_register_bits(REG18_PIN_CONTROL, REG18_EN_ILIM_HIZ_PIN_MASK, 0x00)) {
    return false;
  }
  if (disable_ichg_pin && !this->update_register_bits(REG18_PIN_CONTROL, REG18_EN_ICHG_PIN_MASK, 0x00)) {
    return false;
  }
  return true;
}

bool Bq25756Service::read_charge_precheck(ChargePrecheckSnapshot& snapshot) {
  AdcConfigurationState adc_state{};
  if (!this->read_byte(REG17_CHARGER_CONTROL, snapshot.reg17) ||
      !this->read_byte(REG19_POWER_PATH_CONTROL, snapshot.reg19) || !this->read_status(snapshot.status) ||
      this->read_measurements(snapshot.measurements, false, REG2B_ADC_CONTINUOUS_15_BIT,
                              adc_state) != MeasurementReadResult::OK) {
    return false;
  }

  snapshot.en_chg = (snapshot.reg17 & REG17_EN_CHG_MASK) != 0;
  snapshot.en_hiz = (snapshot.reg17 & REG17_EN_HIZ_MASK) != 0;
  snapshot.dis_ce_pin = (snapshot.reg17 & REG17_DIS_CE_PIN_MASK) != 0;
  snapshot.en_rev = (snapshot.reg19 & REG19_EN_REV_MASK) != 0;
  return true;
}

}  // namespace bq25756_core
