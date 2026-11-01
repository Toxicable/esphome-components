#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::toxic_solar {

class BatteryPercentSensor : public Component, public sensor::Sensor {
 public:
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { this->voltage_sensor_ = voltage_sensor; }
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void publish_voltage_(float voltage);
  sensor::Sensor *voltage_sensor_{nullptr};
};

}  // namespace esphome::toxic_solar
