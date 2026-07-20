#pragma once

#include <cstdint>

#include "bq25628_registers.h"

namespace bq25628_core {

static constexpr uint8_t BQ25628E_PART_NUMBER = 0x04;
static constexpr float VBAT_ADC_LSB_V = 0.00199f;

bool is_bq25628e(uint8_t part_information);
uint8_t enable_adc(uint8_t adc_control);
float decode_battery_voltage_v(uint16_t vbat_adc_raw_be);

}  // namespace bq25628_core
