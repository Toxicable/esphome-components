#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mcf8329a {

class MCF8329AComponent;
class MCF8329ATuningController;

class MCF8329ABrakeSwitch : public switch_::Switch {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void write_state(bool state) override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329ADirectionSelect : public select::Select {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void control(const std::string& value) override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329ASpeedNumber : public number::Number {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void control(float value) override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329AClearFaultsButton : public button::Button {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329AWatchdogTickleButton : public button::Button {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329ATuneInitialParamsButton : public button::Button {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329ARunMPETButton : public button::Button {
 public:
  void set_parent(MCF8329AComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8329AComponent* parent_{nullptr};
};

class MCF8329AComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  ~MCF8329AComponent();
  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_reg32(uint16_t offset, uint32_t& value);
  bool read_reg16(uint16_t offset, uint16_t& value);
  bool write_reg32(uint16_t offset, uint32_t value);
  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value);

  bool set_brake_override(bool brake_on);
  bool set_direction_mode(const std::string& direction_mode);
  bool set_speed_percent(float speed_percent, const char* reason = "external");
  void pulse_clear_faults();
  void pulse_watchdog_tickle();
  void start_tune_initial_params();
  void start_mpet_characterization();

  void set_inter_byte_delay_us(uint32_t inter_byte_delay_us) {
    inter_byte_delay_us_ = inter_byte_delay_us;
  }
  void set_auto_tickle_watchdog(bool auto_tickle_watchdog) {
    auto_tickle_watchdog_ = auto_tickle_watchdog;
  }
  void set_clear_mpet_on_startup(bool clear_mpet_on_startup) {
    clear_mpet_on_startup_ = clear_mpet_on_startup;
  }
  void set_cfg_motor_bemf_const(uint8_t cfg_motor_bemf_const) {
    cfg_motor_bemf_const_ = cfg_motor_bemf_const;
    cfg_motor_bemf_const_set_ = true;
  }
  void set_cfg_brake_mode(uint8_t cfg_brake_mode) {
    cfg_brake_mode_ = cfg_brake_mode;
    cfg_brake_mode_set_ = true;
  }
  void set_cfg_brake_time(uint8_t cfg_brake_time) {
    cfg_brake_time_ = cfg_brake_time;
    cfg_brake_time_set_ = true;
  }
  void set_cfg_mode(uint8_t cfg_mode) {
    cfg_mode_ = cfg_mode;
    cfg_mode_set_ = true;
  }
  void set_cfg_align_time(uint8_t cfg_align_time) {
    cfg_align_time_ = cfg_align_time;
    cfg_align_time_set_ = true;
  }
  void set_cfg_csa_gain(uint8_t cfg_csa_gain) {
    cfg_csa_gain_ = cfg_csa_gain & 0x03u;
    cfg_csa_gain_set_ = true;
  }
  void set_cfg_base_current_code(uint16_t cfg_base_current_code) {
    cfg_base_current_code_ = cfg_base_current_code & 0x7FFFu;
    cfg_base_current_set_ = true;
  }
  void set_cfg_direction_mode(const std::string& cfg_direction_mode) {
    cfg_direction_mode_ = cfg_direction_mode;
    cfg_direction_mode_set_ = true;
  }
  void set_cfg_ilimit(uint8_t cfg_ilimit) {
    cfg_ilimit_ = cfg_ilimit;
    cfg_ilimit_set_ = true;
  }
  void set_cfg_align_or_slow_current_ilimit(uint8_t cfg_align_or_slow_current_ilimit) {
    cfg_align_or_slow_current_ilimit_ = cfg_align_or_slow_current_ilimit;
    cfg_align_or_slow_current_ilimit_set_ = true;
  }
  void set_cfg_lock_mode(uint8_t cfg_lock_mode) {
    cfg_lock_mode_ = cfg_lock_mode;
    cfg_lock_mode_set_ = true;
  }
  void set_cfg_lock_ilimit(uint8_t cfg_lock_ilimit) {
    cfg_lock_ilimit_ = cfg_lock_ilimit;
    cfg_lock_ilimit_set_ = true;
  }
  void set_cfg_hw_lock_ilimit(uint8_t cfg_hw_lock_ilimit) {
    cfg_hw_lock_ilimit_ = cfg_hw_lock_ilimit;
    cfg_hw_lock_ilimit_set_ = true;
  }
  void set_cfg_lock_retry_time(uint8_t cfg_lock_retry_time) {
    cfg_lock_retry_time_ = cfg_lock_retry_time;
    cfg_lock_retry_time_set_ = true;
  }
  void set_cfg_abn_speed_lock_enable(bool cfg_abn_speed_lock_enable) {
    cfg_abn_speed_lock_enable_ = cfg_abn_speed_lock_enable;
    cfg_abn_speed_lock_enable_set_ = true;
  }
  void set_cfg_abn_bemf_lock_enable(bool cfg_abn_bemf_lock_enable) {
    cfg_abn_bemf_lock_enable_ = cfg_abn_bemf_lock_enable;
    cfg_abn_bemf_lock_enable_set_ = true;
  }
  void set_cfg_no_motor_lock_enable(bool cfg_no_motor_lock_enable) {
    cfg_no_motor_lock_enable_ = cfg_no_motor_lock_enable;
    cfg_no_motor_lock_enable_set_ = true;
  }
  void set_cfg_lock_abn_speed_threshold(uint8_t cfg_lock_abn_speed_threshold) {
    cfg_lock_abn_speed_threshold_ = cfg_lock_abn_speed_threshold;
    cfg_lock_abn_speed_threshold_set_ = true;
  }
  void set_cfg_abnormal_bemf_threshold(uint8_t cfg_abnormal_bemf_threshold) {
    cfg_abnormal_bemf_threshold_ = cfg_abnormal_bemf_threshold;
    cfg_abnormal_bemf_threshold_set_ = true;
  }
  void set_cfg_no_motor_threshold(uint8_t cfg_no_motor_threshold) {
    cfg_no_motor_threshold_ = cfg_no_motor_threshold;
    cfg_no_motor_threshold_set_ = true;
  }
  void set_cfg_max_speed_code(uint16_t cfg_max_speed_code) {
    cfg_max_speed_code_ = cfg_max_speed_code & 0x3FFFu;
    cfg_max_speed_set_ = true;
  }
  void set_cfg_open_loop_ilimit(uint8_t cfg_open_loop_ilimit) {
    cfg_open_loop_ilimit_ = cfg_open_loop_ilimit;
    cfg_open_loop_ilimit_set_ = true;
  }
  void set_cfg_open_loop_limit_source(bool cfg_open_loop_limit_use_ilimit) {
    cfg_open_loop_limit_use_ilimit_ = cfg_open_loop_limit_use_ilimit;
    cfg_open_loop_limit_source_set_ = true;
  }
  void set_cfg_open_loop_accel(uint8_t cfg_open_loop_accel) {
    cfg_open_loop_accel_ = cfg_open_loop_accel;
    cfg_open_loop_accel_set_ = true;
  }
  void set_cfg_open_loop_accel2(uint8_t cfg_open_loop_accel2) {
    cfg_open_loop_accel2_ = cfg_open_loop_accel2 & 0x0Fu;
    cfg_open_loop_accel2_set_ = true;
  }
  void set_cfg_auto_handoff_enable(bool cfg_auto_handoff_enable) {
    cfg_auto_handoff_enable_ = cfg_auto_handoff_enable;
    cfg_auto_handoff_enable_set_ = true;
  }
  void set_cfg_open_to_closed_handoff_threshold(uint8_t cfg_open_to_closed_handoff_threshold
  ) {
    cfg_open_to_closed_handoff_threshold_ = cfg_open_to_closed_handoff_threshold & 0x1Fu;
    cfg_open_to_closed_handoff_threshold_set_ = true;
  }
  void set_cfg_theta_error_ramp_rate(uint8_t cfg_theta_error_ramp_rate) {
    cfg_theta_error_ramp_rate_ = cfg_theta_error_ramp_rate & 0x07u;
    cfg_theta_error_ramp_rate_set_ = true;
  }
  void set_cfg_cl_slow_acc(uint8_t cfg_cl_slow_acc) {
    cfg_cl_slow_acc_ = cfg_cl_slow_acc & 0x0Fu;
    cfg_cl_slow_acc_set_ = true;
  }
  void set_cfg_lock_ilimit_deglitch(uint8_t cfg_lock_ilimit_deglitch) {
    cfg_lock_ilimit_deglitch_ = cfg_lock_ilimit_deglitch & 0x0Fu;
    cfg_lock_ilimit_deglitch_set_ = true;
  }
  void set_cfg_hw_lock_ilimit_deglitch(uint8_t cfg_hw_lock_ilimit_deglitch) {
    cfg_hw_lock_ilimit_deglitch_ = cfg_hw_lock_ilimit_deglitch & 0x07u;
    cfg_hw_lock_ilimit_deglitch_set_ = true;
  }
  void set_cfg_speed_loop_kp_code(uint16_t cfg_speed_loop_kp_code) {
    cfg_speed_loop_kp_code_ = cfg_speed_loop_kp_code & 0x03FFu;
    cfg_speed_loop_kp_code_set_ = true;
  }
  void set_cfg_speed_loop_ki_code(uint16_t cfg_speed_loop_ki_code) {
    cfg_speed_loop_ki_code_ = cfg_speed_loop_ki_code & 0x03FFu;
    cfg_speed_loop_ki_code_set_ = true;
  }
  void set_speed_ramp_up_percent_per_s(float speed_ramp_up_percent_per_s) {
    speed_ramp_up_percent_per_s_ = speed_ramp_up_percent_per_s;
  }
  void set_speed_ramp_down_percent_per_s(float speed_ramp_down_percent_per_s) {
    speed_ramp_down_percent_per_s_ = speed_ramp_down_percent_per_s;
  }
  void set_start_boost_percent(float start_boost_percent) {
    start_boost_percent_ = start_boost_percent;
  }
  void set_start_boost_hold_ms(uint32_t start_boost_hold_ms) {
    start_boost_hold_ms_ = start_boost_hold_ms;
  }

