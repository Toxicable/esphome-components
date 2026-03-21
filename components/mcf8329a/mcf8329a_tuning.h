#pragma once

#include <cstdint>

namespace esphome {
namespace mcf8329a {

class MCF8329AComponent;

class MCF8329ATuningController {
 public:
  explicit MCF8329ATuningController(MCF8329AComponent* parent);
  ~MCF8329ATuningController();

  void reset();
  void start_initial_tune();
  void start_mpet_characterization();
  void update(bool normal_operation_ready, bool fault_active);

 private:
  struct Impl;
  Impl* impl_{nullptr};
};

}  // namespace mcf8329a
}  // namespace esphome
