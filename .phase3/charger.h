#pragma once

#include <cstdint>

namespace component_common {

enum class ChargerState : uint8_t {
  UNKNOWN = 0,
  NOT_CHARGING,
  TRICKLE,
  PRECHARGE,
  FAST_CC,
  TAPER_CV,
  TOPOFF,
  TERMINATION_DONE,
};

struct ChargerCapabilities {
  bool enable_control{false};
  bool battery_current{false};
  bool battery_voltage{false};
  bool charge_state{false};
  bool power_good{false};
  bool fault_status{false};
};

struct ChargerSnapshot {
  uint32_t sequence{0};
  uint32_t timestamp_ms{0};
  float current_a{0.0f};
  float voltage_v{0.0f};
  ChargerState state{ChargerState::UNKNOWN};
  uint32_t status_flags{0};
  uint32_t fault_flags{0};
  bool enabled{false};
  bool power_good{false};
  bool fault_active{false};
  bool valid{false};
};

class ChargerInterface {
 public:
  virtual ~ChargerInterface() = default;
  virtual ChargerCapabilities capabilities() const = 0;
  virtual ChargerSnapshot snapshot() const = 0;
  virtual bool request_enabled(bool enabled) = 0;
};

inline const char *charger_state_to_string(ChargerState state) {
  switch (state) {
    case ChargerState::NOT_CHARGING: return "not_charging";
    case ChargerState::TRICKLE: return "trickle";
    case ChargerState::PRECHARGE: return "precharge";
    case ChargerState::FAST_CC: return "fast_cc";
    case ChargerState::TAPER_CV: return "taper_cv";
    case ChargerState::TOPOFF: return "topoff";
    case ChargerState::TERMINATION_DONE: return "termination_done";
    default: return "unknown";
  }
}

}  // namespace component_common
