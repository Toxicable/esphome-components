#pragma once

#include "esphome/core/component.h"

#include <cstdint>

namespace esphome::toxic_solar {

class ToxicSolar : public Component {
 public:
  void set_battery_charge_current_ma(uint8_t current_ma) { battery_charge_current_ma_ = current_ma; }
  void set_boot_indicator(bool enabled) { boot_indicator_ = enabled; }
  void setup() override;
  float get_setup_priority() const override { return 1200.0f; }

 protected:
  void boot_led_on_();
  void boot_led_off_();

  uint8_t battery_charge_current_ma_{50};
  bool boot_indicator_{false};
  uint8_t boot_blinks_remaining_{0};
};

}  // namespace esphome::toxic_solar
