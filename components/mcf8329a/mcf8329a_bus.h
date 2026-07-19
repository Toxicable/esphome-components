#pragma once

#include "../mcf83xx_common/register_bus.h"
#include "mcf8329a_registers.h"

namespace mcf8329a_core {

using RegisterBus = mcf83xx_common::RegisterBus;
using RegisterId = regs::RegisterId;

}  // namespace mcf8329a_core

namespace esphome {
namespace mcf8329a {

using RegisterId = ::mcf8329a_core::RegisterId;

}  // namespace mcf8329a
}  // namespace esphome
