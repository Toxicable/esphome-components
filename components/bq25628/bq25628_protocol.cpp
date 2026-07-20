#include "bq25628_protocol.h"

namespace bq25628_core {

bool is_bq25628e(uint8_t part_information) {
  return fields::PartNumber::decode(part_information) == BQ25628E_PART_NUMBER;
}

uint8_t enable_adc(uint8_t adc_control) {
  return fields::AdcEnable::replace(adc_control, 1);
}

float decode_battery_voltage_v(uint16_t vbat_adc_raw_be) {
  return static_cast<float>(fields::BatteryVoltageAdc::decode(vbat_adc_raw_be)) * VBAT_ADC_LSB_V;
}

}  // namespace bq25628_core
