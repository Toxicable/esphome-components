#pragma once

#include <cstdint>

namespace component_common {

enum class LifecycleState : uint8_t {
  DISCONNECTED = 0,
  CONFIGURING,
  READY,
  FAILED,
};

inline const char *lifecycle_state_to_string(LifecycleState state) {
  switch (state) {
    case LifecycleState::DISCONNECTED:
      return "disconnected";
    case LifecycleState::CONFIGURING:
      return "configuring";
    case LifecycleState::READY:
      return "ready";
    case LifecycleState::FAILED:
      return "failed";
    default:
      return "unknown";
  }
}

template<typename PrimaryFault, typename Flags = uint32_t> struct FaultSnapshot {
  PrimaryFault primary{};
  Flags active_flags{};
  Flags latched_flags{};
};

}  // namespace component_common
