#include "bq769x0_soc.h"

#include <algorithm>
#include <cmath>

namespace esphome {
namespace bq769x0 {

void BQ769X0SocEstimator::configure(const SocConfig &cfg) { this->cfg_ = cfg; }

void BQ769X0SocEstimator::reset() {
  outputs_ = SocOutputs{};
  rest_seconds_ = 0.0f;
  prev_vrest_mv_ = NAN;
  anchor_full_seen_ = false;
  discharged_since_full_mah_ = 0.0f;
  full_hold_seconds_ = 0.0f;
  empty_hold_seconds_ = 0.0f;
}

void BQ769X0SocEstimator::force_full_anchor() {
  anchor_full_seen_ = true;
  discharged_since_full_mah_ = 0.0f;
  if (!std::isnan(outputs_.capacity_mah)) {
    outputs_.q_remaining_mah = outputs_.capacity_mah;
  }
  outputs_.soc_percent = 100.0f;
  outputs_.soc_valid = true;
}

void BQ769X0SocEstimator::force_empty_anchor() {
  outputs_.q_remaining_mah = 0.0f;
  outputs_.soc_percent = 0.0f;
  outputs_.soc_valid = true;
  anchor_full_seen_ = false;
}

void BQ769X0SocEstimator::clear_capacity() {
  outputs_.capacity_mah = NAN;
  outputs_.q_remaining_mah = NAN;
  outputs_.soc_valid = false;
}

void BQ769X0SocEstimator::set_capacity_mah(float capacity_mah) { outputs_.capacity_mah = capacity_mah; }

void BQ769X0SocEstimator::set_soc_percent(float soc_percent) {
  outputs_.soc_percent = soc_percent;
  outputs_.soc_valid = !std::isnan(soc_percent);
}

void BQ769X0SocEstimator::set_q_remaining_mah(float q_remaining_mah) { outputs_.q_remaining_mah = q_remaining_mah; }

float BQ769X0SocEstimator::ocv_soc_from_mv_(float vrest_mv) const {
  if (cfg_.ocv_table.empty()) {
    return NAN;
  }
  if (vrest_mv <= cfg_.ocv_table.front().mv) {
    return cfg_.ocv_table.front().soc;
  }
  if (vrest_mv >= cfg_.ocv_table.back().mv) {
    return cfg_.ocv_table.back().soc;
  }
  for (size_t i = 1; i < cfg_.ocv_table.size(); i++) {
    const auto &low = cfg_.ocv_table[i - 1];
    const auto &high = cfg_.ocv_table[i];
    if (vrest_mv <= high.mv) {
      float t = (vrest_mv - low.mv) / static_cast<float>(high.mv - low.mv);
      return low.soc + t * (high.soc - low.soc);
    }
  }
  return cfg_.ocv_table.back().soc;
}

const SocOutputs &BQ769X0SocEstimator::update(const SocInputs &in) {
  float vrest_mv = cfg_.use_min_cell ? in.vmin_cell_mv : in.vavg_cell_mv;
  float dvdt = 0.0f;
  if (!std::isnan(prev_vrest_mv_) && in.dt_s > 0.0f) {
    dvdt = std::abs(vrest_mv - prev_vrest_mv_) / in.dt_s;
  }
  prev_vrest_mv_ = vrest_mv;

  bool rest_candidate = std::abs(in.current_ma) <= cfg_.rest_current_threshold_ma &&
                        dvdt <= cfg_.rest_dvdt_threshold_mv_per_s;
  if (rest_candidate) {
    rest_seconds_ += in.dt_s;
  } else {
    rest_seconds_ = 0.0f;
  }
  bool rest_qualified = rest_seconds_ >= cfg_.rest_min_seconds;
  outputs_.rest_qualified = rest_qualified;

  float soc_confidence = 0.0f;
  if (rest_qualified && cfg_.rest_full_weight_seconds > 0.0f) {
    soc_confidence = std::clamp((rest_seconds_ - cfg_.rest_min_seconds) / cfg_.rest_full_weight_seconds, 0.0f, 1.0f);
  }
  outputs_.soc_confidence = soc_confidence;

  float ocv_soc = ocv_soc_from_mv_(vrest_mv);
  if (rest_qualified && !std::isnan(ocv_soc)) {
    if (!outputs_.soc_valid) {
      outputs_.soc_percent = ocv_soc;
      outputs_.soc_valid = true;
    } else {
      outputs_.soc_percent = (1.0f - soc_confidence) * outputs_.soc_percent + soc_confidence * ocv_soc;
    }
  }

  float delta_mah = 0.0f;
  if (in.dt_s > 0.0f) {
    float current_ma = in.current_ma;
    if (!cfg_.current_positive_is_discharge) {
      current_ma *= -1.0f;
    }
    float current_eff = current_ma;
    if (current_ma >= 0.0f && cfg_.coulombic_eff_discharge > 0.0f) {
      current_eff = current_ma / cfg_.coulombic_eff_discharge;
    } else if (current_ma < 0.0f) {
      current_eff = current_ma * cfg_.coulombic_eff_charge;
    }

    if (cfg_.balance.enabled && cfg_.balance.balance_current_ma_per_cell > 0.0f && cfg_.balance.balance_duty > 0.0f) {
      float balance_ma = in.balancing_cells * cfg_.balance.balance_current_ma_per_cell * cfg_.balance.balance_duty;
      current_eff += balance_ma;
    }

    delta_mah = current_eff * in.dt_s / 3600.0f;
  }

  bool full_anchor = rest_qualified && vrest_mv >= cfg_.full_cell_mv;
  if (full_anchor) {
    full_hold_seconds_ += in.dt_s;
  } else {
    full_hold_seconds_ = 0.0f;
  }
  if (cfg_.use_hw_fault_anchors && in.sys_ov) {
    full_hold_seconds_ = cfg_.full_hold_seconds;
  }
  if (full_hold_seconds_ >= cfg_.full_hold_seconds) {
    anchor_full_seen_ = true;
    discharged_since_full_mah_ = 0.0f;
    if (!std::isnan(outputs_.capacity_mah)) {
      outputs_.q_remaining_mah = outputs_.capacity_mah;
      outputs_.soc_percent = 100.0f;
      outputs_.soc_valid = true;
    }
  }

  bool empty_anchor = in.current_ma >= cfg_.empty_discharge_current_ma && vrest_mv <= cfg_.empty_cell_mv;
  if (empty_anchor) {
    empty_hold_seconds_ += in.dt_s;
  } else {
    empty_hold_seconds_ = 0.0f;
  }
  if (cfg_.use_hw_fault_anchors && in.sys_uv) {
    empty_hold_seconds_ = cfg_.empty_hold_seconds;
  }

  if (in.is_discharging && delta_mah > 0.0f) {
    discharged_since_full_mah_ += delta_mah;
  }

  if (empty_hold_seconds_ >= cfg_.empty_hold_seconds && anchor_full_seen_) {
    float measured_capacity = discharged_since_full_mah_;
    if (std::isnan(outputs_.capacity_mah)) {
      outputs_.capacity_mah = measured_capacity;
    } else {
      outputs_.capacity_mah = (1.0f - cfg_.learn_alpha) * outputs_.capacity_mah + cfg_.learn_alpha * measured_capacity;
    }
    outputs_.q_remaining_mah = 0.0f;
    outputs_.soc_percent = 0.0f;
    outputs_.soc_valid = true;
    anchor_full_seen_ = false;
  }

  if (!std::isnan(outputs_.capacity_mah)) {
    if (std::isnan(outputs_.q_remaining_mah) && outputs_.soc_valid) {
      outputs_.q_remaining_mah = outputs_.capacity_mah * (outputs_.soc_percent / 100.0f);
    }
    if (!std::isnan(outputs_.q_remaining_mah)) {
      outputs_.q_remaining_mah -= delta_mah;
      outputs_.q_remaining_mah = std::clamp(outputs_.q_remaining_mah, 0.0f, outputs_.capacity_mah);
      outputs_.soc_percent = (outputs_.q_remaining_mah / outputs_.capacity_mah) * 100.0f;
      outputs_.soc_valid = true;
    }
  }

  return outputs_;
}

} // namespace bq769x0
} // namespace esphome
