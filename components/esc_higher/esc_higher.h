#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/number/number.h"
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

class ESCHigherSpeedTargetNumber : public number::Number, public Parented<ESCHigherComponent> {
 public:
  void control(float value) override;
};

class ESCHigherComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_disable_watchdog(bool value) {
    disable_watchdog_ = value;
  }
  void set_watchdog_timeout_ms(uint32_t value) {
    watchdog_timeout_ms_ = value;
  }
  void set_speed_ramp_target_dhz(int32_t value) {
    speed_ramp_target_dhz_ = value;
  }
  void set_speed_ramp_time_ms(int32_t value) {
    speed_ramp_time_ms_ = value;
  }
  void set_bringup_test_id(int32_t value) {
    bringup_test_id_ = value;
  }
  void set_bringup_test_duration_ms(int32_t value) {
    bringup_test_duration_ms_ = value;
  }
  void set_bringup_test_options(int32_t value) {
    bringup_test_options_ = value;
  }

  bool start_motor();
  bool stop_motor();
  bool clear_faults();
  bool estop();
  bool set_speed_ramp();
  bool run_bringup_test();
  bool set_speed_target_dhz_and_send(int32_t target_dhz);

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
  void set_ibus_ma_sensor(sensor::Sensor* s) {
    ibus_ma_sensor_ = s;
  }
  void set_motor_current_ma_sensor(sensor::Sensor* s) {
    motor_current_ma_sensor_ = s;
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
  void set_telemetry_debug0_sensor(sensor::Sensor* s) {
    telemetry_debug0_sensor_ = s;
  }
  void set_telemetry_debug1_sensor(sensor::Sensor* s) {
    telemetry_debug1_sensor_ = s;
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

 protected:
  bool read_register_(uint8_t reg, uint8_t* out, size_t len);
  bool write_command_(uint8_t opcode, int32_t param0, int32_t param1, int32_t param2);
  bool initialize_();
  bool configure_watchdog_();

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

  static constexpr uint8_t REG_ID = 0x00;
  static constexpr uint8_t REG_STATUS = 0x10;
  static constexpr uint8_t REG_COMMAND = 0x20;
  static constexpr uint8_t REG_TELEMETRY = 0x30;
  static constexpr uint8_t REG_BRINGUP = 0x40;
  static constexpr uint8_t REG_DEBUG_TELEMETRY = 0x50;

  static constexpr uint8_t OPCODE_START = 0x01;
  static constexpr uint8_t OPCODE_STOP = 0x02;
  static constexpr uint8_t OPCODE_CLEAR_FAULTS = 0x03;
  static constexpr uint8_t OPCODE_SET_SPEED_RAMP = 0x04;
  static constexpr uint8_t OPCODE_ESTOP = 0x05;
  static constexpr uint8_t OPCODE_SET_WATCHDOG = 0x07;
  static constexpr uint8_t OPCODE_RUN_BRINGUP_TEST = 0x09;

  uint8_t command_seq_{0};
  int32_t speed_ramp_target_dhz_{1000};
  int32_t speed_ramp_time_ms_{1000};
  int32_t bringup_test_id_{1};
  int32_t bringup_test_duration_ms_{5000};
  int32_t bringup_test_options_{0};
  bool disable_watchdog_{true};
  uint32_t watchdog_timeout_ms_{500};
  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};

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
  sensor::Sensor* ibus_ma_sensor_{nullptr};
  sensor::Sensor* motor_current_ma_sensor_{nullptr};
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
  sensor::Sensor* telemetry_debug0_sensor_{nullptr};
  sensor::Sensor* telemetry_debug1_sensor_{nullptr};

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
};

}  // namespace esc_higher
}  // namespace esphome
