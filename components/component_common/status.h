#pragma once

#include <cstdint>

namespace component_common {

enum class ConnectionState : uint8_t {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED,
  FAILED,
};

inline const char *connection_state_to_string(ConnectionState state) {
  switch (state) {
    case ConnectionState::DISCONNECTED:
      return "disconnected";
    case ConnectionState::CONNECTING:
      return "connecting";
    case ConnectionState::CONNECTED:
      return "connected";
    case ConnectionState::FAILED:
      return "failed";
    default:
      return "unknown";
  }
}

}  // namespace component_common