  void set_brake_switch(MCF8329ABrakeSwitch* sw) {
    brake_switch_ = sw;
  }
  void set_direction_select(MCF8329ADirectionSelect* sel) {
    direction_select_ = sel;
  }
  void set_speed_number(MCF8329ASpeedNumber* num) {
    speed_number_ = num;
  }
  void set_fault_active_binary_sensor(binary_sensor::BinarySensor* s) {
    fault_active_binary_sensor_ = s;
  }
  void set_sys_enable_binary_sensor(binary_sensor::BinarySensor* s) {
    sys_enable_binary_sensor_ = s;
  }
  void set_vm_voltage_sensor(sensor::Sensor* s) {
    vm_voltage_sensor_ = s;
  }
  void set_duty_cmd_percent_sensor(sensor::Sensor* s) {
    duty_cmd_percent_sensor_ = s;
  }
  void set_volt_mag_percent_sensor(sensor::Sensor* s) {
    volt_mag_percent_sensor_ = s;
  }
  void set_motor_bemf_constant_sensor(sensor::Sensor* s) {
    motor_bemf_constant_sensor_ = s;
  }
  void set_speed_fdbk_hz_sensor(sensor::Sensor* s) {
    speed_fdbk_hz_sensor_ = s;
  }
  void set_speed_ref_open_loop_hz_sensor(sensor::Sensor* s) {
    speed_ref_open_loop_hz_sensor_ = s;
  }
  void set_fg_speed_fdbk_hz_sensor(sensor::Sensor* s) {
    fg_speed_fdbk_hz_sensor_ = s;
  }
 void set_current_fault_text_sensor(text_sensor::TextSensor* s) {
    current_fault_text_sensor_ = s;
  }

