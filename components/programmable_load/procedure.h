#pragma once

#include "load_types.h"

namespace esphome {
namespace programmable_load {

// A procedure is a pure operation policy. It never owns hardware and never
// calls the core. The core supplies load and charger measurements, then applies
// the returned load-current and charger-enable requests after mutual-exclusion
// and safety checks.
class Procedure {
 public:
  virtual ~Procedure() = default;

  virtual const char *name() const = 0;
  virtual ProcedureResult start(const ProcedureContext &context) = 0;
  virtual ProcedureResult update(const ProcedureContext &context) = 0;
  virtual void stop(StopReason reason) = 0;
};

}  // namespace programmable_load
}  // namespace esphome
