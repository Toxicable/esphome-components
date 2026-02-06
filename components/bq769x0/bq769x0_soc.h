#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace esphome {
namespace bq769x0 {

struct OcvPoint {
  int mv;
  float soc;
};

struct BalanceConfig {
  bool enabled{false};
  float balance_current_ma_per_cell{0.0f};
  float balance_duty{0.0f};
};

struct SocConfig {
  std::vector<OcvPoint> ocv_table;
  float rest_current_threshold_ma{0.0f};
  float rest_min_seconds{0.0f};
  float rest_full_weight_seconds{0.0f};
  float rest_dvdt_threshold_mv_per_s{0.0f};
  bool use_min_cell{true};

  float full_cell_mv{0.0f};
  float full_hold_seconds{0.0f};
  float empty_cell_mv{0.0f};
  float empty_hold_seconds{0.0f};
  float empty_discharge_current_ma{0.0f};
  bool use_hw_fault_anchors{false};

  bool current_positive_is_discharge{true};
  float coulombic_eff_discharge{1.0f};
  float coulombic_eff_charge{1.0f};

  float learn_alpha{1.0f};
  BalanceConfig balance{};
};

struct SocInputs {
  float current_ma{0.0f};
  float vmin_cell_mv{0.0f};
  float vavg_cell_mv{0.0f};
  float dt_s{0.0f};
  bool sys_ov{false};
  bool sys_uv{false};
  bool is_discharging{false};
  int balancing_cells{0};
};

struct SocOutputs {
  float soc_percent{std::numeric_limits<float>::quiet_NaN()};
  float soc_confidence{0.0f};
  float capacity_mah{std::numeric_limits<float>::quiet_NaN()};
  float q_remaining_mah{std::numeric_limits<float>::quiet_NaN()};
  bool soc_valid{false};
  bool rest_qualified{false};
};

class BQ769X0SocEstimator {
public:
  void configure(const SocConfig &cfg);
  void reset();
  void force_full_anchor();
  void force_empty_anchor();
  void clear_capacity();

  void set_capacity_mah(float capacity_mah);
  void set_soc_percent(float soc_percent);
  void set_q_remaining_mah(float q_remaining_mah);

  const SocOutputs &update(const SocInputs &in);

  const SocOutputs &outputs() const { return outputs_; }

protected:
  float ocv_soc_from_mv_(float vrest_mv) const;

  SocConfig cfg_{};
  SocOutputs outputs_{};

  float rest_seconds_{0.0f};
  float prev_vrest_mv_{std::numeric_limits<float>::quiet_NaN()};

  bool anchor_full_seen_{false};
  float discharged_since_full_mah_{0.0f};
  float full_hold_seconds_{0.0f};
  float empty_hold_seconds_{0.0f};
};

} // namespace bq769x0
} // namespace esphome
