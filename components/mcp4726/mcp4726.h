#pragma once

#include <cstdint>

#include "mcp4726_protocol.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mcp4726 {

class MCP4726Output : public Component, public output::FloatOutput, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void write_state(float state) override;

  void set_vref(uint8_t vref) { this->vref_ = vref & 0x03u; }
  void set_gain(uint8_t gain) { this->gain_ = gain & 0x01u; }
  void set_power_down(uint8_t power_down) { this->power_down_ = power_down & 0x03u; }
  void set_zero_on_boot(bool zero_on_boot) { this->zero_on_boot_ = zero_on_boot; }

 protected:
  bool write_code_(uint16_t code);
  const char *vref_name_() const;
  const char *gain_name_() const;
  const char *power_down_name_() const;

  uint8_t vref_{0b11};
  uint8_t gain_{0b0};
  uint8_t power_down_{0b00};
  bool zero_on_boot_{true};
};

}  // namespace mcp4726
}  // namespace esphome
