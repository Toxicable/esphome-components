#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace esc_higher {

class ESCHigherComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esc_higher
}  // namespace esphome
