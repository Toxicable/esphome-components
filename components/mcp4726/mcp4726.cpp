#include "mcp4726.h"

#include <cmath>

#include "esphome/core/log.h"

namespace esphome {
namespace mcp4726 {

static const char *const TAG = "mcp4726.output";

void MCP4726Output::setup() {
  if (this->zero_on_boot_) {
    if (!this->write_code_(0)) {
      this->mark_failed();
    }
  }
}

void MCP4726Output::dump_config() {
  ESP_LOGCONFIG(TAG, "MCP4726 DAC Output:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  VREF: %s", this->vref_name_());
  ESP_LOGCONFIG(TAG, "  Gain: %s", this->gain_name_());
  ESP_LOGCONFIG(TAG, "  Power down: %s", this->power_down_name_());
  ESP_LOGCONFIG(TAG, "  Zero on boot: %s", YESNO(this->zero_on_boot_));
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with MCP4726 failed");
  }
}

void MCP4726Output::write_state(float state) {
  if (state < 0.0f) state = 0.0f;
  if (state > 1.0f) state = 1.0f;

  const uint16_t code = static_cast<uint16_t>(lroundf(state * 4095.0f));
  this->write_code_(code);
}

uint8_t MCP4726Output::command_byte_() const {
  // Write Volatile Memory command for MCP47x6:
  // C2:C0 = 010, then VREF1:VREF0, PD1:PD0, G.
  return static_cast<uint8_t>(0x40 | ((this->vref_ & 0x03) << 3) |
                              ((this->power_down_ & 0x03) << 1) |
                              (this->gain_ & 0x01));
}

bool MCP4726Output::write_code_(uint16_t code) {
  if (code > 4095) code = 4095;

  const uint16_t data = static_cast<uint16_t>(code << 4);  // MCP4726 D11..D0 left-aligned in 16-bit data field
  const uint8_t bytes[3] = {
      this->command_byte_(),
      static_cast<uint8_t>(data >> 8),
      static_cast<uint8_t>(data & 0xFF),
  };

  const auto err = this->write(bytes, sizeof(bytes));
  if (err != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "I2C write failed: %d", err);
    this->status_set_warning();
    return false;
  }

  this->status_clear_warning();
  return true;
}

const char *MCP4726Output::vref_name_() const {
  switch (this->vref_) {
    case 0b00:
    case 0b01:
      return "VDD";
    case 0b10:
      return "VREF pin, unbuffered";
    case 0b11:
      return "VREF pin, buffered";
    default:
      return "unknown";
  }
}

const char *MCP4726Output::gain_name_() const { return this->gain_ ? "2x" : "1x"; }

const char *MCP4726Output::power_down_name_() const {
  switch (this->power_down_) {
    case 0b00:
      return "normal";
    case 0b01:
      return "1k to ground";
    case 0b10:
      return "125k to ground";
    case 0b11:
      return "640k to ground";
    default:
      return "unknown";
  }
}

}  // namespace mcp4726
}  // namespace esphome
