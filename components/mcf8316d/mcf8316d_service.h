#pragma once

#include <cstdint>
#include <string>

#include "../mcf83xx_common/register_access.h"
#include "mcf8316d_bus.h"
#include "mcf8316d_protocol.h"

namespace mcf8316d_core {

class MCF8316DService {
 public:
  explicit MCF8316DService(RegisterBus *bus) : registers_(bus) {}

  bool read_reg32(uint16_t offset, uint32_t &value) const;
  bool read_reg16(uint16_t offset, uint16_t &value) const;
  bool write_reg32(uint16_t offset, uint32_t value) const;
  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value) const;

  bool set_brake_input(bool brake_on) const;
  bool read_brake_input(uint8_t &brake_input_code) const;
  bool set_direction_input(DirectionInputMode mode) const;
  bool read_direction_input(uint8_t &direction_input_code) const;
  bool write_speed_command_percent(float speed_percent) const;
  bool pulse_clear_faults() const;
  bool pulse_watchdog_tickle() const;

 private:
  mcf83xx_common::RegisterAccess registers_;
};

}  // namespace mcf8316d_core
