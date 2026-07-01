#pragma once

#include <cstddef>
#include <cstdint>

#include "husb238_protocol.h"
#include "husb238_bus.h"

namespace husb238_core {

class HusbService {
 public:
  explicit HusbService(RegisterBus *bus) : bus_(bus) {}

  bool probe();
  bool read_status(Status *status);
  bool read_source_pdos(SourcePdo *pdos, size_t count);
  bool request_voltage(uint8_t voltage);
  bool request_source_capabilities();
  bool hard_reset();

  uint8_t last_requested_voltage() const { return this->last_requested_voltage_; }

 private:
  RegisterBus *bus_{nullptr};
  uint8_t last_requested_voltage_{0};
};

}  // namespace husb238_core
