#include "bq76952_soc.h"

#include <algorithm>
#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char *const TAG = "bq76952.soc";
constexpr float CURRENT_DIRECTION_THRESHOLD_A = 0.01F;
constexpr uint32_t SAVE_INTERVAL_MS = 60000;
}  // namespace

void BQ76952Soc::setup(BQ76952CellChemistry chemistry) {
  this->chemistry_ = chemistry;
  this->preference_ = global_preferences->make_preference<PersistedState>(PREFERENCE_NAMESPACE);
  this->preference_valid_ = true;
  this->load();
}

float BQ76952Soc::estimate_from_voltage(int16_t cell_voltage_mv, uint16_t empty_mv, uint16_t full_mv) const {
  const int16_t bounded =
      std::max<int16_t>(static_cast<int16_t>(empty_mv), std::min<int16_t>(static_cast<int16_t>(full_mv),
                                                                         cell_voltage_mv));
  const auto &curve = DEFAULT_LITHIUM_ION_CURVE;
  constexpr size_t curve_size = sizeof(curve) / sizeof(curve[0]);

  if (bounded <= curve[0].mv) {
    return curve[0].percent;
  }
  if (bounded >= curve[curve_size - 1].mv) {
    return curve[curve_size - 1].percent;
  }

  for (size_t i = 0; i + 1 < curve_size; i++) {
    if (bounded >= curve[i].mv && bounded <= curve[i + 1].mv) {
      const float position =
          static_cast<float>(bounded - curve[i].mv) / static_cast<float>(curve[i + 1].mv - curve[i].mv);
      return curve[i].percent + position * (curve[i + 1].percent - curve[i].percent);
    }
  }
  return 50.0F;
}

float BQ76952Soc::update(const BQ76952SocSample &sample) {
  if (!this->have_last_counter_) {
    this->boot_estimate_fraction_ =
        this->estimate_from_voltage(sample.average_cell_voltage_mv, sample.empty_cell_voltage_mv,
                                    sample.full_cell_voltage_mv) /
        100.0F;
    this->boot_relative_charge_ah_ = this->relative_charge_ah_;
  } else {
    const float delta_ah = sample.coulomb_counter_ah - this->last_coulomb_counter_ah_;
    if (std::isfinite(delta_ah) && std::fabs(delta_ah) < MAX_REASONABLE_COUNTER_DELTA_AH) {
      this->relative_charge_ah_ += delta_ah;
    } else {
      ESP_LOGW(TAG, "Ignoring implausible coulomb-counter jump: %.4f Ah", delta_ah);
    }
  }
  this->last_coulomb_counter_ah_ = sample.coulomb_counter_ah;
  this->have_last_counter_ = true;

  if (sample.current_a < -CURRENT_DIRECTION_THRESHOLD_A) {
    this->charge_seen_ = true;
  } else if (sample.current_a > CURRENT_DIRECTION_THRESHOLD_A) {
    this->discharge_seen_ = true;
  }

  const bool full_voltage =
      sample.maximum_cell_voltage_mv >= static_cast<int16_t>(sample.full_cell_voltage_mv - ENDPOINT_MARGIN_MV);
  const bool empty_voltage =
      sample.minimum_cell_voltage_mv <= static_cast<int16_t>(sample.empty_cell_voltage_mv + ENDPOINT_MARGIN_MV);
  const uint32_t now = millis();

  if (full_voltage) {
    if (this->full_hold_start_ms_ == 0) {
      this->full_hold_start_ms_ = now;
    }
  } else {
    this->full_hold_start_ms_ = 0;
  }

  if (empty_voltage) {
    if (this->empty_hold_start_ms_ == 0) {
      this->empty_hold_start_ms_ = now;
    }
  } else {
    this->empty_hold_start_ms_ = 0;
  }

  const bool full_now =
      sample.cell_overvoltage_active ||
      (full_voltage && this->charge_seen_ && std::fabs(sample.current_a) < CURRENT_DIRECTION_THRESHOLD_A &&
       this->full_hold_start_ms_ != 0 && (now - this->full_hold_start_ms_) >= ENDPOINT_HOLD_MS);
  const bool empty_now =
      sample.cell_undervoltage_active ||
      (empty_voltage && this->discharge_seen_ && this->empty_hold_start_ms_ != 0 &&
       (now - this->empty_hold_start_ms_) >= ENDPOINT_HOLD_MS);

  if (!full_now) {
    this->full_endpoint_latched_ = false;
  } else if (!this->full_endpoint_latched_) {
    this->mark_full();
    this->full_endpoint_latched_ = true;
  }

  if (!empty_now) {
    this->empty_endpoint_latched_ = false;
  } else if (!this->empty_endpoint_latched_) {
    this->mark_empty();
    this->empty_endpoint_latched_ = true;
  }

  float percent = 0.0F;
  if (this->have_span_ && this->learned_span_ah_ > 0.001F) {
    percent = 100.0F * (this->relative_charge_ah_ - this->empty_anchor_ah_) / this->learned_span_ah_;
  } else {
    percent = this->estimate_from_voltage(sample.average_cell_voltage_mv, sample.empty_cell_voltage_mv,
                                          sample.full_cell_voltage_mv);
  }

  percent = std::max(0.0F, std::min(100.0F, percent));
  this->save(false);
  return percent;
}