 protected:
  friend class MCF8329ATuningController;

  bool read_probe_and_publish_();
  bool establish_communications_(uint8_t attempts, uint32_t retry_delay_ms, bool log_retry_delays);
  bool probe_device_ack_(i2c::ErrorCode& error_code) const;
  bool scan_i2c_bus_();
  void process_deferred_startup_();
  void apply_post_comms_setup_();
  void recover_from_mcf_reset_if_needed_();
  bool apply_motor_config_();
  const char* i2c_error_to_string_(i2c::ErrorCode error_code) const;
  const char* mode_to_string_(uint8_t mode) const;
  const char* align_time_to_string_(uint8_t code) const;
  const char* brake_mode_to_string_(uint8_t code) const;
  const char* brake_time_to_string_(uint8_t code) const;
  float max_speed_code_to_hz_(uint16_t code) const;
  float open_loop_accel_code_to_hz_per_s_(uint8_t code) const;
  float open_to_closed_handoff_code_to_percent_(uint8_t code) const;
  const char* algorithm_state_to_string_(uint16_t state) const;
  const char* lock_mode_to_string_(uint8_t mode) const;
  const char* lock_retry_time_to_string_(uint8_t code) const;
  bool clear_mpet_bits_(const char* context);
  bool apply_speed_command_(float speed_percent, const char* reason, bool publish_number = true);
  void process_speed_command_ramp_();
  float speed_raw_to_hz_(int32_t raw, float max_speed_hz) const;
  float fg_speed_raw_to_hz_(uint32_t raw, float max_speed_hz) const;
  void log_mpet_bemf_diagnostics_();
  void log_hw_lock_diagnostics_();

