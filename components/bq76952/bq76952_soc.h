#pragma once

#include <cstdint>
#include <limits>

#include "esphome/core/preferences.h"

namespace esphome {
namespace bq76952 {

// Host-side state-of-charge estimation and persistence state. This is kept
// separate from the BMS service/reconciliation state because it is an optional
// derived feature rather than part of the BQ76952 protocol or protection model.
class BQ76952SocState {
 protected:
  struct SocCurvePoint {
    uint16_t mv;
    float soc;
  };

  static constexpr int16_t SOC_FULL_MARGIN_MV = 20;
  static constexpr int16_t SOC_EMPTY_MARGIN_MV = 20;
  static constexpr uint32_t SOC_ENDPOINT_HOLD_S = 30;

  float soc_logical_ah_{0.0f};
  float soc_last_raw_passq_ah_{0.0f};
  bool soc_have_last_raw_{false};

  float soc_full_ah_{0.0f};
  float soc_empty_ah_{0.0f};
  bool soc_have_full_{false};
  bool soc_have_empty_{false};
  float soc_learned_span_ah_{0.0f};
  bool soc_have_span_{false};
  bool soc_span_provisional_{false};

  float soc_boot_estimate_fraction_{0.5f};
  float soc_boot_logical_ah_{0.0f};

  uint32_t soc_full_hold_start_ms_{0};
  uint32_t soc_empty_hold_start_ms_{0};
  bool soc_charge_seen_{false};
  bool soc_discharge_seen_{false};

  uint32_t soc_last_save_ms_{0};

  static constexpr SocCurvePoint DEFAULT_LIION_CURVE_[] = {
    {2800, 0.0f}, {3000, 3.0f}, {3200, 8.0f}, {3300, 12.0f},
    {3500, 25.0f}, {3600, 40.0f}, {3700, 58.0f}, {3800, 75.0f},
    {3900, 86.0f}, {4000, 94.0f}, {4100, 98.0f}, {4200, 100.0f},
  };

  static constexpr float SOC_MAX_REASONABLE_DELTA_AH = 100.0f;
  static constexpr uint32_t SOC_PREF_NAMESPACE = 0xB7695200u;

  struct SocPersistedState {
    float logical_ah{std::numeric_limits<float>::quiet_NaN()};
    float last_raw_passq_ah{std::numeric_limits<float>::quiet_NaN()};
    float full_ah{std::numeric_limits<float>::quiet_NaN()};
    float empty_ah{std::numeric_limits<float>::quiet_NaN()};
    float learned_span_ah{std::numeric_limits<float>::quiet_NaN()};
    float last_soc_percent{std::numeric_limits<float>::quiet_NaN()};
    uint8_t flags{0};
  };

  static constexpr uint8_t SOC_HAVE_FULL = 0x01;
  static constexpr uint8_t SOC_HAVE_EMPTY = 0x02;
  static constexpr uint8_t SOC_HAVE_SPAN = 0x04;
  static constexpr uint8_t SOC_SPAN_PROVISIONAL = 0x08;

  decltype(global_preferences->make_preference<SocPersistedState>(0)) soc_pref_{};
  bool soc_pref_valid_{false};
};

}  // namespace bq76952
}  // namespace esphome
