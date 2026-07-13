#pragma once

#include "load_types.h"

namespace esphome {
namespace programmable_load {

class ProgrammableLoadComponent;

class LoadProcedure {
 public:
  virtual ~LoadProcedure() = default;

  virtual const char *procedure_name() const = 0;
  virtual bool start(const LoadMeasurement &measurement) = 0;
  virtual void update(const LoadMeasurement &measurement) = 0;
  virtual void stop(LoadStopReason reason) = 0;

  virtual float requested_current_a() const = 0;
  virtual bool is_complete() const = 0;

  void set_parent(ProgrammableLoadComponent *parent) { this->parent_ = parent; }

 protected:
  ProgrammableLoadComponent *parent_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome
