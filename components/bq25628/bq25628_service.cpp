#include "bq25628_service.h"

namespace bq25628_core {

bool Bq25628Service::probe() {
  uint8_t part_information;
  return this->read_byte_(RegisterId::PART_INFORMATION, part_information) &&
         is_bq25628e(part_information);
}

bool Bq25628Service::enable_adc() {
  uint8_t adc_control;
  if (!this->read_byte_(RegisterId::ADC_CONTROL, adc_control))
    return false;

  const uint8_t requested = bq25628_core::enable_adc(adc_control);
  return requested == adc_control || this->write_byte_(RegisterId::ADC_CONTROL, requested);
}

bool Bq25628Service::read_battery_voltage_v(float &voltage_v) {
  uint8_t raw[2];
  if (!this->read_bytes_(RegisterId::VBAT_ADC, raw, sizeof(raw)))
    return false;

  const uint16_t raw_be = (static_cast<uint16_t>(raw[0]) << 8) | raw[1];
  voltage_v = decode_battery_voltage_v(raw_be);
  return true;
}

bool Bq25628Service::read_byte_(RegisterId reg, uint8_t &value) {
  return this->read_bytes_(reg, &value, 1);
}

bool Bq25628Service::read_bytes_(RegisterId reg, uint8_t *data, size_t len) {
  return this->bus_ != nullptr &&
         this->bus_->read_registers(register_address(reg), data, len);
}

bool Bq25628Service::write_byte_(RegisterId reg, uint8_t value) {
  return this->bus_ != nullptr &&
         this->bus_->write_registers(register_address(reg), &value, 1);
}

}  // namespace bq25628_core
