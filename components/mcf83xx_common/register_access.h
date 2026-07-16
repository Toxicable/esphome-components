#pragma once

#include <cstdint>

#include "../component_common/bit_field.h"
#include "register_bus.h"

namespace mcf83xx_common {

class RegisterAccess {
 public:
  explicit RegisterAccess(RegisterBus *bus, uint32_t successful_write_delay_us = 0U)
      : bus_(bus), successful_write_delay_us_(successful_write_delay_us) {}

  bool available() const { return this->bus_ != nullptr; }

  bool read32(uint16_t offset, uint32_t &value) const {
    return this->bus_ != nullptr && this->bus_->read_register32(offset, &value);
  }

  bool read16(uint16_t offset, uint16_t &value) const {
    return this->bus_ != nullptr && this->bus_->read_register16(offset, &value);
  }

  bool write32(uint16_t offset, uint32_t value) const {
    if (this->bus_ == nullptr || !this->bus_->write_register32(offset, value)) {
      return false;
    }
    this->delay(this->successful_write_delay_us_);
    return true;
  }

  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value) const {
    uint32_t current = 0;
    if (!this->read32(offset, current)) {
      return false;
    }
    const uint32_t next = component_common::replace_masked(current, mask, value);
    return next == current || this->write32(offset, next);
  }

  bool pulse_bits32(uint16_t offset, uint32_t mask, uint32_t asserted_delay_us = 0U,
                    uint32_t cleared_delay_us = 0U) const {
    if (!this->update_bits32(offset, mask, mask)) {
      return false;
    }
    this->delay(asserted_delay_us);
    const bool cleared = this->update_bits32(offset, mask, 0U);
    this->delay(cleared_delay_us);
    return cleared;
  }

  void delay(uint32_t delay_us) const {
    if (this->bus_ != nullptr && delay_us != 0U) {
      this->bus_->delay_microseconds(delay_us);
    }
  }

 private:
  RegisterBus *bus_{nullptr};
  uint32_t successful_write_delay_us_{0U};
};

}  // namespace mcf83xx_common
