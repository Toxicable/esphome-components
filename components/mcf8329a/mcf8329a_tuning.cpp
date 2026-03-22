#if !defined(MCF8329A_EMBED_IMPL) || defined(MCF8329A_EMBED_IMPL_INCLUDE)

#include "mcf8329a_tuning.h"

#include "mcf8329a.h"
#include "mcf8329a_tables.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8329a {

using namespace regs;

static const char* const TUNING_TAG = "mcf8329a";
struct InitialTuneCandidate {
  uint8_t phase_ilimit_code;
  uint8_t lock_ilimit_code;
  uint8_t hw_lock_ilimit_code;
  uint8_t open_loop_ilimit_code;
  uint8_t open_loop_accel_a1_code;
  uint8_t open_loop_accel_a2_code;
  uint8_t handoff_code;
  uint8_t theta_error_ramp_code;
  uint8_t cl_slow_acc_code;
  uint8_t lock_ilimit_deglitch_code;
  uint8_t hw_lock_ilimit_deglitch_code;
  bool auto_handoff_enable;
  bool abn_bemf_lock_enable;
};

static const InitialTuneCandidate TUNING_INITIAL_TUNE_CANDIDATES[] = {
  // Baseline: manual handoff at 14%, higher OL accel (250Hz/s).
  {6u, 6u, 6u, 3u, 10u, 0u, 13u, 2u, 4u, 8u, 7u, false, false},
  // Faster OL accel variant (500Hz/s) at same handoff.
  {6u, 6u, 6u, 3u, 11u, 0u, 13u, 2u, 4u, 8u, 7u, false, false},
  // Slightly later handoff with baseline accel.
  {6u, 6u, 6u, 3u, 10u, 0u, 15u, 2u, 4u, 8u, 7u, false, false},
  // Conservative fallback: lower accel with earlier handoff.
  {6u, 6u, 6u, 3u, 9u, 0u, 11u, 2u, 4u, 8u, 7u, false, false},
};
static constexpr size_t TUNING_INITIAL_TUNE_CANDIDATE_COUNT =
  sizeof(TUNING_INITIAL_TUNE_CANDIDATES) / sizeof(TUNING_INITIAL_TUNE_CANDIDATES[0]);

struct MCF8329ATuningController::Impl {
 public:
  explicit Impl(MCF8329AComponent* parent) : parent_(parent) {}

  void reset() {
    this->initial_tune_active_ = false;
    this->initial_tune_stage_ = InitialTuneStage::IDLE;
    this->initial_tune_candidate_index_ = 0u;
    this->initial_tune_stage_started_ms_ = 0u;
    this->initial_tune_waiting_fault_recovery_ = false;
    this->initial_tune_last_fault_clear_ms_ = 0u;
    this->initial_tune_closed_loop_seen_ = false;
    this->initial_tune_closed_loop_seen_ms_ = 0u;
    this->current_candidate_start_ms_ = 0u;
    this->current_candidate_reach_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_monitor_timeout_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_open_loop_dwell_timeout_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->handoff_unstable_counter_ = 0u;
    this->handoff_stable_counter_ = 0u;
    this->handoff_telemetry_miss_counter_ = 0u;
    this->refinement_active_ = false;
    this->refinement_candidate_index_ = 0u;
    this->refinement_candidate_count_ = 0u;
    this->refinement_best_candidate_valid_ = false;
    this->refinement_best_score_ = std::numeric_limits<int32_t>::min();
    this->refinement_best_reach_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_metrics_ = CandidateQualityMetrics{};
    this->refinement_best_metrics_ = CandidateQualityMetrics{};
    this->mpet_characterization_active_ = false;
    this->mpet_characterization_started_ms_ = 0u;
    this->mpet_last_status_log_ms_ = 0u;
    this->mpet_states_seen_mask_ = 0u;
    this->mpet_last_status_raw_ = 0u;
    this->mpet_last_status_valid_ = false;
  }

  void start_initial_tune() {
    if (this->parent_ == nullptr || !this->parent_->normal_operation_ready_) {
      ESP_LOGW(TUNING_TAG, "Initial tune requested before communications are ready");
      return;
    }
    if (this->mpet_characterization_active_) {
      ESP_LOGW(TUNING_TAG, "Initial tune blocked while MPET characterization is active");
      return;
    }

    this->clear_runtime_speed_command_("initial_tune_prepare");
    this->parent_->pulse_clear_faults();
    if (this->parent_->cfg_open_loop_limit_source_set_ && this->parent_->cfg_open_loop_limit_use_ilimit_) {
      ESP_LOGW(
        TUNING_TAG,
        "Initial tune warning: open_loop_limit_source=ilimit can increase open-loop current/heating; "
        "prefer ol_ilimit for discovery runs."
      );
    }

    this->initial_tune_active_ = true;
    this->initial_tune_stage_ = InitialTuneStage::APPLY;
    this->initial_tune_candidate_index_ = 0u;
    this->initial_tune_stage_started_ms_ = millis();
    this->initial_tune_waiting_fault_recovery_ = false;
    this->initial_tune_last_fault_clear_ms_ = 0u;
    this->initial_tune_closed_loop_seen_ = false;
    this->initial_tune_closed_loop_seen_ms_ = 0u;
    this->current_candidate_start_ms_ = 0u;
    this->current_candidate_reach_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_monitor_timeout_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_open_loop_dwell_timeout_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->handoff_unstable_counter_ = 0u;
    this->handoff_stable_counter_ = 0u;
    this->handoff_telemetry_miss_counter_ = 0u;
    this->refinement_active_ = false;
    this->refinement_candidate_index_ = 0u;
    this->refinement_candidate_count_ = 0u;
    this->refinement_best_candidate_valid_ = false;
    this->refinement_best_score_ = std::numeric_limits<int32_t>::min();
    this->refinement_best_reach_ms_ = INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    this->current_candidate_metrics_ = CandidateQualityMetrics{};
    this->refinement_best_metrics_ = CandidateQualityMetrics{};

    ESP_LOGI(
      TUNING_TAG,
      "Initial tune started: %u discovery candidate(s), target speed %.1f%%",
      static_cast<unsigned>(TUNING_INITIAL_TUNE_CANDIDATE_COUNT),
      INITIAL_TUNE_SPEED_PERCENT
    );
    ESP_LOGI(TUNING_TAG, "Initial tune will run a refinement sweep after first success (manual-handoff candidates)");
  }

  void start_mpet_characterization() {
    if (this->parent_ == nullptr || !this->parent_->normal_operation_ready_) {
      ESP_LOGW(TUNING_TAG, "MPET characterization requested before communications are ready");
      return;
    }
    if (this->initial_tune_active_) {
      ESP_LOGW(TUNING_TAG, "MPET characterization blocked while initial tuning is active");
      return;
    }
    if (this->mpet_characterization_active_) {
      ESP_LOGW(TUNING_TAG, "MPET characterization is already running");
      return;
    }

    this->clear_runtime_speed_command_("mpet_prepare");
    this->parent_->pulse_clear_faults();
    (void)this->parent_->clear_mpet_bits_("mpet_prepare");

    if (!this->parent_->set_brake_override(false)) {
      ESP_LOGW(TUNING_TAG, "MPET characterization aborted: failed to release brake");
      return;
    }
    if (this->parent_->brake_switch_ != nullptr) {
      this->parent_->brake_switch_->publish_state(false);
    }

    if (!this->parent_->comms_client_.set_mpet_characterization_bits()) {
      ESP_LOGW(TUNING_TAG, "MPET characterization aborted: failed to set MPET command bits");
      return;
    }

    this->mpet_characterization_active_ = true;
    this->mpet_characterization_started_ms_ = millis();
    this->mpet_last_status_log_ms_ = 0u;
    this->mpet_states_seen_mask_ = 0u;
    this->mpet_last_status_raw_ = 0u;
    this->mpet_last_status_valid_ = false;
    const uint32_t mpet_timeout_ms =
      this->parent_->mpet_timeout_ms_ > 0u ? this->parent_->mpet_timeout_ms_
                                           : DEFAULT_MPET_RUN_TIMEOUT_MS;
    ESP_LOGI(
      TUNING_TAG, "MPET characterization started (timeout %us)", static_cast<unsigned>(mpet_timeout_ms / 1000u)
    );
    this->log_mpet_profile_("MPET start");
  }

  void update(bool normal_operation_ready, bool fault_active) {
    if (!normal_operation_ready) {
      return;
    }
    const uint32_t now = millis();
    if (this->initial_tune_active_) {
      this->process_initial_tune_(fault_active, now);
    }
    if (this->mpet_characterization_active_) {
      this->process_mpet_characterization_(fault_active, now);
    }
  }