  bool perform_read_(uint16_t offset, uint32_t& value);
  bool perform_read16_(uint16_t offset, uint16_t& value);
  bool perform_write_(uint16_t offset, uint32_t value);
  uint32_t build_control_word_(bool is_read, uint16_t offset, bool is_32bit) const;

  void publish_faults_(
    uint32_t gate_fault_status,
    bool gate_fault_valid,
    uint32_t controller_fault_status,
    bool controller_fault_valid
  );
  void log_algorithm_state_transition_(uint32_t algo_status, const char* context);
  void publish_algo_status_(uint32_t algo_status);
  const char* brake_input_to_string_(uint32_t brake_input_value) const;
  const char* direction_input_to_string_(uint32_t direction_input_value) const;
  bool severe_current_fault_active_(
    uint32_t controller_fault_status, bool controller_fault_valid
  ) const;
  void handle_fault_shutdown_(
    bool fault_active, uint32_t controller_fault_status, bool controller_fault_valid
  );

  static constexpr uint16_t REG_CONTROLLER_FAULT_STATUS = 0x00E2;
  static constexpr uint16_t REG_GATE_DRIVER_FAULT_STATUS = 0x00E0;
  static constexpr uint16_t REG_ALGO_STATUS = 0x00E4;
  static constexpr uint16_t REG_ALGORITHM_STATE = 0x0196;
  static constexpr uint16_t REG_MTR_PARAMS = 0x00E6;
  static constexpr uint16_t REG_ALGO_CTRL1 = 0x00EA;
  static constexpr uint16_t REG_ALGO_DEBUG1 = 0x00EC;
  static constexpr uint16_t REG_ALGO_DEBUG2 = 0x00EE;
  static constexpr uint16_t REG_PIN_CONFIG = 0x00A4;
  static constexpr uint16_t REG_PERI_CONFIG1 = 0x00AA;
  static constexpr uint16_t REG_MOTOR_STARTUP1 = 0x0084;
  static constexpr uint16_t REG_MOTOR_STARTUP2 = 0x0086;
  static constexpr uint16_t REG_CLOSED_LOOP3 = 0x008C;
  static constexpr uint16_t REG_CLOSED_LOOP4 = 0x008E;
  static constexpr uint16_t REG_CLOSED_LOOP2 = 0x008A;
  static constexpr uint16_t REG_GD_CONFIG1 = 0x00AC;
  static constexpr uint16_t REG_GD_CONFIG2 = 0x00AE;
  static constexpr uint16_t REG_FAULT_CONFIG1 = 0x0090;
  static constexpr uint16_t REG_FAULT_CONFIG2 = 0x0092;
  static constexpr uint16_t REG_INT_ALGO_2 = 0x00A2;
  static constexpr uint16_t REG_VM_VOLTAGE = 0x045C;
  static constexpr uint16_t REG_FG_SPEED_FDBK = 0x019C;
  static constexpr uint16_t REG_SPEED_REF_OPEN_LOOP = 0x0532;
  static constexpr uint16_t REG_SPEED_FDBK = 0x076E;

  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_MASK = (0x3u << 2);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_BRAKE = (0x1u << 2);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_NO_BRAKE = (0x2u << 2);

  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_MASK = (0x3u << 19);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_HARDWARE = (0x0u << 19);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CW = (0x1u << 19);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CCW = (0x2u << 19);

