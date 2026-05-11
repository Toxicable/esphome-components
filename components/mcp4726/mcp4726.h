#pragma once

#include <cstdint>

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace mcp4726 {

class MCP4726Output : public Component, public output::FloatOutput, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void write_state(float state) override;

  void set_vref(uint8_t vref) { this->vref_ = vref & 0x03; }
  void set_gain(uint8_t gain) { this->gain_ = gain & 0x01; }
  void set_power_down(uint8_t power_down) { this->power_down_ = power_down & 0x03; }
  void set_zero_on_boot(bool zero_on_boot) { this->zero_on_boot_ = zero_on_boot; }

 protected:
  bool write_code_(uint16_t code);
  uint8_t command_byte_() const;
  const char *vref_name_() const;
  const char *gain_name_() const;
  const char *power_down_name_() const;

  // MCP47x6 volatile memory command fields.
  // Command byte: 0b010_VREF1_VREF0_PD1_PD0_G
  uint8_t vref_{0b11};        // default: VREF pin buffered
  uint8_t gain_{0b0};         // default: 1x
  uint8_t power_down_{0b00};  // default: normal operation
  bool zero_on_boot_{true};
};

}  // namespace mcp4726
}  // namespace esphome
