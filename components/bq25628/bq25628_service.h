#pragma once

#include "bq25628_bus.h"
#include "bq25628_protocol.h"

namespace bq25628_core {

class Bq25628Service {
 public:
  explicit Bq25628Service(RegisterBus *bus) : bus_(bus) {}

  bool probe();
  bool enable_adc();
 bool read_battery_voltage_v(float &voltage_v);

 private:
  bool read_byte_(RegisterId reg, uint8_t &value);
  bool read_bytes_(RegisterId reg, uint8_t *data, size_t len);
  bool write_byte_(RegisterId reg, uint8_t value);

  RegisterBus *bus_{nullptr};
};

}  // namespace bq25628_core
