#pragma once

#include <cstddef>
#include <cstdint>

namespace bq25628_core {

class RegisterBus {
 public:
  virtual ~RegisterBus() = default;

  virtual bool read_registers(uint8_t reg, uint8_t *data, size_t len) = 0;
  virtual bool write_registers(uint8_t reg, const uint8_t *data, size_t len) = 0;
};

}  // namespace bq25628_core
