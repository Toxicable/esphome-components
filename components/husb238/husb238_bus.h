#pragma once

#include <cstdint>

namespace husb238_core {

class RegisterBus {
 public:
  virtual ~RegisterBus() = default;

  virtual bool read_register(uint8_t reg, uint8_t *value) = 0;
  virtual bool write_register(uint8_t reg, uint8_t value) = 0;
  virtual void delay_ms(uint32_t ms) = 0;
};

}  // namespace husb238_core
