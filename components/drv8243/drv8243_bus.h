#pragma once

#include <cstdint>

namespace drv8243_core {

class PinBus {
 public:
  virtual ~PinBus() = default;

  virtual void write_nsleep(bool level) = 0;
  virtual bool read_nfault(bool *level) = 0;
  virtual void write_out2(bool level) = 0;
  virtual void delay_ms(uint32_t ms) = 0;
  virtual void delay_us(uint32_t us) = 0;
};

}  // namespace drv8243_core