 private:
  enum class InitialTuneStage : uint8_t {
    IDLE = 0,
    APPLY = 1,
    START = 2,
    MONITOR = 3,
    COOLDOWN = 4,
  };

  struct CandidateQualityMetrics {
    uint16_t sample_count{0u};
    uint16_t telemetry_miss_count{0u};
    uint16_t unstable_count{0u};
    float tracking_error_sum_hz{0.0f};
    float mismatch_error_sum_hz{0.0f};
    float peak_speed_ratio{0.0f};
  };

  static constexpr float INITIAL_TUNE_SPEED_PERCENT = 11.0f;
  static constexpr uint32_t INITIAL_TUNE_SETTLE_MS = 250u;
  static constexpr uint32_t INITIAL_TUNE_MONITOR_TIMEOUT_MS = 7000u;
  static constexpr uint32_t INITIAL_TUNE_MONITOR_TIMEOUT_MIN_MS = 7000u;
  static constexpr uint32_t INITIAL_TUNE_MONITOR_TIMEOUT_MAX_MS = 45000u;
  static constexpr float INITIAL_TUNE_MONITOR_TIMEOUT_SCALE = 1.6f;
  static constexpr uint32_t INITIAL_TUNE_MONITOR_TIMEOUT_MARGIN_MS = 2500u;
  static constexpr uint32_t INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MIN_MS = 2000u;
  static constexpr uint32_t INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MAX_MS = 12000u;
  static constexpr float INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_SCALE = 1.3f;
  static constexpr uint32_t INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MARGIN_MS = 1200u;
  static constexpr uint32_t INITIAL_TUNE_SUCCESS_HOLD_MS = 800u;
  static constexpr uint32_t INITIAL_TUNE_HANDOFF_GUARD_GRACE_MS = 250u;
  static constexpr uint32_t INITIAL_TUNE_COOLDOWN_MS = 700u;
  static constexpr uint32_t INITIAL_TUNE_FAULT_RECOVERY_PULSE_INTERVAL_MS = 500u;
  // Large low-kV motors can spend a long dwell in KE measurement.
  static constexpr uint32_t DEFAULT_MPET_RUN_TIMEOUT_MS = 120000u;
  static constexpr uint32_t MPET_STATUS_LOG_INTERVAL_MS = 1000u;
  static constexpr uint8_t MAX_REFINED_CANDIDATES = 6u;
  static constexpr int32_t ACCEL_A1_BONUS_SCORE = 40;
  static constexpr float SCORE_TRACKING_ERROR_WEIGHT = 90.0f;
  static constexpr float SCORE_MISMATCH_ERROR_WEIGHT = 120.0f;
  static constexpr float SCORE_OVERSPEED_RATIO_WEIGHT = 4500.0f;
  static constexpr int32_t SCORE_UNSTABLE_SAMPLE_PENALTY = 1800;
  static constexpr int32_t SCORE_TELEMETRY_MISS_PENALTY = 700;
  static constexpr int32_t SCORE_NO_SAMPLE_PENALTY = 25000;
  static constexpr uint8_t HANDOFF_UNSTABLE_REJECT_COUNT = 2u;
  static constexpr uint8_t HANDOFF_STABLE_ACCEPT_COUNT = 2u;
  static constexpr uint8_t HANDOFF_TELEMETRY_MISS_REJECT_COUNT = 3u;

  bool is_closed_loop_state_(uint16_t algo_state) const {
    return algo_state == 0x0008u || algo_state == 0x0009u;
  }

  bool read_algorithm_state_(uint16_t& algo_state) const {
    if (this->parent_ == nullptr) {
      return false;
    }
    return this->parent_->read_reg16(REG_ALGORITHM_STATE, algo_state);
  }

  void clear_runtime_speed_command_(const char* reason) {
    if (this->parent_ == nullptr) {
      return;
    }
    this->parent_->speed_target_percent_ = 0.0f;
    this->parent_->speed_target_active_ = false;
    this->parent_->start_boost_active_ = false;
    this->parent_->start_boost_until_ms_ = 0u;
    this->parent_->last_ramp_update_ms_ = 0u;
    (void)this->parent_->apply_speed_command_(0.0f, reason, true);
  }

  bool apply_tune_candidate_(const InitialTuneCandidate& candidate) {
    if (this->parent_ == nullptr) {
      return false;
    }

    const uint32_t fault_cfg1_mask = FAULT_CONFIG1_ILIMIT_MASK |
                                     FAULT_CONFIG1_LOCK_ILIMIT_MASK |
                                     FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK |
                                     FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK;
    const uint32_t fault_cfg1_value =
      ((static_cast<uint32_t>(candidate.phase_ilimit_code) << FAULT_CONFIG1_ILIMIT_SHIFT) &
       FAULT_CONFIG1_ILIMIT_MASK) |
      ((static_cast<uint32_t>(candidate.lock_ilimit_code) <<
        FAULT_CONFIG1_LOCK_ILIMIT_SHIFT) &
       FAULT_CONFIG1_LOCK_ILIMIT_MASK) |
      ((static_cast<uint32_t>(candidate.hw_lock_ilimit_code) <<
        FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT) &
       FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK) |
      ((static_cast<uint32_t>(candidate.lock_ilimit_deglitch_code) <<
        FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT) &
       FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK);
    if (!this->parent_->update_bits32(REG_FAULT_CONFIG1, fault_cfg1_mask, fault_cfg1_value)) {
      return false;
    }

    const uint32_t fault_cfg2_mask = FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK |
                                     FAULT_CONFIG2_LOCK2_EN_MASK;
    uint32_t fault_cfg2_value =
      ((static_cast<uint32_t>(candidate.hw_lock_ilimit_deglitch_code) <<
        FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT) &
       FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK);
    if (candidate.abn_bemf_lock_enable) {
      fault_cfg2_value |= FAULT_CONFIG2_LOCK2_EN_MASK;
    }
    if (!this->parent_->update_bits32(REG_FAULT_CONFIG2, fault_cfg2_mask, fault_cfg2_value)) {
      return false;
    }

    const uint32_t startup2_mask = MOTOR_STARTUP2_OL_ILIMIT_MASK |
                                   MOTOR_STARTUP2_OL_ACC_A1_MASK |
                                   MOTOR_STARTUP2_OL_ACC_A2_MASK |
                                   MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK |
                                   MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK |
                                   MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK;
    uint32_t startup2_value =
      ((static_cast<uint32_t>(candidate.open_loop_ilimit_code) <<
        MOTOR_STARTUP2_OL_ILIMIT_SHIFT) &
       MOTOR_STARTUP2_OL_ILIMIT_MASK) |
      ((static_cast<uint32_t>(candidate.open_loop_accel_a1_code) <<
        MOTOR_STARTUP2_OL_ACC_A1_SHIFT) &
       MOTOR_STARTUP2_OL_ACC_A1_MASK) |
      ((static_cast<uint32_t>(candidate.open_loop_accel_a2_code) <<
        MOTOR_STARTUP2_OL_ACC_A2_SHIFT) &
       MOTOR_STARTUP2_OL_ACC_A2_MASK) |
      ((static_cast<uint32_t>(candidate.handoff_code) <<
        MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT) &
       MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK) |
      ((static_cast<uint32_t>(candidate.theta_error_ramp_code) <<
        MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_SHIFT) &
       MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK);
    if (candidate.auto_handoff_enable) {
      startup2_value |= MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK;
    }
    if (!this->parent_->update_bits32(REG_MOTOR_STARTUP2, startup2_mask, startup2_value)) {
      return false;
    }

    const uint32_t int_algo2_value =
      (static_cast<uint32_t>(candidate.cl_slow_acc_code) << INT_ALGO_2_CL_SLOW_ACC_SHIFT) &
      INT_ALGO_2_CL_SLOW_ACC_MASK;
    return this->parent_->update_bits32(
      REG_INT_ALGO_2,
      INT_ALGO_2_CL_SLOW_ACC_MASK,
      int_algo2_value
    );
  }

