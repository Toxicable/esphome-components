#pragma once

#include "load_types.h"

namespace esphome {
namespace programmable_load {

// A procedure is a pure operation policy. It never owns hardware and never
// calls the core. The core supplies calibrated measurements and consumes the
// returned current request after applying all limits and fault checks.
class Procedure {
 public:
  virtual ~Procedure() = default;

  virtual const char *name() const = 0;
  virtual ProcedureResult start(const Measurement &measurement) = 0;
  virtual ProcedureResult update(const Measurement &measurement) = 0;
  virtual void stop(StopReason reason) = 0;
};

}  // namespace programmable_load
}  // namespace esphome