  static constexpr uint32_t ALGO_DEBUG1_OVERRIDE_MASK = (1u << 31);
  static constexpr uint32_t ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK = (0x7FFFu << 16);

  static constexpr uint32_t ALGO_CTRL1_CLR_FLT_MASK = (1u << 29);
  static constexpr uint32_t ALGO_CTRL1_WATCHDOG_TICKLE_MASK = (1u << 10);
  static constexpr uint32_t ALGO_DEBUG2_MPET_CMD_MASK = (1u << 5);
  static constexpr uint32_t ALGO_DEBUG2_MPET_KE_MASK = (1u << 2);
  static constexpr uint32_t ALGO_DEBUG2_MPET_MECH_MASK = (1u << 1);
  static constexpr uint32_t ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK = (1u << 0);
  static constexpr uint32_t ALGO_DEBUG2_MPET_ALL_MASK =
    ALGO_DEBUG2_MPET_CMD_MASK | ALGO_DEBUG2_MPET_KE_MASK | ALGO_DEBUG2_MPET_MECH_MASK |
    ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK;

  static constexpr uint32_t ALGO_STATUS_DUTY_CMD_MASK = (0x0FFFu << 4);
  static constexpr uint32_t ALGO_STATUS_DUTY_CMD_SHIFT = 4;
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_MASK = (0xFFFFu << 16);
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_SHIFT = 16;
  static constexpr uint32_t ALGO_STATUS_SYS_ENABLE_FLAG_MASK = (1u << 2);
  static constexpr uint32_t MTR_PARAMS_MOTOR_BEMF_CONST_MASK = (0xFFu << 16);
  static constexpr uint32_t MTR_PARAMS_MOTOR_BEMF_CONST_SHIFT = 16;

  static constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_MASK = (0x3u << 29);
  static constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_SHIFT = 29;
  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_MASK = (0xFu << 21);
  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_SHIFT = 21;
  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK = (0xFu << 17);
  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT = 17;
  static constexpr uint32_t MOTOR_STARTUP1_OL_ILIMIT_CONFIG_MASK = (1u << 3);
  static constexpr uint32_t MOTOR_STARTUP1_OL_ILIMIT_CONFIG_SHIFT = 3;
  static constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_MASK = (0xFu << 27);
  static constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_SHIFT = 27;
  static constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A1_MASK = (0xFu << 23);
  static constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A1_SHIFT = 23;
  static constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A2_MASK = (0xFu << 19);
  static constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A2_SHIFT = 19;
  static constexpr uint32_t MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK = (1u << 18);
  static constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK = (0x1Fu << 13);
  static constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT = 13;
  static constexpr uint32_t MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK = (0x7u << 0);
  static constexpr uint32_t MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_SHIFT = 0;

  static constexpr uint32_t CLOSED_LOOP2_MTR_STOP_MASK = (0x7u << 28);
  static constexpr uint32_t CLOSED_LOOP2_MTR_STOP_SHIFT = 28;
  static constexpr uint32_t CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK = (0xFu << 24);
  static constexpr uint32_t CLOSED_LOOP2_MTR_STOP_BRK_TIME_SHIFT = 24;
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_MASK = (0xFFu << 8);
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_SHIFT = 8;
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_MASK = (0xFFu << 0);
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_SHIFT = 0;
  static constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK = (0xFFu << 23);
  static constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT = 23;
  static constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK = 0x7u;
  static constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT = 0;
  static constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK = (0x7Fu << 24);
  static constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT = 24;
  static constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_MASK = (0x3FFu << 14);
  static constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_SHIFT = 14;
  static constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_MASK = 0x3FFFu;
  static constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_SHIFT = 0;