  void log_tune_candidate_(const InitialTuneCandidate& candidate, const char* prefix) const {
    ESP_LOGI(TUNING_TAG, "%s", prefix);
    ESP_LOGI(
      TUNING_TAG,
      "  phase_current_limit_percent: %u",
      static_cast<unsigned>(tables::LOCK_ILIMIT_PERCENT[candidate.phase_ilimit_code & 0x0Fu])
    );
    ESP_LOGI(
      TUNING_TAG,
      "  lock_ilimit_percent: %u",
      static_cast<unsigned>(tables::LOCK_ILIMIT_PERCENT[candidate.lock_ilimit_code & 0x0Fu])
    );
    ESP_LOGI(
      TUNING_TAG,
      "  hw_lock_ilimit_percent: %u",
      static_cast<unsigned>(tables::LOCK_ILIMIT_PERCENT[candidate.hw_lock_ilimit_code & 0x0Fu])
    );
    ESP_LOGI(
      TUNING_TAG,
      "  open_loop_ilimit_percent: %u",
      static_cast<unsigned>(tables::LOCK_ILIMIT_PERCENT[candidate.open_loop_ilimit_code & 0x0Fu])
    );
    ESP_LOGI(
      TUNING_TAG,
      "  open_loop_accel_hz_per_s: %.2f",
      this->parent_ != nullptr
        ? this->parent_->comms_client_.decode_open_loop_accel_hz_per_s(candidate.open_loop_accel_a1_code)
        : 0.0f
    );
    ESP_LOGI(
      TUNING_TAG,
      "  open_loop_accel2_hz_per_s2: %.2f",
      tables::OPEN_LOOP_ACCEL2_HZ_PER_S2[candidate.open_loop_accel_a2_code & 0x0Fu]
    );
    ESP_LOGI(
      TUNING_TAG,
      "  open_to_closed_handoff_percent: %.1f",
      this->parent_ != nullptr
        ? this->parent_->comms_client_.decode_open_to_closed_handoff_percent(candidate.handoff_code)
        : 0.0f
    );
    ESP_LOGI(
      TUNING_TAG,
      "  theta_error_ramp_rate: %.2f",
      tables::THETA_ERROR_RAMP_RATE[candidate.theta_error_ramp_code & 0x07u]
    );
    ESP_LOGI(
      TUNING_TAG,
      "  cl_slow_acc_hz_per_s: %.1f",
      tables::CL_SLOW_ACC_HZ_PER_S[candidate.cl_slow_acc_code & 0x0Fu]
    );
    ESP_LOGI(
      TUNING_TAG,
      "  lock_ilimit_deglitch_ms: %.1f",
      tables::LOCK_ILIMIT_DEGLITCH_MS[candidate.lock_ilimit_deglitch_code & 0x0Fu]
    );
    ESP_LOGI(
      TUNING_TAG,
      "  hw_lock_ilimit_deglitch_us: %u",
      static_cast<unsigned>(
        tables::HW_LOCK_ILIMIT_DEGLITCH_US[candidate.hw_lock_ilimit_deglitch_code & 0x07u]
      )
    );
    ESP_LOGI(TUNING_TAG, "  auto_handoff_enable: %s", candidate.auto_handoff_enable ? "true" : "false");
    ESP_LOGI(TUNING_TAG, "  abn_bemf_lock_enable: %s", candidate.abn_bemf_lock_enable ? "true" : "false");
  }

  bool candidates_equal_(const InitialTuneCandidate& a, const InitialTuneCandidate& b) const {
    return a.phase_ilimit_code == b.phase_ilimit_code && a.lock_ilimit_code == b.lock_ilimit_code &&
           a.hw_lock_ilimit_code == b.hw_lock_ilimit_code &&
           a.open_loop_ilimit_code == b.open_loop_ilimit_code &&
           a.open_loop_accel_a1_code == b.open_loop_accel_a1_code &&
           a.open_loop_accel_a2_code == b.open_loop_accel_a2_code && a.handoff_code == b.handoff_code &&
           a.theta_error_ramp_code == b.theta_error_ramp_code &&
           a.cl_slow_acc_code == b.cl_slow_acc_code &&
           a.lock_ilimit_deglitch_code == b.lock_ilimit_deglitch_code &&
           a.hw_lock_ilimit_deglitch_code == b.hw_lock_ilimit_deglitch_code &&
           a.auto_handoff_enable == b.auto_handoff_enable &&
           a.abn_bemf_lock_enable == b.abn_bemf_lock_enable;
  }

  void append_refined_candidate_(const InitialTuneCandidate& candidate) {
    if (this->refinement_candidate_count_ >= MAX_REFINED_CANDIDATES) {
      return;
    }
    for (uint8_t i = 0u; i < this->refinement_candidate_count_; i++) {
      if (this->candidates_equal_(this->refinement_candidates_[i], candidate)) {
        return;
      }
    }
    this->refinement_candidates_[this->refinement_candidate_count_++] = candidate;
  }

  float candidate_avg_tracking_error_hz_(const CandidateQualityMetrics& metrics) const {
    if (metrics.sample_count == 0u) {
      return 999.0f;
    }
    return metrics.tracking_error_sum_hz / static_cast<float>(metrics.sample_count);
  }

  float candidate_avg_mismatch_error_hz_(const CandidateQualityMetrics& metrics) const {
    if (metrics.sample_count == 0u) {
      return 999.0f;
    }
    return metrics.mismatch_error_sum_hz / static_cast<float>(metrics.sample_count);
  }

  void reset_candidate_quality_metrics_() {
    this->current_candidate_metrics_ = CandidateQualityMetrics{};
  }

  void record_candidate_quality_sample_(float commanded_hz, float fdbk_hz, float fg_hz, bool unstable) {
    const float commanded_abs = std::fabs(commanded_hz);
    const float fdbk_abs = std::fabs(fdbk_hz);
    const float fg_abs = std::fabs(fg_hz);
    const float command_basis = std::max(20.0f, commanded_abs);
    const float pair_avg = (fdbk_abs + fg_abs) * 0.5f;
    const float pair_max = std::max(fdbk_abs, fg_abs);

    if (this->current_candidate_metrics_.sample_count < std::numeric_limits<uint16_t>::max()) {
      this->current_candidate_metrics_.sample_count++;
    }
    this->current_candidate_metrics_.tracking_error_sum_hz += std::fabs(pair_avg - command_basis);
    this->current_candidate_metrics_.mismatch_error_sum_hz += std::fabs(fdbk_abs - fg_abs);

    const float speed_ratio = pair_max / command_basis;
    if (speed_ratio > this->current_candidate_metrics_.peak_speed_ratio) {
      this->current_candidate_metrics_.peak_speed_ratio = speed_ratio;
    }
    if (unstable && this->current_candidate_metrics_.unstable_count < std::numeric_limits<uint16_t>::max()) {
      this->current_candidate_metrics_.unstable_count++;
    }
  }

  void record_candidate_quality_telemetry_miss_() {
    if (this->current_candidate_metrics_.telemetry_miss_count < std::numeric_limits<uint16_t>::max()) {
      this->current_candidate_metrics_.telemetry_miss_count++;
    }
  }

  int32_t score_candidate_(
    const InitialTuneCandidate& candidate, uint32_t reach_ms, const CandidateQualityMetrics& metrics
  ) const {
    const float avg_tracking_hz = this->candidate_avg_tracking_error_hz_(metrics);
    const float avg_mismatch_hz = this->candidate_avg_mismatch_error_hz_(metrics);
    const float overspeed_ratio = std::max(0.0f, metrics.peak_speed_ratio - 1.10f);
    const float quality_penalty = (avg_tracking_hz * SCORE_TRACKING_ERROR_WEIGHT) +
                                  (avg_mismatch_hz * SCORE_MISMATCH_ERROR_WEIGHT) +
                                  (overspeed_ratio * SCORE_OVERSPEED_RATIO_WEIGHT);

    int32_t score = -static_cast<int32_t>(reach_ms);
    score -= static_cast<int32_t>(quality_penalty + 0.5f);
    score -= static_cast<int32_t>(metrics.unstable_count) * SCORE_UNSTABLE_SAMPLE_PENALTY;
    score -= static_cast<int32_t>(metrics.telemetry_miss_count) * SCORE_TELEMETRY_MISS_PENALTY;
    if (metrics.sample_count == 0u) {
      score -= SCORE_NO_SAMPLE_PENALTY;
    }
    score += static_cast<int32_t>(candidate.open_loop_accel_a1_code) * ACCEL_A1_BONUS_SCORE;
    return score;
  }

