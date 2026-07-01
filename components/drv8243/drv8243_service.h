#pragma once

#include "drv8243_bus.h"
#include "drv8243_protocol.h"

namespace drv8243_core {

class Drv8243Service {
 public:
  explicit Drv8243Service(PinBus *bus) : bus_(bus) {}

  HandshakeResult handshake();
  void set_static_polarity(bool level);

 private:
  PinBus *bus_{nullptr};
};

}  // namespace drv8243_core
