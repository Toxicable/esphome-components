#include "bq769x0.h"

#include <cmath>
#include <vector>

#include "helpers/crc8.h"

namespace esphome {
namespace bq769x0 {

namespace {
constexpr uint8_t SYS_CTRL1_ADC_EN = 0x10;
constexpr uint8_t SYS_CTRL1_TEMP_SEL = 0x08;

constexpr float TS_LSB_V = 382e-6f;
constexpr float DIE_TEMP_V25 = 1.200f;
constexpr float DIE_TEMP_SLOPE = 0.0042f;
constexpr float CC_LSB_UV = 8.44f;
} // namespace

bool BQ769X0Driver::read_calibration(Cal *out) {
  uint8_t gain1 = 0;
  uint8_t gain2 = 0;
  uint8_t offset_raw = 0;
  if (!this->read_register8(Register::ADCGAIN1, &gain1))
    return false;
  if (!this->read_register8(Register::ADCOFFSET, &offset_raw))
    return false;
  if (!this->read_register8(Register::ADCGAIN2, &gain2))
    return false;

  uint8_t gain4_3 = (gain1 >> 2) & 0x03;
  uint8_t gain2_0 = (gain2 >> 5) & 0x07;
  uint8_t adcgain_code = static_cast<uint8_t>((gain4_3 << 3) | gain2_0);
  out->gain_uV_per_lsb = 365 + adcgain_code;
  out->offset_mV = static_cast<int8_t>(offset_raw);
  return true;
}

bool BQ769X0Driver::write_cc_cfg_0x19() {
  const uint8_t value = 0x19;
  return this->write_register8(Register::CC_CFG, value);
}

bool BQ769X0Driver::set_adc_enabled(bool on) {
  uint8_t reg = 0;
  if (!this->read_register8(Register::SYS_CTRL1, &reg))
    return false;
  if (on) {
    reg |= SYS_CTRL1_ADC_EN;
  } else {
    reg &= ~SYS_CTRL1_ADC_EN;
  }
  return this->write_register8(Register::SYS_CTRL1, reg);
}

bool BQ769X0Driver::set_temp_sel(bool thermistor) {
  uint8_t reg = 0;
  if (!this->read_register8(Register::SYS_CTRL1, &reg))
    return false;
  if (thermistor) {
    reg |= SYS_CTRL1_TEMP_SEL;
  } else {
    reg &= ~SYS_CTRL1_TEMP_SEL;
  }
  return this->write_register8(Register::SYS_CTRL1, reg);
}

bool BQ769X0Driver::read_sys_stat(uint8_t *sys_stat) { return this->read_register8(Register::SYS_STAT, sys_stat); }

bool BQ769X0Driver::clear_sys_stat_bits(uint8_t mask) { return this->write_register8(Register::SYS_STAT, mask); }

bool BQ769X0Driver::read_cell_adc14(uint8_t cell_index_1based, uint16_t *adc14) {
  if (cell_index_1based < 1 || cell_index_1based > 5) {
    return false;
  }
  uint8_t reg = Register::VC1_HI + (cell_index_1based - 1) * 2;
  uint8_t data[2] = {0, 0};
  if (!this->read_register_block(reg, data, sizeof(data)))
    return false;
  *adc14 = static_cast<uint16_t>(((data[0] & 0x3F) << 8) | data[1]);
  return true;
}

bool BQ769X0Driver::read_bat16(uint16_t *bat) {
  uint8_t data[2] = {0, 0};
  if (!this->read_register_block(Register::BAT_HI, data, sizeof(data)))
    return false;
  *bat = static_cast<uint16_t>((data[0] << 8) | data[1]);
  return true;
}

bool BQ769X0Driver::read_ts1_adc14(uint16_t *adc14) {
  uint8_t data[2] = {0, 0};
  if (!this->read_register_block(Register::TS1_HI, data, sizeof(data)))
    return false;
  *adc14 = static_cast<uint16_t>(((data[0] & 0x3F) << 8) | data[1]);
  return true;
}

bool BQ769X0Driver::read_cc16(int16_t *cc) {
  uint8_t data[2] = {0, 0};
  if (!this->read_register_block(Register::CC_HI, data, sizeof(data)))
    return false;
  *cc = static_cast<int16_t>((data[0] << 8) | data[1]);
  return true;
}

int BQ769X0Driver::cell_mV_from_adc(uint16_t adc14, const Cal &cal) {
  return static_cast<int>((cal.gain_uV_per_lsb * static_cast<int>(adc14)) / 1000) + cal.offset_mV;
}

int BQ769X0Driver::bat_mV_from_bat16(uint16_t bat16, int cell_count, const Cal &cal) {
  return static_cast<int>((4 * cal.gain_uV_per_lsb * static_cast<int>(bat16)) / 1000) +
         (cell_count * cal.offset_mV);
}

float BQ769X0Driver::ts_voltage_V_from_adc(uint16_t adc14) { return static_cast<float>(adc14) * TS_LSB_V; }

float BQ769X0Driver::ts_resistance_ohm_from_voltage(float vts_V) {
  const float pullup_ohm = 10000.0f;
  const float vref = 3.3f;
  if (vts_V >= vref) {
    return NAN;
  }
  return (pullup_ohm * vts_V) / (vref - vts_V);
}

float BQ769X0Driver::die_temp_C_from_adc(uint16_t adc14) {
  float vts = ts_voltage_V_from_adc(adc14);
  return 25.0f - ((vts - DIE_TEMP_V25) / DIE_TEMP_SLOPE);
}

float BQ769X0Driver::sense_uV_from_cc(int16_t cc) { return static_cast<float>(cc) * CC_LSB_UV; }

bool BQ769X0Driver::read_register_block(uint8_t reg, uint8_t *data, size_t len) {
  if (!this->crc_enabled_) {
    auto error = this->dev_->read_register(reg, data, len);
    this->last_error_ = error;
    return error == i2c::ERROR_OK;
  }
  return this->read_register_block_crc_(reg, data, len);
}

bool BQ769X0Driver::write_register_block(uint8_t reg, const uint8_t *data, size_t len) {
  if (!this->crc_enabled_) {
    auto error = this->dev_->write_bytes(reg, data, len);
    this->last_error_ = error;
    return error == i2c::ERROR_OK;
  }
  return this->write_register_block_crc_(reg, data, len);
}

bool BQ769X0Driver::read_register8(uint8_t reg, uint8_t *value) { return this->read_register_block(reg, value, 1); }

bool BQ769X0Driver::write_register8(uint8_t reg, uint8_t value) { return this->write_register_block(reg, &value, 1); }

bool BQ769X0Driver::read_register_block_crc_(uint8_t reg, uint8_t *data, size_t len) {
  auto error = this->dev_->write(&reg, 1);
  if (error != i2c::ERROR_OK) {
    this->last_error_ = error;
    return false;
  }

  std::vector<uint8_t> buffer(len * 2, 0);
  error = this->dev_->read(buffer.data(), buffer.size());
  if (error != i2c::ERROR_OK) {
    this->last_error_ = error;
    return false;
  }

  const uint8_t read_address = static_cast<uint8_t>((this->address_ << 1) | 0x01);
  for (size_t i = 0; i < len; i++) {
    uint8_t data_byte = buffer[i * 2];
    uint8_t crc_byte = buffer[i * 2 + 1];
    uint8_t computed_crc = 0;
    if (i == 0) {
      uint8_t crc_input[2] = {read_address, data_byte};
      computed_crc = helpers::crc8(crc_input, sizeof(crc_input));
    } else {
      computed_crc = helpers::crc8(&data_byte, 1);
    }
    if (computed_crc != crc_byte) {
      this->last_error_ = i2c::ERROR_UNKNOWN;
      return false;
    }
    data[i] = data_byte;
  }

  this->last_error_ = i2c::ERROR_OK;
  return true;
}

bool BQ769X0Driver::write_register_block_crc_(uint8_t reg, const uint8_t *data, size_t len) {
  if (len == 0) {
    return true;
  }

  std::vector<uint8_t> buffer;
  buffer.reserve(len * 2 + 1);
  buffer.push_back(reg);

  const uint8_t write_address = static_cast<uint8_t>((this->address_ << 1) | 0x00);
  for (size_t i = 0; i < len; i++) {
    buffer.push_back(data[i]);
    uint8_t computed_crc = 0;
    if (i == 0) {
      uint8_t crc_input[3] = {write_address, reg, data[i]};
      computed_crc = helpers::crc8(crc_input, sizeof(crc_input));
    } else {
      computed_crc = helpers::crc8(&data[i], 1);
    }
    buffer.push_back(computed_crc);
  }

  auto error = this->dev_->write(buffer.data(), buffer.size());
  this->last_error_ = error;
  return error == i2c::ERROR_OK;
}

} // namespace bq769x0
} // namespace esphome