  bool read_speed_triplet_hz_(float& ref_ol_hz, float& fdbk_hz, float& fg_hz, float& max_speed_hz) const {
    if (this->parent_ == nullptr) {
      return false;
    }

    uint32_t closed_loop4 = 0;
    max_speed_hz = this->parent_->cfg_max_speed_set_
                     ? this->parent_->comms_client_.decode_max_speed_hz(this->parent_->cfg_max_speed_code_)
                     : 0.0f;
    if (this->parent_->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
      const uint16_t max_speed_code = static_cast<uint16_t>(
        (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >>
        CLOSED_LOOP4_MAX_SPEED_SHIFT
      );
      max_speed_hz = this->parent_->comms_client_.decode_max_speed_hz(max_speed_code);
    }
    if (max_speed_hz <= 0.0f) {
      return false;
    }

    uint32_t raw_ref = 0;
    uint32_t raw_fdbk = 0;
    uint32_t raw_fg = 0;
    if (!this->parent_->read_reg32(REG_SPEED_REF_OPEN_LOOP, raw_ref) ||
        !this->parent_->read_reg32(REG_SPEED_FDBK, raw_fdbk) ||
        !this->parent_->read_reg32(REG_FG_SPEED_FDBK, raw_fg)) {
      return false;
    }

    ref_ol_hz = this->parent_->comms_client_.decode_speed_hz(static_cast<int32_t>(raw_ref), max_speed_hz);
    fdbk_hz = this->parent_->comms_client_.decode_speed_hz(static_cast<int32_t>(raw_fdbk), max_speed_hz);
    fg_hz = this->parent_->comms_client_.decode_fg_speed_hz(raw_fg, max_speed_hz);
    return true;
  }

  bool handoff_feedback_unstable_(float commanded_hz, float fdbk_hz, float fg_hz) const {
    const float commanded_abs = std::fabs(commanded_hz);
    const float fdbk_abs = std::fabs(fdbk_hz);
    const float fg_abs = std::fabs(fg_hz);
    const float command_basis = std::max(20.0f, commanded_abs);

    const bool severe_overspeed = fdbk_abs > (command_basis * 2.2f) && fg_abs > (command_basis * 2.2f);

    const float pair_max = std::max(fdbk_abs, fg_abs);
    const bool fdbk_fg_mismatch =
      pair_max > 20.0f && std::fabs(fdbk_abs - fg_abs) > std::max(35.0f, pair_max * 0.55f);

    return severe_overspeed || fdbk_fg_mismatch;
  }

  bool handoff_feedback_stable_(float commanded_hz, float fdbk_hz, float fg_hz) const {
    const float commanded_abs = std::fabs(commanded_hz);
    const float fdbk_abs = std::fabs(fdbk_hz);
    const float fg_abs = std::fabs(fg_hz);
    const float command_basis = std::max(20.0f, commanded_abs);

    const float min_expected = command_basis * 0.2f;
    const float max_expected = command_basis * 1.9f;
    const bool both_in_expected_band = fdbk_abs >= min_expected && fg_abs >= min_expected &&
                                       fdbk_abs <= max_expected && fg_abs <= max_expected;

    const float pair_max = std::max(fdbk_abs, fg_abs);
    const bool pair_consistent = pair_max <= 20.0f || std::fabs(fdbk_abs - fg_abs) <= std::max(25.0f, pair_max * 0.45f);

    return both_in_expected_band && pair_consistent;
  }

  bool estimate_handoff_time_ms_(const InitialTuneCandidate& candidate, float& est_handoff_ms) const {
    est_handoff_ms = 0.0f;
    if (this->parent_ == nullptr) {
      return false;
    }

    float max_speed_hz = this->parent_->cfg_max_speed_set_
                           ? this->parent_->comms_client_.decode_max_speed_hz(this->parent_->cfg_max_speed_code_)
                           : 0.0f;
    uint32_t closed_loop4 = 0;
    if (this->parent_->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
      const uint16_t max_speed_code = static_cast<uint16_t>(
        (closed_loop4 & CLOSED_LOOP4_MAX_SPEED_MASK) >>
        CLOSED_LOOP4_MAX_SPEED_SHIFT
      );
      max_speed_hz = this->parent_->comms_client_.decode_max_speed_hz(max_speed_code);
    }
    if (max_speed_hz <= 0.0f) {
      return false;
    }

    const float handoff_percent =
      this->parent_->comms_client_.decode_open_to_closed_handoff_percent(candidate.handoff_code & 0x1Fu);
    const float handoff_hz = max_speed_hz * handoff_percent / 100.0f;
    const float ol_accel_hz_per_s =
      this->parent_->comms_client_.decode_open_loop_accel_hz_per_s(candidate.open_loop_accel_a1_code);
    if (ol_accel_hz_per_s <= 0.0f) {
      return false;
    }

    est_handoff_ms = (handoff_hz / ol_accel_hz_per_s) * 1000.0f;
    return std::isfinite(est_handoff_ms) && est_handoff_ms >= 0.0f;
  }

  uint32_t candidate_open_loop_dwell_timeout_ms_(const InitialTuneCandidate& candidate) const {
    float est_handoff_ms = 0.0f;
    if (!this->estimate_handoff_time_ms_(candidate, est_handoff_ms)) {
      return INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    }
    const float timeout_ms = est_handoff_ms * INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_SCALE +
                             static_cast<float>(INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MARGIN_MS);
    const float timeout_clamped_ms = std::clamp(
      timeout_ms,
      static_cast<float>(INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MIN_MS),
      static_cast<float>(INITIAL_TUNE_OPEN_LOOP_DWELL_TIMEOUT_MAX_MS)
    );
    return static_cast<uint32_t>(timeout_clamped_ms + 0.5f);
  }

  uint32_t candidate_monitor_timeout_ms_(const InitialTuneCandidate& candidate) const {
    float est_handoff_ms = 0.0f;
    if (!this->estimate_handoff_time_ms_(candidate, est_handoff_ms)) {
      return INITIAL_TUNE_MONITOR_TIMEOUT_MS;
    }

    const float est_to_handoff_ms = est_handoff_ms * INITIAL_TUNE_MONITOR_TIMEOUT_SCALE;
    const float timeout_ms =
      est_to_handoff_ms + static_cast<float>(INITIAL_TUNE_SUCCESS_HOLD_MS + INITIAL_TUNE_MONITOR_TIMEOUT_MARGIN_MS);
    const float timeout_clamped_ms = std::clamp(
      timeout_ms,
      static_cast<float>(INITIAL_TUNE_MONITOR_TIMEOUT_MIN_MS),
      static_cast<float>(INITIAL_TUNE_MONITOR_TIMEOUT_MAX_MS)
    );
    return static_cast<uint32_t>(timeout_clamped_ms + 0.5f);
  }

  bool active_pass_is_refinement_() const {
    return this->refinement_active_;
  }

  uint8_t active_candidate_index_() const {
    return this->active_pass_is_refinement_() ? this->refinement_candidate_index_
                                              : this->initial_tune_candidate_index_;
  }

  uint8_t active_candidate_count_() const {
    return this->active_pass_is_refinement_() ? this->refinement_candidate_count_
                                              : static_cast<uint8_t>(TUNING_INITIAL_TUNE_CANDIDATE_COUNT);
  }

  bool active_candidates_exhausted_() const {
    return this->active_candidate_index_() >= this->active_candidate_count_();
  }

  const InitialTuneCandidate& active_candidate_() const {
    if (this->active_pass_is_refinement_()) {
      return this->refinement_candidates_[this->refinement_candidate_index_];
    }
    return TUNING_INITIAL_TUNE_CANDIDATES[this->initial_tune_candidate_index_];
  }

  void advance_active_candidate_() {
    if (this->active_pass_is_refinement_()) {
      this->refinement_candidate_index_++;
    } else {
      this->initial_tune_candidate_index_++;
    }
  }

  bool begin_refinement_(
    const InitialTuneCandidate& baseline,
    uint32_t baseline_reach_ms,
    const CandidateQualityMetrics& baseline_metrics
  ) {
    this->refinement_active_ = false;
    this->refinement_candidate_index_ = 0u;
    this->refinement_candidate_count_ = 0u;
    this->refinement_best_candidate_valid_ = true;
    this->refinement_best_candidate_ = baseline;
    this->refinement_best_reach_ms_ = baseline_reach_ms;
    this->refinement_best_metrics_ = baseline_metrics;
    this->refinement_best_score_ = this->score_candidate_(baseline, baseline_reach_ms, baseline_metrics);

    this->append_refined_candidate_(baseline);  // index 0 baseline marker (already known successful)

    InitialTuneCandidate candidate = baseline;
    candidate.open_loop_accel_a1_code =
      static_cast<uint8_t>(std::min<int>(15, static_cast<int>(baseline.open_loop_accel_a1_code) + 1));
    this->append_refined_candidate_(candidate);

    candidate = baseline;
    candidate.handoff_code = static_cast<uint8_t>(
      std::min<int>(31, static_cast<int>(baseline.handoff_code) + 2)
    );
    this->append_refined_candidate_(candidate);

    candidate = baseline;
    candidate.handoff_code =
      static_cast<uint8_t>(std::max<int>(0, static_cast<int>(baseline.handoff_code) - 2));
    this->append_refined_candidate_(candidate);

    if (baseline.open_loop_accel_a1_code > 0u) {
      candidate = baseline;
      candidate.open_loop_accel_a1_code = static_cast<uint8_t>(baseline.open_loop_accel_a1_code - 1u);
      this->append_refined_candidate_(candidate);
    }

    if (this->refinement_candidate_count_ <= 1u) {
      return false;
    }

    this->refinement_active_ = true;
    this->refinement_candidate_index_ = 0u;

    ESP_LOGI(
      TUNING_TAG,
      "Initial tune baseline reached closed-loop in %ums; starting refinement sweep (%u variant candidate(s))",
      static_cast<unsigned>(baseline_reach_ms),
      static_cast<unsigned>(this->refinement_candidate_count_ - 1u)
    );
    return true;
  }

  void record_successful_candidate_(
    const InitialTuneCandidate& candidate,
    uint32_t reach_ms,
    const CandidateQualityMetrics& metrics
  ) {
    const float avg_tracking_hz = this->candidate_avg_tracking_error_hz_(metrics);
    const float avg_mismatch_hz = this->candidate_avg_mismatch_error_hz_(metrics);
    const int32_t score = this->score_candidate_(candidate, reach_ms, metrics);
    ESP_LOGI(
      TUNING_TAG,
      "Initial tune candidate success: reach=%ums score=%d accel=%.2fHz/s handoff=%.1f%% "
      "handoff_samples=%u avg_track=%.1fHz avg_mismatch=%.1fHz peak_ratio=%.2f",
      static_cast<unsigned>(reach_ms),
      static_cast<int>(score),
      this->parent_->comms_client_.decode_open_loop_accel_hz_per_s(candidate.open_loop_accel_a1_code),
      this->parent_->comms_client_.decode_open_to_closed_handoff_percent(candidate.handoff_code),
      static_cast<unsigned>(metrics.sample_count),
      avg_tracking_hz,
      avg_mismatch_hz,
      metrics.peak_speed_ratio
    );
    if (!this->refinement_best_candidate_valid_ || score > this->refinement_best_score_) {
      this->refinement_best_candidate_valid_ = true;
      this->refinement_best_candidate_ = candidate;
      this->refinement_best_reach_ms_ = reach_ms;
      this->refinement_best_score_ = score;
      this->refinement_best_metrics_ = metrics;
    }
  }

  void finalize_initial_tune_success_() {
    this->initial_tune_active_ = false;
    this->initial_tune_stage_ = InitialTuneStage::IDLE;
    this->initial_tune_waiting_fault_recovery_ = false;
    this->initial_tune_last_fault_clear_ms_ = 0u;
    this->initial_tune_closed_loop_seen_ = false;
    this->handoff_unstable_counter_ = 0u;
    this->handoff_stable_counter_ = 0u;
    this->handoff_telemetry_miss_counter_ = 0u;
    this->current_candidate_metrics_ = CandidateQualityMetrics{};

    if (!this->refinement_best_candidate_valid_) {
      ESP_LOGW(TUNING_TAG, "Initial tune ended without a successful candidate result");
      return;
    }

    ESP_LOGI(
      TUNING_TAG,
      "Initial tune success: best candidate reach=%ums score=%d handoff_samples=%u avg_track=%.1fHz "
      "avg_mismatch=%.1fHz peak_ratio=%.2f",
      static_cast<unsigned>(this->refinement_best_reach_ms_),
      static_cast<int>(this->refinement_best_score_),
      static_cast<unsigned>(this->refinement_best_metrics_.sample_count),
      this->candidate_avg_tracking_error_hz_(this->refinement_best_metrics_),
      this->candidate_avg_mismatch_error_hz_(this->refinement_best_metrics_),
      this->refinement_best_metrics_.peak_speed_ratio
    );
    this->log_tune_candidate_(
      this->refinement_best_candidate_,
      "Initial tune success: copy these keys into your YAML under mcf8329a:"
    );
  }

  void fail_initial_tune_candidate_(const char* reason, uint32_t now) {
    const char* phase = this->active_pass_is_refinement_() ? "refinement" : "discovery";
    ESP_LOGW(
      TUNING_TAG,
      "Initial tune %s candidate %u/%u failed: %s",
      phase,
      static_cast<unsigned>(this->active_candidate_index_() + 1u),
      static_cast<unsigned>(this->active_candidate_count_()),
      reason
    );
    this->clear_runtime_speed_command_("initial_tune_fail");
    this->parent_->pulse_clear_faults();
    this->initial_tune_waiting_fault_recovery_ = false;
    this->initial_tune_last_fault_clear_ms_ = 0u;
    this->initial_tune_closed_loop_seen_ = false;
    this->handoff_unstable_counter_ = 0u;
    this->handoff_stable_counter_ = 0u;
    this->handoff_telemetry_miss_counter_ = 0u;
    this->current_candidate_metrics_ = CandidateQualityMetrics{};
    this->initial_tune_stage_ = InitialTuneStage::COOLDOWN;
    this->initial_tune_stage_started_ms_ = now;
  }

  void finalize_initial_tune_failure_() {
    this->initial_tune_active_ = false;
    this->initial_tune_stage_ = InitialTuneStage::IDLE;
    this->initial_tune_waiting_fault_recovery_ = false;
    this->initial_tune_last_fault_clear_ms_ = 0u;
    this->initial_tune_closed_loop_seen_ = false;
    this->handoff_unstable_counter_ = 0u;
    this->handoff_stable_counter_ = 0u;
    this->handoff_telemetry_miss_counter_ = 0u;
    this->current_candidate_metrics_ = CandidateQualityMetrics{};
    ESP_LOGW(
      TUNING_TAG,
      "Initial tune failed: no discovery candidate reached closed-loop at %.1f%% command",
      INITIAL_TUNE_SPEED_PERCENT
    );
  }

  void process_initial_tune_(bool fault_active, uint32_t now) {
    if (this->active_candidates_exhausted_()) {
      if (this->active_pass_is_refinement_()) {
        this->finalize_initial_tune_success_();
      } else {
        this->finalize_initial_tune_failure_();
      }
      return;
    }

    const InitialTuneCandidate& candidate = this->active_candidate_();
    const char* phase = this->active_pass_is_refinement_() ? "refinement" : "discovery";

    switch (this->initial_tune_stage_) {
      case InitialTuneStage::APPLY: {
        ESP_LOGI(
          TUNING_TAG,
          "Initial tune %s candidate %u/%u",
          phase,
          static_cast<unsigned>(this->active_candidate_index_() + 1u),
          static_cast<unsigned>(this->active_candidate_count_())
        );
        if (!this->apply_tune_candidate_(candidate)) {
          this->fail_initial_tune_candidate_("register write failed", now);
          return;
        }
        this->log_tune_candidate_(candidate, "Initial tune candidate values:");
        this->initial_tune_stage_ = InitialTuneStage::START;
        this->initial_tune_stage_started_ms_ = now;
        return;
      }

      case InitialTuneStage::START: {
        if ((now - this->initial_tune_stage_started_ms_) < INITIAL_TUNE_SETTLE_MS) {
          return;
        }
        if (!this->parent_->apply_speed_command_(INITIAL_TUNE_SPEED_PERCENT, "initial_tune_start", true)) {
          this->fail_initial_tune_candidate_("speed command failed", now);
          return;
        }
        this->initial_tune_stage_ = InitialTuneStage::MONITOR;
        this->initial_tune_stage_started_ms_ = now;
        this->current_candidate_start_ms_ = now;
        this->current_candidate_open_loop_dwell_timeout_ms_ =
          this->candidate_open_loop_dwell_timeout_ms_(candidate);
        this->current_candidate_monitor_timeout_ms_ = std::max(
          this->candidate_monitor_timeout_ms_(candidate),
          this->current_candidate_open_loop_dwell_timeout_ms_ + INITIAL_TUNE_SUCCESS_HOLD_MS + 1000u
        );
        this->current_candidate_reach_ms_ = this->current_candidate_monitor_timeout_ms_;
        this->initial_tune_closed_loop_seen_ = false;
        this->initial_tune_closed_loop_seen_ms_ = 0u;
        this->handoff_unstable_counter_ = 0u;
        this->handoff_stable_counter_ = 0u;
        this->handoff_telemetry_miss_counter_ = 0u;
        this->reset_candidate_quality_metrics_();
        ESP_LOGI(
          TUNING_TAG,
          "Initial tune %s candidate %u/%u monitor timeout=%ums open_loop_dwell_timeout=%ums",
          phase,
          static_cast<unsigned>(this->active_candidate_index_() + 1u),
          static_cast<unsigned>(this->active_candidate_count_()),
          static_cast<unsigned>(this->current_candidate_monitor_timeout_ms_),
          static_cast<unsigned>(this->current_candidate_open_loop_dwell_timeout_ms_)
        );
        return;
      }

      case InitialTuneStage::MONITOR: {
        if (fault_active) {
          this->fail_initial_tune_candidate_("fault asserted", now);
          return;
        }

        uint16_t algo_state = 0;
        if (this->read_algorithm_state_(algo_state) && this->is_closed_loop_state_(algo_state)) {
          if (!this->initial_tune_closed_loop_seen_) {
            this->initial_tune_closed_loop_seen_ = true;
            this->initial_tune_closed_loop_seen_ms_ = now;
            this->current_candidate_reach_ms_ =
              now >= this->current_candidate_start_ms_ ? (now - this->current_candidate_start_ms_) : 0u;
            this->handoff_unstable_counter_ = 0u;
            this->handoff_stable_counter_ = 0u;
            this->handoff_telemetry_miss_counter_ = 0u;
            this->reset_candidate_quality_metrics_();
            ESP_LOGI(
              TUNING_TAG,
              "Initial tune %s candidate %u/%u entered %s (reach=%ums)",
              phase,
              static_cast<unsigned>(this->active_candidate_index_() + 1u),
              static_cast<unsigned>(this->active_candidate_count_()),
              this->parent_->algorithm_state_to_string_(algo_state),
              static_cast<unsigned>(this->current_candidate_reach_ms_)
            );
          } else if ((now - this->initial_tune_closed_loop_seen_ms_) >= INITIAL_TUNE_HANDOFF_GUARD_GRACE_MS) {
            float ref_ol_hz = 0.0f;
            float fdbk_hz = 0.0f;
            float fg_hz = 0.0f;
            float max_speed_hz = 0.0f;
            if (this->read_speed_triplet_hz_(ref_ol_hz, fdbk_hz, fg_hz, max_speed_hz)) {
              this->handoff_telemetry_miss_counter_ = 0u;
              const float commanded_hz =
                (std::fabs(this->parent_->speed_applied_percent_) / 100.0f) * max_speed_hz;
              const bool handoff_unstable = this->handoff_feedback_unstable_(commanded_hz, fdbk_hz, fg_hz);
              this->record_candidate_quality_sample_(commanded_hz, fdbk_hz, fg_hz, handoff_unstable);
              if (!handoff_unstable) {
                this->handoff_unstable_counter_ = 0u;
                if (this->handoff_feedback_stable_(commanded_hz, fdbk_hz, fg_hz)) {
                  if (this->handoff_stable_counter_ < 0xFFu) {
                    this->handoff_stable_counter_++;
                  }
                } else {
                  this->handoff_stable_counter_ = 0u;
                }
              } else {
                this->handoff_unstable_counter_++;
                this->handoff_stable_counter_ = 0u;
                ESP_LOGW(
                  TUNING_TAG,
                  "Initial tune handoff guard: unstable sample %u/%u cmd=%.1fHz ref_ol=%.1fHz fdbk=%.1fHz fg=%.1fHz",
                  static_cast<unsigned>(this->handoff_unstable_counter_),
                  static_cast<unsigned>(HANDOFF_UNSTABLE_REJECT_COUNT),
                  commanded_hz,
                  ref_ol_hz,
                  fdbk_hz,
                  fg_hz
                );
                if (this->handoff_unstable_counter_ >= HANDOFF_UNSTABLE_REJECT_COUNT) {
                  this->fail_initial_tune_candidate_("handoff feedback unstable", now);
                  return;
                }
              }
            } else {
              this->handoff_stable_counter_ = 0u;
              if (this->handoff_telemetry_miss_counter_ < 0xFFu) {
                this->handoff_telemetry_miss_counter_++;
              }
              this->record_candidate_quality_telemetry_miss_();
              ESP_LOGW(
                TUNING_TAG,
                "Initial tune handoff guard: speed telemetry unavailable sample %u/%u",
                static_cast<unsigned>(this->handoff_telemetry_miss_counter_),
                static_cast<unsigned>(HANDOFF_TELEMETRY_MISS_REJECT_COUNT)
              );
              if (this->handoff_telemetry_miss_counter_ >= HANDOFF_TELEMETRY_MISS_REJECT_COUNT) {
                this->fail_initial_tune_candidate_("handoff telemetry unavailable", now);
                return;
              }
            }
          }

          if ((now - this->initial_tune_closed_loop_seen_ms_) >= INITIAL_TUNE_SUCCESS_HOLD_MS) {
            if (this->handoff_stable_counter_ < HANDOFF_STABLE_ACCEPT_COUNT) {
              return;
            }
            const CandidateQualityMetrics candidate_metrics = this->current_candidate_metrics_;
            this->clear_runtime_speed_command_("initial_tune_success");
            this->record_successful_candidate_(candidate, this->current_candidate_reach_ms_, candidate_metrics);

            if (!this->active_pass_is_refinement_()) {
              const bool has_refinement =
                this->begin_refinement_(candidate, this->current_candidate_reach_ms_, candidate_metrics);
              if (!has_refinement) {
                this->finalize_initial_tune_success_();
                return;
              }
            }

            this->initial_tune_closed_loop_seen_ = false;
            this->initial_tune_stage_ = InitialTuneStage::COOLDOWN;
            this->initial_tune_stage_started_ms_ = now;
            return;
          }
        } else {
          this->initial_tune_closed_loop_seen_ = false;
          this->initial_tune_closed_loop_seen_ms_ = 0u;
          this->handoff_unstable_counter_ = 0u;
          this->handoff_stable_counter_ = 0u;
          this->handoff_telemetry_miss_counter_ = 0u;
          this->reset_candidate_quality_metrics_();
        }

        if (!this->initial_tune_closed_loop_seen_ &&
            (now - this->current_candidate_start_ms_) >= this->current_candidate_open_loop_dwell_timeout_ms_) {
          this->fail_initial_tune_candidate_("open-loop dwell timeout (heating guard)", now);
          return;
        }

        if ((now - this->initial_tune_stage_started_ms_) >= this->current_candidate_monitor_timeout_ms_) {
          this->fail_initial_tune_candidate_("timeout waiting for closed-loop", now);
          return;
        }
        return;
      }

      case InitialTuneStage::COOLDOWN: {
        if ((now - this->initial_tune_stage_started_ms_) < INITIAL_TUNE_COOLDOWN_MS) {
          return;
        }
        if (this->parent_ != nullptr && (fault_active || this->parent_->severe_fault_speed_lockout_)) {
          if (!this->initial_tune_waiting_fault_recovery_) {
            this->initial_tune_waiting_fault_recovery_ = true;
            ESP_LOGW(
              TUNING_TAG,
              "Initial tune waiting for fault recovery before retrying candidate: "
              "fault_active=%s lockout=%s",
              YESNO(fault_active),
              YESNO(this->parent_->severe_fault_speed_lockout_)
            );
          }
          if (this->initial_tune_last_fault_clear_ms_ == 0u ||
              (now - this->initial_tune_last_fault_clear_ms_) >=
                INITIAL_TUNE_FAULT_RECOVERY_PULSE_INTERVAL_MS) {
            this->parent_->pulse_clear_faults();
            this->initial_tune_last_fault_clear_ms_ = now;
          }
          if (fault_active || this->parent_->severe_fault_speed_lockout_) {
            return;
          }
        }
        if (this->initial_tune_waiting_fault_recovery_) {
          ESP_LOGI(TUNING_TAG, "Initial tune fault recovery complete; resuming candidate sweep");
          this->initial_tune_waiting_fault_recovery_ = false;
          this->initial_tune_last_fault_clear_ms_ = 0u;
        }
        this->advance_active_candidate_();
        if (this->active_candidates_exhausted_()) {
          if (this->active_pass_is_refinement_()) {
            this->finalize_initial_tune_success_();
          } else {
            this->finalize_initial_tune_failure_();
          }
          return;
        }
        this->initial_tune_stage_ = InitialTuneStage::APPLY;
        this->initial_tune_stage_started_ms_ = now;
        return;
      }

      case InitialTuneStage::IDLE:
      default:
        this->initial_tune_active_ = false;
        return;
    }
  }

  void log_mpet_results_() const {
    uint32_t closed_loop2 = 0;
    uint32_t closed_loop3 = 0;
    uint32_t closed_loop4 = 0;
    if (!this->parent_->read_reg32(REG_CLOSED_LOOP2, closed_loop2) ||
        !this->parent_->read_reg32(REG_CLOSED_LOOP3, closed_loop3) ||
        !this->parent_->read_reg32(REG_CLOSED_LOOP4, closed_loop4)) {
      ESP_LOGW(TUNING_TAG, "MPET finished but failed to read parameter registers");
      return;
    }

    const uint8_t motor_res = static_cast<uint8_t>(
      (closed_loop2 & CLOSED_LOOP2_MOTOR_RES_MASK) >> CLOSED_LOOP2_MOTOR_RES_SHIFT
    );
    const uint8_t motor_ind = static_cast<uint8_t>(
      (closed_loop2 & CLOSED_LOOP2_MOTOR_IND_MASK) >> CLOSED_LOOP2_MOTOR_IND_SHIFT
    );
    const uint8_t motor_bemf_const = static_cast<uint8_t>(
      (closed_loop3 & CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK) >>
      CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT
    );
    const uint16_t speed_loop_kp_code = static_cast<uint16_t>(
      (((closed_loop3 & CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK) >>
        CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT)
       << 7) |
      ((closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK) >>
       CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT)
    );
    const uint16_t speed_loop_ki_code = static_cast<uint16_t>(
      (closed_loop4 & CLOSED_LOOP4_SPD_LOOP_KI_MASK) >>
      CLOSED_LOOP4_SPD_LOOP_KI_SHIFT
    );

    ESP_LOGI(
      TUNING_TAG,
      "MPET result: motor_res=%u motor_ind=%u motor_bemf_const=0x%02X speed_loop_kp_code=%u speed_loop_ki_code=%u",
      static_cast<unsigned>(motor_res),
      static_cast<unsigned>(motor_ind),
      static_cast<unsigned>(motor_bemf_const),
      static_cast<unsigned>(speed_loop_kp_code),
      static_cast<unsigned>(speed_loop_ki_code)
    );
    ESP_LOGI(TUNING_TAG, "MPET result: copy these keys into your YAML under mcf8329a:");
    ESP_LOGI(TUNING_TAG, "  motor_bemf_const: 0x%02X", static_cast<unsigned>(motor_bemf_const));
    ESP_LOGI(TUNING_TAG, "  speed_loop_kp_code: %u", static_cast<unsigned>(speed_loop_kp_code));
    ESP_LOGI(TUNING_TAG, "  speed_loop_ki_code: %u", static_cast<unsigned>(speed_loop_ki_code));
  }

  void record_mpet_state_(bool algo_state_ok, uint16_t algo_state) {
    if (!algo_state_ok) {
      return;
    }
    if (algo_state < 32u) {
      this->mpet_states_seen_mask_ |= (1u << algo_state);
    }
  }

  bool read_mpet_status_(uint32_t& status) const {
    if (this->parent_ == nullptr) {
      return false;
    }
    return this->parent_->read_reg32(REG_ALGO_STATUS_MPET, status);
  }

  void log_mpet_profile_(const char* prefix) const {
    if (this->parent_ == nullptr) {
      return;
    }
    uint8_t mpet_curr_code =
      this->parent_->cfg_mpet_open_loop_curr_ref_set_ ? this->parent_->cfg_mpet_open_loop_curr_ref_ : 0u;
    uint8_t mpet_speed_code =
      this->parent_->cfg_mpet_open_loop_speed_ref_set_ ? this->parent_->cfg_mpet_open_loop_speed_ref_ : 0u;
    uint8_t mpet_slew_code =
      this->parent_->cfg_mpet_open_loop_slew_set_ ? this->parent_->cfg_mpet_open_loop_slew_ : 0u;
    bool mpet_use_dedicated = this->parent_->cfg_mpet_use_dedicated_params_set_ &&
                              this->parent_->cfg_mpet_use_dedicated_params_;

    uint32_t int_algo1 = 0;
    if (this->parent_->read_reg32(REG_INT_ALGO_1, int_algo1)) {
      mpet_curr_code = static_cast<uint8_t>(
        (int_algo1 & INT_ALGO_1_MPET_OPEN_LOOP_CURR_REF_MASK) >>
        INT_ALGO_1_MPET_OPEN_LOOP_CURR_REF_SHIFT
      );
      mpet_speed_code = static_cast<uint8_t>(
        (int_algo1 & INT_ALGO_1_MPET_OPEN_LOOP_SPEED_REF_MASK) >>
        INT_ALGO_1_MPET_OPEN_LOOP_SPEED_REF_SHIFT
      );
      mpet_slew_code = static_cast<uint8_t>(
        (int_algo1 & INT_ALGO_1_MPET_OPEN_LOOP_SLEW_RATE_MASK) >>
        INT_ALGO_1_MPET_OPEN_LOOP_SLEW_RATE_SHIFT
      );
    }

    uint32_t int_algo2 = 0;
    if (this->parent_->read_reg32(REG_INT_ALGO_2, int_algo2)) {
      mpet_use_dedicated =
        (int_algo2 & INT_ALGO_2_MPET_KE_MEAS_PARAMETER_SELECT_MASK) != 0u;
    }

    const uint32_t mpet_timeout_ms =
      this->parent_->mpet_timeout_ms_ > 0u ? this->parent_->mpet_timeout_ms_
                                           : DEFAULT_MPET_RUN_TIMEOUT_MS;
    ESP_LOGI(
      TUNING_TAG,
      "%s profile: dedicated=%s curr_ref=%u(%u%%) speed_ref=%u(%u%%) slew=%u(%.1fHz/s) timeout=%us",
      prefix,
      YESNO(mpet_use_dedicated),
      static_cast<unsigned>(mpet_curr_code),
      static_cast<unsigned>(tables::MPET_OPEN_LOOP_CURR_REF_PERCENT[mpet_curr_code & 0x07u]),
      static_cast<unsigned>(mpet_speed_code),
      static_cast<unsigned>(tables::MPET_OPEN_LOOP_SPEED_REF_PERCENT[mpet_speed_code & 0x03u]),
      static_cast<unsigned>(mpet_slew_code),
      tables::MPET_OPEN_LOOP_SLEW_HZ_PER_S[mpet_slew_code & 0x07u],
      static_cast<unsigned>(mpet_timeout_ms / 1000u)
    );
  }

  void log_mpet_status_tick_(uint32_t now, uint16_t algo_state, bool algo_state_ok) {
    if (this->mpet_last_status_log_ms_ != 0u &&
        (now - this->mpet_last_status_log_ms_) < MPET_STATUS_LOG_INTERVAL_MS) {
      return;
    }
    this->mpet_last_status_log_ms_ = now;

    uint32_t mpet_status = 0;
    const bool mpet_status_ok = this->read_mpet_status_(mpet_status);
    if (mpet_status_ok) {
      this->mpet_last_status_raw_ = mpet_status;
      this->mpet_last_status_valid_ = true;
    }

    const bool mpet_ke_status =
      mpet_status_ok && ((mpet_status & ALGO_STATUS_MPET_KE_STATUS_MASK) != 0u);
    const bool mpet_mech_status =
      mpet_status_ok && ((mpet_status & ALGO_STATUS_MPET_MECH_STATUS_MASK) != 0u);
    const uint8_t mpet_pwm_freq_code = mpet_status_ok
                                         ? static_cast<uint8_t>(
                                             (mpet_status & ALGO_STATUS_MPET_PWM_FREQ_MASK) >>
                                             ALGO_STATUS_MPET_PWM_FREQ_SHIFT
                                           )
                                         : 0u;

    float ref_ol_hz = 0.0f;
    float fdbk_hz = 0.0f;
    float fg_hz = 0.0f;
    float max_speed_hz = 0.0f;
    const bool speed_ok = this->read_speed_triplet_hz_(ref_ol_hz, fdbk_hz, fg_hz, max_speed_hz);
    const uint32_t elapsed_ms = now - this->mpet_characterization_started_ms_;

    if (speed_ok) {
      ESP_LOGI(
        TUNING_TAG,
        "MPET status: elapsed=%us state=0x%04X(%s) mpet_status=%s(0x%08X) "
        "ke=%s mech=%s pwm_freq_code=%u ref_ol=%.1fHz fdbk=%.1fHz fg=%.1fHz",
        static_cast<unsigned>(elapsed_ms / 1000u),
        static_cast<unsigned>(algo_state),
        algo_state_ok ? this->parent_->algorithm_state_to_string_(algo_state) : "unavailable",
        YESNO(mpet_status_ok),
        mpet_status,
        YESNO(mpet_ke_status),
        YESNO(mpet_mech_status),
        static_cast<unsigned>(mpet_pwm_freq_code),
        ref_ol_hz,
        fdbk_hz,
        fg_hz
      );
    } else {
      ESP_LOGI(
        TUNING_TAG,
        "MPET status: elapsed=%us state=0x%04X(%s) mpet_status=%s(0x%08X) "
        "ke=%s mech=%s pwm_freq_code=%u speed=unavailable",
        static_cast<unsigned>(elapsed_ms / 1000u),
        static_cast<unsigned>(algo_state),
        algo_state_ok ? this->parent_->algorithm_state_to_string_(algo_state) : "unavailable",
        YESNO(mpet_status_ok),
        mpet_status,
        YESNO(mpet_ke_status),
        YESNO(mpet_mech_status),
        static_cast<unsigned>(mpet_pwm_freq_code)
      );
    }
  }

  void log_mpet_summary_(const char* outcome, uint32_t now, uint16_t algo_state, bool algo_state_ok) {
    uint32_t mpet_status = this->mpet_last_status_raw_;
    bool mpet_status_ok = this->mpet_last_status_valid_;
    uint32_t latest_status = 0;
    if (this->read_mpet_status_(latest_status)) {
      mpet_status = latest_status;
      mpet_status_ok = true;
      this->mpet_last_status_raw_ = latest_status;
      this->mpet_last_status_valid_ = true;
    }

    const bool mpet_ke_status =
      mpet_status_ok && ((mpet_status & ALGO_STATUS_MPET_KE_STATUS_MASK) != 0u);
    const bool mpet_mech_status =
      mpet_status_ok && ((mpet_status & ALGO_STATUS_MPET_MECH_STATUS_MASK) != 0u);
    const uint8_t mpet_pwm_freq_code = mpet_status_ok
                                         ? static_cast<uint8_t>(
                                             (mpet_status & ALGO_STATUS_MPET_PWM_FREQ_MASK) >>
                                             ALGO_STATUS_MPET_PWM_FREQ_SHIFT
                                           )
                                         : 0u;
    const uint32_t elapsed_ms = now - this->mpet_characterization_started_ms_;
    ESP_LOGI(
      TUNING_TAG,
      "MPET summary[%s]: elapsed=%us states_mask=0x%08X last_state=0x%04X(%s) "
      "mpet_status=%s(0x%08X) ke=%s mech=%s pwm_freq_code=%u",
      outcome,
      static_cast<unsigned>(elapsed_ms / 1000u),
      this->mpet_states_seen_mask_,
      static_cast<unsigned>(algo_state),
      algo_state_ok ? this->parent_->algorithm_state_to_string_(algo_state) : "unavailable",
      YESNO(mpet_status_ok),
      mpet_status,
      YESNO(mpet_ke_status),
      YESNO(mpet_mech_status),
      static_cast<unsigned>(mpet_pwm_freq_code)
    );
    this->log_mpet_profile_("MPET summary");
  }

  void process_mpet_characterization_(bool fault_active, uint32_t now) {
    uint16_t algo_state = 0;
    const bool algo_state_ok = this->read_algorithm_state_(algo_state);
    this->record_mpet_state_(algo_state_ok, algo_state);
    this->log_mpet_status_tick_(now, algo_state, algo_state_ok);

    if (algo_state_ok && algo_state == 0x0017u) {
      this->mpet_characterization_active_ = false;
      this->clear_runtime_speed_command_("mpet_done");
      (void)this->parent_->clear_mpet_bits_("mpet_done");
      this->log_mpet_summary_("done", now, algo_state, algo_state_ok);
      ESP_LOGI(TUNING_TAG, "MPET characterization completed successfully");
      this->log_mpet_results_();
      return;
    }

    const bool mpet_fault_state = algo_state_ok && algo_state == 0x0018u;
    const bool motor_fault_state = algo_state_ok && algo_state == 0x000Eu;
    if (mpet_fault_state || (fault_active && motor_fault_state)) {
      this->mpet_characterization_active_ = false;
      this->clear_runtime_speed_command_("mpet_fault");
      (void)this->parent_->clear_mpet_bits_("mpet_fault");
      this->parent_->pulse_clear_faults();
      this->log_mpet_summary_("fault", now, algo_state, algo_state_ok);
      ESP_LOGW(
        TUNING_TAG,
        "MPET characterization failed in state 0x%04X(%s)",
        static_cast<unsigned>(algo_state),
        algo_state_ok ? this->parent_->algorithm_state_to_string_(algo_state) : "unknown"
      );
      return;
    }

    const uint32_t mpet_timeout_ms =
      this->parent_ != nullptr && this->parent_->mpet_timeout_ms_ > 0u
        ? this->parent_->mpet_timeout_ms_
        : DEFAULT_MPET_RUN_TIMEOUT_MS;
    if ((now - this->mpet_characterization_started_ms_) >= mpet_timeout_ms) {
      this->mpet_characterization_active_ = false;
      this->clear_runtime_speed_command_("mpet_timeout");
      (void)this->parent_->clear_mpet_bits_("mpet_timeout");
      this->parent_->pulse_clear_faults();
      this->log_mpet_summary_("timeout", now, algo_state, algo_state_ok);
      ESP_LOGW(
        TUNING_TAG,
        "MPET characterization timed out after %us (last state: %s)",
        static_cast<unsigned>(mpet_timeout_ms / 1000u),
        algo_state_ok ? this->parent_->algorithm_state_to_string_(algo_state) : "unavailable"
      );
    }
  }

  MCF8329AComponent* parent_{nullptr};
  bool initial_tune_active_{false};
  InitialTuneStage initial_tune_stage_{InitialTuneStage::IDLE};
  uint8_t initial_tune_candidate_index_{0u};
  uint32_t initial_tune_stage_started_ms_{0u};
  bool initial_tune_waiting_fault_recovery_{false};
  uint32_t initial_tune_last_fault_clear_ms_{0u};
  bool initial_tune_closed_loop_seen_{false};
  uint32_t initial_tune_closed_loop_seen_ms_{0u};
  uint32_t current_candidate_start_ms_{0u};
  uint32_t current_candidate_reach_ms_{INITIAL_TUNE_MONITOR_TIMEOUT_MS};
  uint32_t current_candidate_monitor_timeout_ms_{INITIAL_TUNE_MONITOR_TIMEOUT_MS};
  uint32_t current_candidate_open_loop_dwell_timeout_ms_{INITIAL_TUNE_MONITOR_TIMEOUT_MS};
  uint8_t handoff_unstable_counter_{0u};
  uint8_t handoff_stable_counter_{0u};
  uint8_t handoff_telemetry_miss_counter_{0u};
  CandidateQualityMetrics current_candidate_metrics_{};
  bool refinement_active_{false};
  uint8_t refinement_candidate_index_{0u};
  uint8_t refinement_candidate_count_{0u};
  InitialTuneCandidate refinement_candidates_[MAX_REFINED_CANDIDATES]{};
  bool refinement_best_candidate_valid_{false};
  InitialTuneCandidate refinement_best_candidate_{};
  CandidateQualityMetrics refinement_best_metrics_{};
  int32_t refinement_best_score_{std::numeric_limits<int32_t>::min()};
  uint32_t refinement_best_reach_ms_{INITIAL_TUNE_MONITOR_TIMEOUT_MS};
  bool mpet_characterization_active_{false};
  uint32_t mpet_characterization_started_ms_{0u};
  uint32_t mpet_last_status_log_ms_{0u};
  uint32_t mpet_states_seen_mask_{0u};
  uint32_t mpet_last_status_raw_{0u};
  bool mpet_last_status_valid_{false};
};

MCF8329ATuningController::MCF8329ATuningController(MCF8329AComponent* parent)
  : impl_(new Impl(parent)) {}

MCF8329ATuningController::~MCF8329ATuningController() {
  delete this->impl_;
  this->impl_ = nullptr;
}

void MCF8329ATuningController::reset() {
  if (this->impl_ != nullptr) {
    this->impl_->reset();
  }
}

void MCF8329ATuningController::start_initial_tune() {
  if (this->impl_ != nullptr) {
    this->impl_->start_initial_tune();
  }
}

void MCF8329ATuningController::start_mpet_characterization() {
  if (this->impl_ != nullptr) {
    this->impl_->start_mpet_characterization();
  }
}

void MCF8329ATuningController::update(bool normal_operation_ready, bool fault_active) {
  if (this->impl_ != nullptr) {
    this->impl_->update(normal_operation_ready, fault_active);
  }
}

}  // namespace mcf8329a
}  // namespace esphome

#endif  // !MCF8329A_EMBED_IMPL || MCF8329A_EMBED_IMPL_INCLUDE
