#pragma once

#include <cstddef>
#include <cstdint>

#include "bq25628_bus.h"
#include "bq25628_service.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq25628 {

class BQ25628Component : public PollingComponent,
                         public i2c::I2CDevice,
                         public ::bq25628_core::RegisterBus {
 public:
  BQ25628Component() : service_(this) {}

  void set_battery_voltage_sensor(sensor::Sensor *sensor) {
    battery_voltage_sensor_ = sensor;
  }

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  bool read_registers(uint8_t reg, uint8_t *data, size_t len) override;
  bool write_registers(uint8_t reg, const uint8_t *data, size_t len) override;

 protected:
  ::bq25628_core::Bq25628Service service_;
  sensor::Sensor *battery_voltage_sensor_{nullptr};
};

}  // namespace bq25628
}  // namespace esphome
