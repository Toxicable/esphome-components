#pragma once

#include "../mcf83xx_common/register_bus.h"
#include "mcf8316d_registers.h"

namespace mcf8316d_core {

using RegisterBus = mcf83xx_common::RegisterBus;
using RegisterId = regs::RegisterId;

}  // namespace mcf8316d_core

namespace esphome {
namespace mcf8316d {

using RegisterId = ::mcf8316d_core::RegisterId;

}  // namespace mcf8316d
}  // namespace esphome