void BQ76952Soc::mark_full() {
  this->have_full_ = true;
  this->full_anchor_ah_ = this->relative_charge_ah_;
  ESP_LOGI(TAG, "Detected full endpoint at relative_charge=%.4f Ah", this->full_anchor_ah_);

  if (this->have_empty_) {
    this->update_learned_span();
  } else if (this->boot_estimate_fraction_ >= 0.10F && this->boot_estimate_fraction_ <= 0.90F) {
    const float span =
        (this->full_anchor_ah_ - this->boot_relative_charge_ah_) / (1.0F - this->boot_estimate_fraction_);
    if (span > 0.001F) {
      this->learned_span_ah_ = span;
      this->empty_anchor_ah_ = this->full_anchor_ah_ - span;
      this->have_span_ = true;
      this->span_provisional_ = true;
    }
  }

  this->full_hold_start_ms_ = 0;
  this->save(true);
}

void BQ76952Soc::mark_empty() {
  this->have_empty_ = true;
  this->empty_anchor_ah_ = this->relative_charge_ah_;
  ESP_LOGI(TAG, "Detected empty endpoint at relative_charge=%.4f Ah", this->empty_anchor_ah_);

  if (this->have_full_) {
    this->update_learned_span();
  } else if (this->boot_estimate_fraction_ >= 0.10F && this->boot_estimate_fraction_ <= 0.90F) {
    const float span =
        (this->boot_relative_charge_ah_ - this->empty_anchor_ah_) / this->boot_estimate_fraction_;
    if (span > 0.001F) {
      this->learned_span_ah_ = span;
      this->full_anchor_ah_ = this->empty_anchor_ah_ + span;
      this->have_span_ = true;
      this->span_provisional_ = true;
    }
  }

  this->empty_hold_start_ms_ = 0;
  this->save(true);
}

void BQ76952Soc::update_learned_span() {
  const float measured_span = this->full_anchor_ah_ - this->empty_anchor_ah_;
  if (measured_span <= 0.001F) {
    ESP_LOGW(TAG, "Ignoring invalid full/empty SoC span: %.4f Ah", measured_span);
    return;
  }

  this->learned_span_ah_ = measured_span;
  this->have_span_ = true;
  this->span_provisional_ = false;
  ESP_LOGI(TAG, "Updated learned capacity span: %.4f Ah", this->learned_span_ah_);
}

void BQ76952Soc::load() {
  if (!this->preference_valid_) {
    return;
  }

  PersistedState state{};
  if (!this->preference_.load(&state) || !std::isfinite(state.relative_charge_ah)) {
    ESP_LOGD(TAG, "No valid persisted SoC state");
    return;
  }

  this->relative_charge_ah_ = state.relative_charge_ah;
  // The device counter may reset independently of ESPHome. Establish a fresh
  // baseline from the first sample after every host boot instead of applying a
  // stale persisted device-counter delta.
  this->last_coulomb_counter_ah_ = 0.0F;
  this->have_last_counter_ = false;
  this->full_anchor_ah_ = state.full_anchor_ah;
  this->empty_anchor_ah_ = state.empty_anchor_ah;
  this->learned_span_ah_ = state.learned_span_ah;
  this->have_full_ = (state.flags & HAVE_FULL) != 0;
  this->have_empty_ = (state.flags & HAVE_EMPTY) != 0;
  this->have_span_ = (state.flags & HAVE_SPAN) != 0 && std::isfinite(state.learned_span_ah) &&
                     state.learned_span_ah > 0.001F;
  this->span_provisional_ = (state.flags & SPAN_PROVISIONAL) != 0;
  ESP_LOGI(TAG, "Loaded SoC state: relative_charge=%.4f Ah span=%.4f Ah", this->relative_charge_ah_,
           this->learned_span_ah_);
}

void BQ76952Soc::save(bool force) {
  if (!this->preference_valid_) {
    return;
  }

  const uint32_t now = millis();
  if (!force && (now - this->last_save_ms_) < SAVE_INTERVAL_MS) {
    return;
  }

  PersistedState state{};
  state.relative_charge_ah = this->relative_charge_ah_;
  state.full_anchor_ah = this->full_anchor_ah_;
  state.empty_anchor_ah = this->empty_anchor_ah_;
  state.learned_span_ah = this->learned_span_ah_;
  if (this->have_full_) state.flags |= HAVE_FULL;
  if (this->have_empty_) state.flags |= HAVE_EMPTY;
  if (this->have_span_) state.flags |= HAVE_SPAN;
  if (this->span_provisional_) state.flags |= SPAN_PROVISIONAL;

  if (!this->preference_.save(&state)) {
    ESP_LOGW(TAG, "Failed saving SoC state");
    return;
  }
  this->last_save_ms_ = now;
}

}  // namespace bq76952
}  // namespace esphome
