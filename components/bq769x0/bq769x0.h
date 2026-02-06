#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace bq769x0 {

enum Register : uint8_t {
  SYS_STAT = 0x00,
  CELLBAL1 = 0x01,
  SYS_CTRL1 = 0x04,
  SYS_CTRL2 = 0x05,
  PROTECT1 = 0x06,
  PROTECT2 = 0x07,
  PROTECT3 = 0x08,
  OV_TRIP = 0x09,
  UV_TRIP = 0x0A,
  CC_CFG = 0x0B,
  VC1_HI = 0x0C,
  VC1_LO = 0x0D,
  VC2_HI = 0x0E,
  VC2_LO = 0x0F,
  VC3_HI = 0x10,
  VC3_LO = 0x11,
  VC4_HI = 0x12,
  VC4_LO = 0x13,
  VC5_HI = 0x14,
  VC5_LO = 0x15,
  BAT_HI = 0x2A,
  BAT_LO = 0x2B,
  TS1_HI = 0x2C,
  TS1_LO = 0x2D,
  CC_HI = 0x32,
  CC_LO = 0x33,
  ADCGAIN1 = 0x50,
  ADCOFFSET = 0x51,
  ADCGAIN2 = 0x59,
};

struct Cal {
  int gain_uV_per_lsb;
  int offset_mV;
};

class BQ769X0Driver {
public:
  explicit BQ769X0Driver(esphome::i2c::I2CDevice *dev) : dev_(dev) {}

  void set_crc_enabled(bool enabled) { this->crc_enabled_ = enabled; }
  void set_i2c_address(uint8_t address) { this->address_ = address; }

  bool read_calibration(Cal *out);
  bool write_cc_cfg_0x19();
  bool set_adc_enabled(bool on);
  bool set_temp_sel(bool thermistor);

  bool read_sys_stat(uint8_t *sys_stat);
  bool clear_sys_stat_bits(uint8_t mask);
  bool read_cell_adc14(uint8_t cell_index_1based, uint16_t *adc14);
  bool read_bat16(uint16_t *bat);
  bool read_ts1_adc14(uint16_t *adc14);
  bool read_cc16(int16_t *cc);

  int cell_mV_from_adc(uint16_t adc14, const Cal &cal);
  int bat_mV_from_bat16(uint16_t bat16, int cell_count, const Cal &cal);
  float ts_voltage_V_from_adc(uint16_t adc14);
  float ts_resistance_ohm_from_voltage(float vts_V);
  float die_temp_C_from_adc(uint16_t adc14);
  float sense_uV_from_cc(int16_t cc);

  i2c::ErrorCode last_error() const { return this->last_error_; }

  bool read_register_block(uint8_t reg, uint8_t *data, size_t len);
  bool write_register_block(uint8_t reg, const uint8_t *data, size_t len);
  bool read_register8(uint8_t reg, uint8_t *value);
  bool write_register8(uint8_t reg, uint8_t value);

protected:
  bool read_register_block_crc_(uint8_t reg, uint8_t *data, size_t len);
  bool write_register_block_crc_(uint8_t reg, const uint8_t *data, size_t len);

  esphome::i2c::I2CDevice *dev_{nullptr};
  bool crc_enabled_{false};
  uint8_t address_{0x08};
  i2c::ErrorCode last_error_{i2c::ERROR_OK};
};

} // namespace bq769x0
} // namespace esphome
