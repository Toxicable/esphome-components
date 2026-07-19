#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "esc_higher_registers.h"

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace esc_higher {

class ESCHigherComponent;

class ESCHigherStartButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherStopButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherClearFaultsButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherEstopButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherRunBringupTestButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherRunBridgeStaticVectorTestButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherRunForcedTimerDiffPwmTestButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherApplyMotorConfigButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherBringupTestSelect : public select::Select, public Parented<ESCHigherComponent> {
 public:
  void control(const std::string& value) override;
};

class ESCHigherSpeedTargetNumber : public number::Number, public Parented<ESCHigherComponent> {
 public:
  void control(float value) override;
};

class ESCHigherComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  void set_speed_ramp_target_dhz(int32_t value) {
    speed_ramp_target_dhz_ = value;
  }
  void set_speed_ramp_time_ms(int32_t value) {
    speed_ramp_time_ms_ = value;
  }
  void set_bringup_test_id(uint8_t value) {
    bringup_test_id_ = value;
  }


  // Motor config setters
  void set_provision_config(bool v) { provision_config_ = v; }
  void set_mc_name(const char* s) { if (s) { std::memcpy(mc_name_, s, std::min(std::strlen(s), sizeof(mc_name_)-1)); mc_name_[sizeof(mc_name_)-1] = '\0'; } }
  void set_mc_pole_pairs(uint16_t v) { mc_pole_pairs_ = v; }
  void set_mc_rs_ohm(float v) { mc_rs_ohm_ = v; }
  void set_mc_ls_h(float v) { mc_ls_h_ = v; }
  void set_mc_ke_vll_rms_per_krpm(float v) { mc_ke_vll_rms_per_krpm_ = v; }
  void set_mc_max_current_mA(uint16_t v) { mc_max_current_mA_ = v; }
  void set_mc_startup_current_limit_mA(uint16_t v) { mc_startup_current_limit_mA_ = v; }
  void set_mc_run_current_limit_mA(uint16_t v) { mc_run_current_limit_mA_ = v; }
  void set_mc_max_speed_unit(uint16_t v) { mc_max_speed_unit_ = v; }
  void set_mc_observer_min_speed_unit(uint16_t v) { mc_observer_min_speed_unit_ = v; }
  void set_mc_observer_min_fly_speed_unit(uint16_t v) { mc_observer_min_fly_speed_unit_ = v; }
  void set_mc_startup_consistency_tests(uint16_t v) { mc_startup_consistency_tests_ = v; }
  void set_mc_variance_percentage(uint16_t v) { mc_variance_percentage_ = v; }
  void set_mc_speed_band_upper_16ths(uint16_t v) { mc_speed_band_upper_16ths_ = v; }
  void set_mc_speed_band_lower_16ths(uint16_t v) { mc_speed_band_lower_16ths_ = v; }
  void set_mc_bemf_consistency_gain(uint16_t v) { mc_bemf_consistency_gain_ = v; }
  void set_mc_bemf_consistency_tolerance(uint16_t v) { mc_bemf_consistency_tolerance_ = v; }
  void set_mc_transition_duration_ms(uint16_t v) { mc_transition_duration_ms_ = v; }
  void set_mc_speed_kp(int16_t v) { mc_speed_kp_ = v; }
  void set_mc_speed_ki(int16_t v) { mc_speed_ki_ = v; }
  void set_mc_iq_kp(int16_t v) { mc_iq_kp_ = v; }
  void set_mc_iq_ki(int16_t v) { mc_iq_ki_ = v; }
  void set_mc_id_kp(int16_t v) { mc_id_kp_ = v; }
  void set_mc_id_ki(int16_t v) { mc_id_ki_ = v; }
  void set_mc_revup(uint8_t idx, uint16_t duration_ms, int16_t final_speed_unit, int16_t final_current_mA);
  void set_mc_run_current_limit_dwell_ms(uint16_t v) { mc_run_current_limit_dwell_ms_ = v; }
  void set_mc_normal_start_guard_extra_ms(uint16_t v) { mc_normal_start_guard_extra_ms_ = v; }


  bool start_motor();
  bool stop_motor();
  bool clear_faults();
  bool estop();
  bool set_speed_ramp();
  bool run_bringup_test();
  bool run_bridge_static_vector_test();
  bool run_forced_timer_diff_pwm_test();
  bool apply_motor_config();
  bool set_speed_target_dhz_and_send(int32_t target_dhz);

  // Config provisioning
  bool config_begin(uint16_t size, uint8_t schema, uint32_t crc);
  bool config_write_chunk(uint16_t offset, const uint8_t* data, size_t len);
  bool config_validate();
  bool config_commit();
  bool config_erase();
  bool config_provision(const uint8_t* data, size_t len);

  bool config_provision_with_crc();


  void set_proto_major_sensor(sensor::Sensor* s) {
    proto_major_sensor_ = s;
  }
  void set_proto_minor_sensor(sensor::Sensor* s) {
    proto_minor_sensor_ = s;
  }
  void set_fw_major_sensor(sensor::Sensor* s) {
    fw_major_sensor_ = s;
  }
  void set_fw_minor_sensor(sensor::Sensor* s) {
    fw_minor_sensor_ = s;
  }
  void set_hw_id_sensor(sensor::Sensor* s) {
    hw_id_sensor_ = s;
  }
  void set_max_block_len_sensor(sensor::Sensor* s) {
    max_block_len_sensor_ = s;
  }
  void set_capabilities_sensor(sensor::Sensor* s) {
    capabilities_sensor_ = s;
  }

  void set_status_seq_sensor(sensor::Sensor* s) {
    status_seq_sensor_ = s;
  }
  void set_telemetry_seq_sensor(sensor::Sensor* s) {
    telemetry_seq_sensor_ = s;
  }
  void set_bringup_seq_sensor(sensor::Sensor* s) {
    bringup_seq_sensor_ = s;
  }
  void set_debug_seq_sensor(sensor::Sensor* s) {
    debug_seq_sensor_ = s;
  }
  void set_esc_state_sensor(sensor::Sensor* s) {
    esc_state_sensor_ = s;
  }
  void set_mc_state_sensor(sensor::Sensor* s) {
    mc_state_sensor_ = s;
  }
  void set_last_cmd_seq_sensor(sensor::Sensor* s) {
    last_cmd_seq_sensor_ = s;
  }
  void set_last_cmd_error_sensor(sensor::Sensor* s) {
    last_cmd_error_sensor_ = s;
  }
  void set_fault_detail_sensor(sensor::Sensor* s) {
    fault_detail_sensor_ = s;
  }
  void set_current_faults_sensor(sensor::Sensor* s) {
    current_faults_sensor_ = s;
  }
  void set_occurred_faults_sensor(sensor::Sensor* s) {
    occurred_faults_sensor_ = s;
  }
  void set_status_flags_sensor(sensor::Sensor* s) {
    status_flags_sensor_ = s;
  }
  void set_watchdog_ms_left_sensor(sensor::Sensor* s) {
    watchdog_ms_left_sensor_ = s;
  }

  void set_vbus_mv_sensor(sensor::Sensor* s) {
    vbus_mv_sensor_ = s;
  }
  void set_current_sensor(sensor::Sensor* s) {
    current_sensor_ = s;
  }
  void set_speed_dhz_sensor(sensor::Sensor* s) {
    speed_dhz_sensor_ = s;
  }
  void set_duty_centi_pct_sensor(sensor::Sensor* s) {
    duty_centi_pct_sensor_ = s;
  }
  void set_temp_mc_sensor(sensor::Sensor* s) {
    temp_mc_sensor_ = s;
  }
  void set_target_speed_dhz_sensor(sensor::Sensor* s) {
    target_speed_dhz_sensor_ = s;
  }
  void set_drive_limit_centi_pct_sensor(sensor::Sensor* s) {
    drive_limit_centi_pct_sensor_ = s;
  }
  void set_uptime_s_sensor(sensor::Sensor* s) {
    uptime_s_sensor_ = s;
  }
  void set_bringup_active_sensor(sensor::Sensor* s) {
    bringup_active_sensor_ = s;
  }
  void set_bringup_test_id_sensor(sensor::Sensor* s) {
    bringup_test_id_sensor_ = s;
  }
  void set_bringup_step_id_sensor(sensor::Sensor* s) {
    bringup_step_id_sensor_ = s;
  }
  void set_bringup_state_sensor(sensor::Sensor* s) {
    bringup_state_sensor_ = s;
  }
  void set_bringup_result_sensor(sensor::Sensor* s) {
    bringup_result_sensor_ = s;
  }
  void set_bringup_failure_code_sensor(sensor::Sensor* s) {
    bringup_failure_code_sensor_ = s;
  }
  void set_bringup_measured0_sensor(sensor::Sensor* s) {
    bringup_measured0_sensor_ = s;
  }
  void set_bringup_measured1_sensor(sensor::Sensor* s) {
    bringup_measured1_sensor_ = s;
  }
  void set_bringup_phase_a_count_sensor(sensor::Sensor* s) {
    bringup_phase_a_count_sensor_ = s;
  }
  void set_bringup_phase_b_count_sensor(sensor::Sensor* s) {
    bringup_phase_b_count_sensor_ = s;
  }
  void set_bringup_phase_c_count_sensor(sensor::Sensor* s) {
    bringup_phase_c_count_sensor_ = s;
  }
  void set_bringup_pwm_spread_ticks_sensor(sensor::Sensor* s) {
    bringup_pwm_spread_ticks_sensor_ = s;
  }
  void set_bringup_max_phase_current_ma_sensor(sensor::Sensor* s) {
    bringup_max_phase_current_ma_sensor_ = s;
  }
  void set_bringup_limit_min_sensor(sensor::Sensor* s) {
    bringup_limit_min_sensor_ = s;
  }
  void set_bringup_limit_max_sensor(sensor::Sensor* s) {
    bringup_limit_max_sensor_ = s;
  }
  void set_bringup_vbus_mv_at_test_sensor(sensor::Sensor* s) {
    bringup_vbus_mv_at_test_sensor_ = s;
  }
  void set_bringup_current_faults_at_test_sensor(sensor::Sensor* s) {
    bringup_current_faults_at_test_sensor_ = s;
  }
  void set_bringup_occurred_faults_at_test_sensor(sensor::Sensor* s) {
    bringup_occurred_faults_at_test_sensor_ = s;
  }
  void set_bringup_mc_state_at_test_sensor(sensor::Sensor* s) {
    bringup_mc_state_at_test_sensor_ = s;
  }
  void set_bringup_esc_state_at_test_sensor(sensor::Sensor* s) {
    bringup_esc_state_at_test_sensor_ = s;
  }
  void set_bringup_gd_ready_sensor(sensor::Sensor* s) {
    bringup_gd_ready_sensor_ = s;
  }
  void set_bringup_elapsed_ms_sensor(sensor::Sensor* s) {
    bringup_elapsed_ms_sensor_ = s;
  }
  void set_bringup_last_passed_step_sensor(sensor::Sensor* s) {
    bringup_last_passed_step_sensor_ = s;
  }
  void set_bringup_steps_total_sensor(sensor::Sensor* s) {
    bringup_steps_total_sensor_ = s;
  }
  void set_bringup_attempt_count_sensor(sensor::Sensor* s) {
    bringup_attempt_count_sensor_ = s;
  }
  void set_bringup_debug0_sensor(sensor::Sensor* s) {
    bringup_debug0_sensor_ = s;
  }
  void set_bringup_debug1_sensor(sensor::Sensor* s) {
    bringup_debug1_sensor_ = s;
  }
  void set_bringup_last_app_fault_detail_sensor(sensor::Sensor* s) {
    bringup_last_app_fault_detail_sensor_ = s;
  }
  void set_bringup_switch_over_ms_sensor(sensor::Sensor* s) {
    bringup_switch_over_ms_sensor_ = s;
  }
  void set_bringup_run_ms_sensor(sensor::Sensor* s) {
    bringup_run_ms_sensor_ = s;
  }
  void set_bringup_max_speed_dhz_sensor(sensor::Sensor* s) {
    bringup_max_speed_dhz_sensor_ = s;
  }
  void set_bringup_max_current_reference_ma_sensor(sensor::Sensor* s) {
    bringup_max_current_reference_ma_sensor_ = s;
  }
  void set_bringup_max_phase_current_reported_ma_sensor(sensor::Sensor* s) {
    bringup_max_phase_current_reported_ma_sensor_ = s;
  }
  void set_debug_v_alpha_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_alpha_raw_s16_sensor_ = s;
  }
  void set_debug_v_beta_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_beta_raw_s16_sensor_ = s;
  }
  void set_debug_v_q_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_q_raw_s16_sensor_ = s;
  }
  void set_debug_v_d_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_d_raw_s16_sensor_ = s;
  }
  void set_debug_v_u_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_u_raw_s16_sensor_ = s;
  }
  void set_debug_v_v_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_v_raw_s16_sensor_ = s;
  }
  void set_debug_v_w_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_w_raw_s16_sensor_ = s;
  }
  void set_debug_v_amp_raw_s16_sensor(sensor::Sensor* s) {
    debug_v_amp_raw_s16_sensor_ = s;
  }
  void set_debug_phase_ia_ma_sensor(sensor::Sensor* s) {
    debug_phase_ia_ma_sensor_ = s;
  }
  void set_debug_phase_ib_ma_sensor(sensor::Sensor* s) {
    debug_phase_ib_ma_sensor_ = s;
  }
  void set_debug_phase_ic_ma_sensor(sensor::Sensor* s) {
    debug_phase_ic_ma_sensor_ = s;
  }

  void set_esc_state_text_sensor(text_sensor::TextSensor* s) {
    esc_state_text_sensor_ = s;
  }
  void set_last_cmd_error_text_sensor(text_sensor::TextSensor* s) {
    last_cmd_error_text_sensor_ = s;
  }
  void set_fault_detail_text_sensor(text_sensor::TextSensor* s) {
    fault_detail_text_sensor_ = s;
  }
  void set_current_fault_text_sensor(text_sensor::TextSensor* s) {
    current_fault_text_sensor_ = s;
  }
  void set_status_flags_text_sensor(text_sensor::TextSensor* s) {
    status_flags_text_sensor_ = s;
  }
  void set_current_faults_text_sensor(text_sensor::TextSensor* s) {
    current_faults_text_sensor_ = s;
  }
  void set_occurred_faults_text_sensor(text_sensor::TextSensor* s) {
    occurred_faults_text_sensor_ = s;
  }
  void set_capabilities_text_sensor(text_sensor::TextSensor* s) {
    capabilities_text_sensor_ = s;
  }
  void set_mc_state_text_sensor(text_sensor::TextSensor* s) {
    mc_state_text_sensor_ = s;
  }
  void set_bringup_state_text_sensor(text_sensor::TextSensor* s) {
    bringup_state_text_sensor_ = s;
  }
  void set_bringup_result_text_sensor(text_sensor::TextSensor* s) {
    bringup_result_text_sensor_ = s;
  }
  void set_bringup_test_id_text_sensor(text_sensor::TextSensor* s) {
    bringup_test_id_text_sensor_ = s;
  }
  void set_bringup_current_faults_text_sensor(text_sensor::TextSensor* s) {
    bringup_current_faults_text_sensor_ = s;
  }
  void set_bringup_occurred_faults_text_sensor(text_sensor::TextSensor* s) {
    bringup_occurred_faults_text_sensor_ = s;
  }
  void set_debug_log_text_sensor(text_sensor::TextSensor* s) {
    debug_log_text_sensor_ = s;
  }
  void set_bringup_test_select(select::Select* s) {
    bringup_test_select_ = s;
  }

 protected:
  void maybe_log_command_result_(uint8_t last_cmd_seq, uint8_t last_cmd_error, uint8_t esc_state, uint8_t mc_state,
                                 uint8_t fault_detail, uint16_t current_faults, uint16_t occurred_faults);
  bool read_register_(::esc_higher_core::registers::RegisterId reg, uint8_t* out, size_t len);
  bool read_debug_info_(uint32_t* debug_seq, uint16_t* used_len, uint16_t* export_len, uint16_t* capacity,
                        uint16_t* dropped, uint16_t* crc16);
  bool read_debug_chunk_(uint16_t offset, uint8_t length, uint8_t* out);
  bool publish_debug_log_(uint32_t debug_seq, uint16_t export_len, uint16_t capacity, uint16_t dropped, uint16_t crc16);
  bool write_command_(::esc_higher_core::registers::CommandId opcode, int32_t param0, int32_t param1, int32_t param2, uint8_t* seq_out = nullptr);
  bool wait_for_command_result_(uint8_t seq, const char* label, uint32_t timeout_ms = 250);
  bool initialize_();
  bool configure_watchdog_();
  static uint16_t crc16_ccitt_false_(const uint8_t* data, size_t len);

  static uint16_t u16_(const uint8_t* b, size_t off) {
    return static_cast<uint16_t>(b[off]) | (static_cast<uint16_t>(b[off + 1]) << 8);
  }
  static int32_t i32_(const uint8_t* b, size_t off) {
    return static_cast<int32_t>(
      static_cast<uint32_t>(b[off]) | (static_cast<uint32_t>(b[off + 1]) << 8) |
      (static_cast<uint32_t>(b[off + 2]) << 16) | (static_cast<uint32_t>(b[off + 3]) << 24)
    );
  }
  static uint32_t u32_(const uint8_t* b, size_t off) {
    return static_cast<uint32_t>(b[off]) | (static_cast<uint32_t>(b[off + 1]) << 8) |
           (static_cast<uint32_t>(b[off + 2]) << 16) |
           (static_cast<uint32_t>(b[off + 3]) << 24);
  }
  static int16_t i16_(const uint8_t* b, size_t off) {
    return static_cast<int16_t>(u16_(b, off));
  }


  static constexpr uint16_t CAP_DEBUG_LOG = 1u << 7;
  static constexpr uint16_t DEBUG_BUFFER_SIZE = 4096;
  static constexpr uint8_t DEBUG_READ_CHUNK_SIZE = 64;

  static constexpr uint8_t BRINGUP_TEST_FULL_SPIN_SEQUENCE = 101;
  static constexpr uint8_t BRINGUP_TEST_BRIDGE_STATIC_VECTOR = 102;
  static constexpr uint8_t BRINGUP_TEST_FORCED_TIMER_DIFF_PWM = 103;
  static constexpr int32_t BRINGUP_TEST_FULL_SPIN_DURATION_MS = 5000;
  static constexpr int32_t BRINGUP_TEST_BRIDGE_STATIC_VECTOR_DURATION_MS = 50;
  static constexpr int32_t BRINGUP_TEST_FORCED_TIMER_DIFF_PWM_DURATION_MS = 1000;
  static constexpr int32_t BRINGUP_TEST_OPTIONS_DEFAULT = 0;
  static constexpr int32_t BRINGUP_OPT_ALLOW_FORCED_TIMER_DIFF_PWM = 1;

  // Config provisioning
  static constexpr uint8_t CONFIG_DATA_CHUNK_SIZE = 61;
  static constexpr uint8_t MOTOR_CONFIG_SCHEMA_VERSION = 3;

  uint8_t command_seq_{0};
  int32_t speed_ramp_target_dhz_{1000};
  int32_t speed_ramp_time_ms_{1000};

  // Motor config provisioning fields
  bool provision_config_{false};
  char mc_name_[32]{};
  uint16_t mc_pole_pairs_{0};
  float mc_rs_ohm_{0};
  float mc_ls_h_{0};
  float mc_ke_vll_rms_per_krpm_{0};
  uint16_t mc_max_current_mA_{0};
  uint16_t mc_startup_current_limit_mA_{0};
  uint16_t mc_run_current_limit_mA_{0};
  uint16_t mc_max_speed_unit_{0};
  uint16_t mc_observer_min_speed_unit_{0};
  uint16_t mc_observer_min_fly_speed_unit_{0};
  uint16_t mc_startup_consistency_tests_{8};
  uint16_t mc_variance_percentage_{10};
  uint16_t mc_speed_band_upper_16ths_{16};
  uint16_t mc_speed_band_lower_16ths_{16};
  uint16_t mc_bemf_consistency_gain_{128};
  uint16_t mc_bemf_consistency_tolerance_{256};
  uint16_t mc_transition_duration_ms_{500};
  int16_t mc_speed_kp_{100};
  int16_t mc_speed_ki_{10};
  int16_t mc_iq_kp_{50};
  int16_t mc_iq_ki_{5};
  int16_t mc_id_kp_{30};
  int16_t mc_id_ki_{3};
  struct {
    uint16_t duration_ms;
    int16_t final_speed_unit;
    int16_t final_current_mA;
  } mc_revup_[5]{{0}};
  uint8_t mc_revup_count_{0};
  uint16_t mc_run_current_limit_dwell_ms_{1000};
  uint16_t mc_normal_start_guard_extra_ms_{500};

  uint8_t bringup_test_id_{BRINGUP_TEST_FULL_SPIN_SEQUENCE};
  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};
  uint16_t capabilities_{0};
  uint8_t max_block_len_{0};
  bool debug_log_supported_{false};
  bool debug_log_read_failed_{false};
  uint8_t last_bringup_report_seq_{0xFF};
  uint8_t last_logged_cmd_seq_{0xFF};
  uint8_t last_logged_cmd_error_{0xFF};
  uint8_t last_logged_fault_detail_{0xFF};
  bool force_next_bringup_debug_read_{false};
  uint32_t last_heartbeat_ms_{0};

  sensor::Sensor* proto_major_sensor_{nullptr};
  sensor::Sensor* proto_minor_sensor_{nullptr};
  sensor::Sensor* fw_major_sensor_{nullptr};
  sensor::Sensor* fw_minor_sensor_{nullptr};
  sensor::Sensor* hw_id_sensor_{nullptr};
  sensor::Sensor* max_block_len_sensor_{nullptr};
  sensor::Sensor* capabilities_sensor_{nullptr};

  sensor::Sensor* status_seq_sensor_{nullptr};
  sensor::Sensor* telemetry_seq_sensor_{nullptr};
  sensor::Sensor* bringup_seq_sensor_{nullptr};
  sensor::Sensor* debug_seq_sensor_{nullptr};
  sensor::Sensor* esc_state_sensor_{nullptr};
  sensor::Sensor* mc_state_sensor_{nullptr};
  sensor::Sensor* last_cmd_seq_sensor_{nullptr};
  sensor::Sensor* last_cmd_error_sensor_{nullptr};
  sensor::Sensor* fault_detail_sensor_{nullptr};
  sensor::Sensor* current_faults_sensor_{nullptr};
  sensor::Sensor* occurred_faults_sensor_{nullptr};
  sensor::Sensor* status_flags_sensor_{nullptr};
  sensor::Sensor* watchdog_ms_left_sensor_{nullptr};

  sensor::Sensor* vbus_mv_sensor_{nullptr};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* speed_dhz_sensor_{nullptr};
  sensor::Sensor* duty_centi_pct_sensor_{nullptr};
  sensor::Sensor* temp_mc_sensor_{nullptr};
  sensor::Sensor* target_speed_dhz_sensor_{nullptr};
  sensor::Sensor* drive_limit_centi_pct_sensor_{nullptr};
  sensor::Sensor* uptime_s_sensor_{nullptr};

  sensor::Sensor* bringup_active_sensor_{nullptr};
  sensor::Sensor* bringup_test_id_sensor_{nullptr};
  sensor::Sensor* bringup_step_id_sensor_{nullptr};
  sensor::Sensor* bringup_state_sensor_{nullptr};
  sensor::Sensor* bringup_result_sensor_{nullptr};
  sensor::Sensor* bringup_failure_code_sensor_{nullptr};
  sensor::Sensor* bringup_measured0_sensor_{nullptr};
  sensor::Sensor* bringup_measured1_sensor_{nullptr};
  sensor::Sensor* bringup_phase_a_count_sensor_{nullptr};
  sensor::Sensor* bringup_phase_b_count_sensor_{nullptr};
  sensor::Sensor* bringup_phase_c_count_sensor_{nullptr};
  sensor::Sensor* bringup_pwm_spread_ticks_sensor_{nullptr};
  sensor::Sensor* bringup_max_phase_current_ma_sensor_{nullptr};
  sensor::Sensor* bringup_limit_min_sensor_{nullptr};
  sensor::Sensor* bringup_limit_max_sensor_{nullptr};
  sensor::Sensor* bringup_vbus_mv_at_test_sensor_{nullptr};
  sensor::Sensor* bringup_current_faults_at_test_sensor_{nullptr};
  sensor::Sensor* bringup_occurred_faults_at_test_sensor_{nullptr};
  sensor::Sensor* bringup_mc_state_at_test_sensor_{nullptr};
  sensor::Sensor* bringup_esc_state_at_test_sensor_{nullptr};
  sensor::Sensor* bringup_gd_ready_sensor_{nullptr};
  sensor::Sensor* bringup_elapsed_ms_sensor_{nullptr};
  sensor::Sensor* bringup_last_passed_step_sensor_{nullptr};
  sensor::Sensor* bringup_steps_total_sensor_{nullptr};
  sensor::Sensor* bringup_attempt_count_sensor_{nullptr};
  sensor::Sensor* bringup_debug0_sensor_{nullptr};
  sensor::Sensor* bringup_debug1_sensor_{nullptr};

  sensor::Sensor* bringup_last_app_fault_detail_sensor_{nullptr};
  sensor::Sensor* bringup_switch_over_ms_sensor_{nullptr};
  sensor::Sensor* bringup_run_ms_sensor_{nullptr};
  sensor::Sensor* bringup_max_speed_dhz_sensor_{nullptr};
  sensor::Sensor* bringup_max_current_reference_ma_sensor_{nullptr};
  sensor::Sensor* bringup_max_phase_current_reported_ma_sensor_{nullptr};


  sensor::Sensor* debug_v_alpha_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_beta_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_q_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_d_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_u_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_v_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_w_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_v_amp_raw_s16_sensor_{nullptr};
  sensor::Sensor* debug_phase_ia_ma_sensor_{nullptr};
  sensor::Sensor* debug_phase_ib_ma_sensor_{nullptr};
  sensor::Sensor* debug_phase_ic_ma_sensor_{nullptr};

  text_sensor::TextSensor* esc_state_text_sensor_{nullptr};
  text_sensor::TextSensor* last_cmd_error_text_sensor_{nullptr};
  text_sensor::TextSensor* fault_detail_text_sensor_{nullptr};
  text_sensor::TextSensor* current_fault_text_sensor_{nullptr};
  text_sensor::TextSensor* status_flags_text_sensor_{nullptr};
  text_sensor::TextSensor* current_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* occurred_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* mc_state_text_sensor_{nullptr};
  text_sensor::TextSensor* capabilities_text_sensor_{nullptr};
  text_sensor::TextSensor* bringup_state_text_sensor_{nullptr};
  text_sensor::TextSensor* bringup_result_text_sensor_{nullptr};
  text_sensor::TextSensor* bringup_test_id_text_sensor_{nullptr};
  text_sensor::TextSensor* bringup_current_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* bringup_occurred_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* debug_log_text_sensor_{nullptr};
  select::Select* bringup_test_select_{nullptr};
};

}  // namespace esc_higher
}  // namespace esphome
