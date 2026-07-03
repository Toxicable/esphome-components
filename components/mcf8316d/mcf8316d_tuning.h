#pragma once

#include <cstdint>

namespace esphome {
namespace mcf8316d {

class MCF8316DComponent;

class MCF8316DTuningController {
 public:
  explicit MCF8316DTuningController(MCF8316DComponent *parent) : parent_(parent) {}

  bool apply_startup_tune_profile();
  bool apply_hw_lock_report_only_profile();
  bool start_startup_current_sweep();
  bool start_scope_probe_test();
  void apply_post_comms_setup();
  bool needs_algorithm_state() const;
  void update(
    bool algorithm_state_valid,
    uint16_t algorithm_state,
    bool fault_active,
    bool fault_state_valid,
    bool controller_valid,
    uint32_t controller_fault_status,
    uint16_t volt_mag_raw
  );
  void log_mpet_entry_conditions(const char *context, uint32_t algo_debug2);

 private:
  bool ensure_buck_current_limit_for_manual_();
  bool seed_closed_loop_params_if_zero_();
  void log_mpet_diagnostics_(const char *context);
  bool apply_startup_sweep_current_limits_(uint32_t current_limit_code);
  bool begin_startup_sweep_step_();
  void schedule_startup_sweep_step_(uint32_t delay_ms);
  void process_startup_sweep_(
    bool algorithm_state_valid,
    uint16_t algorithm_state,
    bool fault_active,
    bool fault_state_valid,
    bool controller_valid,
    uint32_t controller_fault_status,
    uint16_t volt_mag_raw
  );
  bool begin_scope_probe_stage_();
  void process_scope_probe_test_(
    bool algorithm_state_valid,
    uint16_t algorithm_state,
    bool fault_active,
    bool fault_state_valid,
    bool controller_valid,
    uint32_t controller_fault_status,
    uint16_t volt_mag_raw
  );
  float scope_probe_stage_speed_percent_(uint8_t stage_index) const;
  uint32_t scope_probe_stage_hold_ms_(uint8_t stage_index) const;
  uint32_t startup_sweep_current_code_(uint8_t step_index) const;
  float current_limit_code_to_amps_(uint32_t current_limit_code) const;

  MCF8316DComponent *parent_;
  bool startup_sweep_active_{false};
  bool startup_sweep_step_pending_{false};
  bool scope_probe_test_active_{false};
  bool scope_probe_stage_pending_{false};
  uint8_t startup_sweep_step_index_{0};
  uint8_t startup_sweep_pass_count_{0};
  uint8_t scope_probe_stage_index_{0};
  uint32_t startup_sweep_step_start_ms_{0};
  uint32_t startup_sweep_next_step_due_ms_{0};
  uint32_t scope_probe_stage_start_ms_{0};
  uint32_t scope_probe_next_stage_due_ms_{0};
  uint32_t last_mpet_diag_log_ms_{0};
};

}  // namespace mcf8316d
}  // namespace esphome
