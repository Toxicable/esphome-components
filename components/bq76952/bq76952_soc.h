#pragma once

#include <cstdint>
#include <limits>

#include "esphome/core/preferences.h"

#include "bq76952_config.h"

namespace esphome {
namespace bq76952 {

struct BQ76952SocSample {
  float current_a{0.0f};

  // Integrated charge reported by DASTATUS6. This is a signed coulomb-counter
  // position in amp-hours, not energy and not a user-facing lifetime total.
  float coulomb_counter_ah{0.0f};

  int16_t minimum_cell_voltage_mv{0};
  int16_t maximum_cell_voltage_mv{0};
  int16_t average_cell_voltage_mv{0};
  bool cell_undervoltage_active{false};
  bool cell_overvoltage_active{false};
  uint16_t empty_cell_voltage_mv{0};
  uint16_t full_cell_voltage_mv{0};
};

// Ancillary SoC logic owned by BQ76952Service. It is separated only to keep
// estimation and persistence out of the core protocol/reconciliation code.
class BQ76952Soc {
 public:
  void setup(BQ76952CellChemistry chemistry);
  float update(const BQ76952SocSample &sample);

  bool has_confirmed_capacity() const;
  float learned_capacity_ah() const;
  const char *capacity_calibration_status() const;

 private:
  struct CurvePoint {
    uint16_t mv;
    float percent;
  };

  struct PersistedState {
    float relative_charge_ah{std::numeric_limits<float>::quiet_NaN()};
    float full_anchor_ah{std::numeric_limits<float>::quiet_NaN()};
    float empty_anchor_ah{std::numeric_limits<float>::quiet_NaN()};
    float learned_span_ah{std::numeric_limits<float>::quiet_NaN()};
    uint8_t flags{0};
  };

  float estimate_from_voltage(int16_t cell_voltage_mv, uint16_t empty_mv, uint16_t full_mv) const;
  void mark_full();
  void mark_empty();
  void update_learned_span();
  void load();
  void save(bool force);

  static constexpr int16_t ENDPOINT_MARGIN_MV = 20;
  static constexpr uint32_t ENDPOINT_HOLD_MS = 30000;
  static constexpr float MAX_REASONABLE_COUNTER_DELTA_AH = 100.0f;
  static constexpr uint32_t PREFERENCE_NAMESPACE = 0xB7695200u;

  static constexpr uint8_t HAVE_FULL = 0x01;
  static constexpr uint8_t HAVE_EMPTY = 0x02;
  static constexpr uint8_t HAVE_SPAN = 0x04;
  static constexpr uint8_t SPAN_PROVISIONAL = 0x08;

  // Voltage-only fallback used until full/empty coulomb-count anchors have
  // been learned. The configured chemistry must match this curve.
  static constexpr CurvePoint DEFAULT_LITHIUM_ION_CURVE[] = {
      {2800, 0.0f},  {3000, 3.0f},  {3200, 8.0f},  {3300, 12.0f},
      {3500, 25.0f}, {3600, 40.0f}, {3700, 58.0f}, {3800, 75.0f},
      {3900, 86.0f}, {4000, 94.0f}, {4100, 98.0f}, {4200, 100.0f},
  };

  BQ76952CellChemistry chemistry_{BQ76952CellChemistry::LITHIUM_ION};

  // Continuous internal charge coordinate. It follows deltas from the device
  // coulomb counter while surviving device-counter resets and wraparound.
  float relative_charge_ah_{0.0f};
  float last_coulomb_counter_ah_{0.0f};
  bool have_last_counter_{false};

  // Learned relative-charge positions corresponding to full and empty.
  float full_anchor_ah_{0.0f};
  float empty_anchor_ah_{0.0f};
  float learned_span_ah_{0.0f};
  bool have_full_{false};
  bool have_empty_{false};
  bool have_span_{false};
  bool span_provisional_{false};

  float boot_estimate_fraction_{0.5f};
  float boot_relative_charge_ah_{0.0f};
  uint32_t full_hold_start_ms_{0};
  uint32_t empty_hold_start_ms_{0};
  bool full_endpoint_latched_{false};
  bool empty_endpoint_latched_{false};
  bool charge_seen_{false};
  bool discharge_seen_{false};
  uint32_t last_save_ms_{0};

  decltype(global_preferences->make_preference<PersistedState>(0)) preference_{};
  bool preference_valid_{false};
};

}  // namespace bq76952
}  // namespace esphome