  static constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_RES = 0x01u;
  static constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_IND = 0x01u;

  static constexpr uint32_t GD_CONFIG1_CSA_GAIN_MASK = 0x3u;
  static constexpr uint32_t GD_CONFIG1_CSA_GAIN_SHIFT = 0;
  static constexpr uint32_t GD_CONFIG2_BASE_CURRENT_MASK = 0x7FFFu;
  static constexpr uint32_t GD_CONFIG2_BASE_CURRENT_SHIFT = 0;
  static constexpr uint32_t INT_ALGO_2_CL_SLOW_ACC_MASK = (0xFu << 6);
  static constexpr uint32_t INT_ALGO_2_CL_SLOW_ACC_SHIFT = 6;

  static constexpr uint32_t FAULT_CONFIG1_ILIMIT_MASK = (0xFu << 27);
  static constexpr uint32_t FAULT_CONFIG1_ILIMIT_SHIFT = 27;
  static constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK = (0xFu << 23);
  static constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT = 23;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MASK = (0xFu << 19);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_SHIFT = 19;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK = (0xFu << 15);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT = 15;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK = (0xFu << 11);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT = 11;
  static constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_MASK = (0xFu << 7);
  static constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_SHIFT = 7;
  static constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_MASK = (0xFu << 3);
  static constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_SHIFT = 3;

  static constexpr uint32_t FAULT_CONFIG2_LOCK1_EN_MASK = (1u << 30);
  static constexpr uint32_t FAULT_CONFIG2_LOCK2_EN_MASK = (1u << 29);
  static constexpr uint32_t FAULT_CONFIG2_LOCK3_EN_MASK = (1u << 28);
  static constexpr uint32_t FAULT_CONFIG2_LOCK_ABN_SPEED_MASK = (0x7u << 25);
  static constexpr uint32_t FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT = 25;
  static constexpr uint32_t FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK = (0x7u << 22);
  static constexpr uint32_t FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT = 22;
  static constexpr uint32_t FAULT_CONFIG2_NO_MTR_THR_MASK = (0x7u << 19);
  static constexpr uint32_t FAULT_CONFIG2_NO_MTR_THR_SHIFT = 19;
  static constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK = (0xFu << 15);
  static constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT = 15;
  static constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK = (0x7u << 12);
  static constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT = 12;

  static constexpr uint32_t GATE_DRIVER_FAULT_ACTIVE_MASK = (1u << 31);
  static constexpr uint32_t GATE_FAULT_OTS = (1u << 29);
  static constexpr uint32_t GATE_FAULT_OCP_VDS = (1u << 28);
  static constexpr uint32_t GATE_FAULT_OCP_SNS = (1u << 27);
  static constexpr uint32_t GATE_FAULT_BST_UV = (1u << 26);
  static constexpr uint32_t GATE_FAULT_GVDD_UV = (1u << 25);
  static constexpr uint32_t GATE_FAULT_DRV_OFF = (1u << 24);

  static constexpr uint32_t CONTROLLER_FAULT_ACTIVE_MASK = (1u << 31);
  static constexpr uint32_t FAULT_IPD_FREQ = (1u << 29);
  static constexpr uint32_t FAULT_IPD_T1 = (1u << 28);
  static constexpr uint32_t FAULT_BUS_CURRENT_LIMIT = (1u << 26);
  static constexpr uint32_t FAULT_MPET_BEMF = (1u << 24);
  static constexpr uint32_t FAULT_ABN_SPEED = (1u << 23);
  static constexpr uint32_t FAULT_ABN_BEMF = (1u << 22);
  static constexpr uint32_t FAULT_NO_MTR = (1u << 21);
  static constexpr uint32_t FAULT_MTR_LCK = (1u << 20);
  static constexpr uint32_t FAULT_LOCK_LIMIT = (1u << 19);
  static constexpr uint32_t FAULT_HW_LOCK_LIMIT = (1u << 18);
  static constexpr uint32_t FAULT_DCBUS_UNDER_VOLTAGE = (1u << 17);
  static constexpr uint32_t FAULT_DCBUS_OVER_VOLTAGE = (1u << 16);
  static constexpr uint32_t FAULT_SPEED_LOOP_SATURATION = (1u << 15);
  static constexpr uint32_t FAULT_CURRENT_LOOP_SATURATION = (1u << 14);
  static constexpr uint32_t FAULT_WATCHDOG = (1u << 3);

