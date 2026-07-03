#pragma once

#include <cstdint>

namespace mcf8316d_core {

class RegisterBus {
 public:
  virtual ~RegisterBus() = default;

  virtual bool read_register32(uint16_t offset, uint32_t *value) = 0;
  virtual bool read_register16(uint16_t offset, uint16_t *value) = 0;
  virtual bool write_register32(uint16_t offset, uint32_t value) = 0;
  virtual void delay_microseconds(uint32_t delay_us) = 0;
};

}  // namespace mcf8316d_core
