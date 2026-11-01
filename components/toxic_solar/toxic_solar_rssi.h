#pragma once

#include "esphome/components/openthread/openthread.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::toxic_solar {

class ParentLastRssiSensor : public PollingComponent, public sensor::Sensor {
 public:
  void update() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
};

}  // namespace esphome::toxic_solar