  static constexpr float VM_VOLTAGE_SCALE = 60.0f / 134217728.0f;  // 60 / 2^27
  static constexpr float SPEED_Q27_SCALE = 1.0f / 134217728.0f;     // 1 / 2^27

  static constexpr uint8_t STARTUP_COMMS_ATTEMPTS = 20u;
  static constexpr uint32_t STARTUP_COMMS_RETRY_DELAY_MS = 250u;
  static constexpr uint32_t DEFERRED_COMMS_RETRY_INTERVAL_MS = 1000u;
  static constexpr uint32_t DEFERRED_SCAN_INTERVAL_MS = 5000u;
  static constexpr uint32_t STARTUP_PROFILE_CHECK_INTERVAL_MS = 1000u;
  static constexpr uint32_t STARTUP_PROFILE_RECOVERY_COOLDOWN_MS = 3000u;
  static constexpr uint8_t I2C_SCAN_ADDRESS_MIN = 0x00u;
  static constexpr uint8_t I2C_SCAN_ADDRESS_MAX = 0x7Eu;

  uint32_t inter_byte_delay_us_{100};
  bool auto_tickle_watchdog_{false};
  bool clear_mpet_on_startup_{true};
  bool cfg_motor_bemf_const_set_{false};
  bool cfg_brake_mode_set_{false};
  bool cfg_brake_time_set_{false};
  bool cfg_mode_set_{false};
  bool cfg_align_time_set_{false};
  bool cfg_csa_gain_set_{false};
  bool cfg_base_current_set_{false};
  bool cfg_direction_mode_set_{false};
  bool cfg_ilimit_set_{false};
  bool cfg_align_or_slow_current_ilimit_set_{false};
  bool cfg_lock_mode_set_{false};
  bool cfg_lock_ilimit_set_{false};
  bool cfg_hw_lock_ilimit_set_{false};
  bool cfg_lock_retry_time_set_{false};
  bool cfg_abn_speed_lock_enable_set_{false};
  bool cfg_abn_bemf_lock_enable_set_{false};
  bool cfg_no_motor_lock_enable_set_{false};
  bool cfg_lock_abn_speed_threshold_set_{false};
  bool cfg_abnormal_bemf_threshold_set_{false};
  bool cfg_no_motor_threshold_set_{false};
  bool cfg_max_speed_set_{false};
  bool cfg_open_loop_ilimit_set_{false};
  bool cfg_open_loop_limit_source_set_{false};
  bool cfg_open_loop_accel_set_{false};
  bool cfg_open_loop_accel2_set_{false};
  bool cfg_auto_handoff_enable_set_{false};
  bool cfg_open_to_closed_handoff_threshold_set_{false};
  bool cfg_theta_error_ramp_rate_set_{false};
  bool cfg_cl_slow_acc_set_{false};
  bool cfg_lock_ilimit_deglitch_set_{false};
  bool cfg_hw_lock_ilimit_deglitch_set_{false};
  bool cfg_speed_loop_kp_code_set_{false};
  bool cfg_speed_loop_ki_code_set_{false};
  uint8_t cfg_motor_bemf_const_{0};
  uint8_t cfg_brake_mode_{0};
  uint8_t cfg_brake_time_{0};
  uint8_t cfg_mode_{0};
  uint8_t cfg_align_time_{0};
  uint8_t cfg_csa_gain_{0};
  uint16_t cfg_base_current_code_{0};
  uint8_t cfg_ilimit_{0};
  uint8_t cfg_align_or_slow_current_ilimit_{0};
  uint8_t cfg_lock_mode_{0};
  uint8_t cfg_lock_ilimit_{0};
  uint8_t cfg_hw_lock_ilimit_{0};
  uint8_t cfg_lock_retry_time_{0};
  bool cfg_abn_speed_lock_enable_{false};
  bool cfg_abn_bemf_lock_enable_{false};
  bool cfg_no_motor_lock_enable_{false};
  uint8_t cfg_lock_abn_speed_threshold_{0};
  uint8_t cfg_abnormal_bemf_threshold_{0};
  uint8_t cfg_no_motor_threshold_{0};
  uint16_t cfg_max_speed_code_{0};
  uint8_t cfg_open_loop_ilimit_{0};
  bool cfg_open_loop_limit_use_ilimit_{false};
  uint8_t cfg_open_loop_accel_{0};
  uint8_t cfg_open_loop_accel2_{0};
  bool cfg_auto_handoff_enable_{false};
  uint8_t cfg_open_to_closed_handoff_threshold_{0};
  uint8_t cfg_theta_error_ramp_rate_{0};
  uint8_t cfg_cl_slow_acc_{0};
  uint8_t cfg_lock_ilimit_deglitch_{0};
  uint8_t cfg_hw_lock_ilimit_deglitch_{0};
  uint16_t cfg_speed_loop_kp_code_{0};
  uint16_t cfg_speed_loop_ki_code_{0};
  float speed_ramp_up_percent_per_s_{0.0f};
  float speed_ramp_down_percent_per_s_{0.0f};
  float start_boost_percent_{0.0f};
  uint32_t start_boost_hold_ms_{0u};
  float speed_target_percent_{0.0f};
  float speed_applied_percent_{0.0f};
  bool speed_target_active_{false};
  bool start_boost_active_{false};
  uint32_t start_boost_until_ms_{0u};
  uint32_t last_ramp_update_ms_{0u};
  std::string cfg_direction_mode_{"hardware"};
  uint32_t last_watchdog_tickle_ms_{0};
  uint32_t last_vm_diag_log_ms_{0};
  uint32_t last_speed_diag_log_ms_{0};
  bool fault_latched_{false};
  bool normal_operation_ready_{false};
  uint32_t deferred_comms_last_retry_ms_{0};
  uint32_t deferred_comms_last_scan_ms_{0};
  std::string last_fault_summary_{"none"};
  std::string motor_config_summary_{"default"};
  bool mpet_bemf_fault_latched_{false};
  bool hw_lock_fault_latched_{false};
  bool severe_fault_speed_lockout_{false};
  bool algorithm_state_valid_{false};
  bool algorithm_state_read_error_latched_{false};
  uint16_t last_algorithm_state_{0xFFFFu};
  uint32_t startup_profile_last_check_ms_{0u};
  uint32_t startup_profile_last_recovery_ms_{0u};
  std::unique_ptr<MCF8329ATuningController> tuning_controller_{nullptr};

  MCF8329ABrakeSwitch* brake_switch_{nullptr};
  MCF8329ADirectionSelect* direction_select_{nullptr};
  MCF8329ASpeedNumber* speed_number_{nullptr};
  binary_sensor::BinarySensor* fault_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* sys_enable_binary_sensor_{nullptr};
  sensor::Sensor* vm_voltage_sensor_{nullptr};
  sensor::Sensor* duty_cmd_percent_sensor_{nullptr};
  sensor::Sensor* volt_mag_percent_sensor_{nullptr};
  sensor::Sensor* motor_bemf_constant_sensor_{nullptr};
  sensor::Sensor* speed_fdbk_hz_sensor_{nullptr};
  sensor::Sensor* speed_ref_open_loop_hz_sensor_{nullptr};
  sensor::Sensor* fg_speed_fdbk_hz_sensor_{nullptr};
  text_sensor::TextSensor* current_fault_text_sensor_{nullptr};
};

}  // namespace mcf8329a
}  // namespace esphome
