#pragma once

#include <cstdint>
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
namespace mcf8316d_manual {

class MCF8316DManualComponent;

class MCF8316DBrakeSwitch : public switch_::Switch {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void write_state(bool state) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DDirectionSelect : public select::Select {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void control(const std::string &value) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DSpeedNumber : public number::Number {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void control(float value) override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DClearFaultsButton : public button::Button {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DWatchdogTickleButton : public button::Button {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DApplyStartupTuneButton : public button::Button {
 public:
  void set_parent(MCF8316DManualComponent *parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MCF8316DManualComponent *parent_{nullptr};
};

class MCF8316DManualComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_reg32(uint16_t offset, uint32_t &value);
  bool read_reg16(uint16_t offset, uint16_t &value);
  bool write_reg32(uint16_t offset, uint32_t value);
  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value);

  bool set_brake_override(bool brake_on);
  bool set_direction_mode(const std::string &direction_mode);
  bool set_speed_percent(float speed_percent);
  void pulse_clear_faults();
  void pulse_watchdog_tickle();
  bool apply_startup_tune_profile();

  void set_inter_byte_delay_us(uint32_t inter_byte_delay_us) { inter_byte_delay_us_ = inter_byte_delay_us; }
  void set_auto_tickle_watchdog(bool auto_tickle_watchdog) { auto_tickle_watchdog_ = auto_tickle_watchdog; }

  void set_brake_switch(MCF8316DBrakeSwitch *sw) { brake_switch_ = sw; }
  void set_direction_select(MCF8316DDirectionSelect *sel) { direction_select_ = sel; }
  void set_speed_number(MCF8316DSpeedNumber *num) { speed_number_ = num; }
  void set_fault_active_binary_sensor(binary_sensor::BinarySensor *s) { fault_active_binary_sensor_ = s; }
  void set_sys_enable_binary_sensor(binary_sensor::BinarySensor *s) { sys_enable_binary_sensor_ = s; }
  void set_vm_voltage_sensor(sensor::Sensor *s) { vm_voltage_sensor_ = s; }
  void set_duty_cmd_percent_sensor(sensor::Sensor *s) { duty_cmd_percent_sensor_ = s; }
  void set_volt_mag_percent_sensor(sensor::Sensor *s) { volt_mag_percent_sensor_ = s; }
  void set_fault_summary_text_sensor(text_sensor::TextSensor *s) { fault_summary_text_sensor_ = s; }

 protected:
  bool read_probe_and_publish_();
  bool perform_read16_(uint16_t offset, uint16_t &value);
  bool perform_read_(uint16_t offset, uint32_t &value);
  bool perform_write_(uint16_t offset, uint32_t value);
  uint32_t build_control_word_(bool is_read, uint16_t offset, bool is_32bit) const;
  void delay_between_bytes_() const;
  void publish_faults_(uint32_t gate_fault_status, bool gate_fault_valid, uint32_t controller_fault_status,
                       bool controller_fault_valid);
  void publish_algo_status_(uint32_t algo_status);
  bool ensure_buck_current_limit_for_manual_();
  bool seed_closed_loop_params_if_zero_();
  void log_buck_fault_diagnostics_(const char *context, uint32_t gate_fault_status);
  void log_mpet_diagnostics_(const char *context);
  void log_mpet_entry_conditions_(const char *context, uint32_t algo_debug2);
  void log_lock_limit_diagnostics_(const char *context, uint32_t controller_fault_status);
  bool should_force_speed_shutdown_(uint32_t gate_fault_status, bool gate_fault_valid, uint32_t controller_fault_status,
                                    bool controller_fault_valid) const;
  const char *algorithm_state_to_string_(uint16_t state) const;
  void handle_fault_shutdown_(bool fault_active);

  static constexpr uint16_t REG_CONTROLLER_FAULT_STATUS = 0x00E2;
  static constexpr uint16_t REG_GATE_DRIVER_FAULT_STATUS = 0x00E0;
  static constexpr uint16_t REG_ALGO_STATUS = 0x00E4;
  static constexpr uint16_t REG_MTR_PARAMS = 0x00E6;
  static constexpr uint16_t REG_ALGO_STATUS_MPET = 0x00E8;
  static constexpr uint16_t REG_ALGO_CTRL1 = 0x00EA;
  static constexpr uint16_t REG_ALGO_DEBUG1 = 0x00EC;
  static constexpr uint16_t REG_ALGO_DEBUG2 = 0x00EE;
  static constexpr uint16_t REG_ALGORITHM_STATE = 0x018E;
  static constexpr uint16_t REG_FAULT_CONFIG1 = 0x0090;
  static constexpr uint16_t REG_MOTOR_STARTUP1 = 0x0084;
  static constexpr uint16_t REG_MOTOR_STARTUP2 = 0x0086;
  static constexpr uint16_t REG_CLOSED_LOOP2 = 0x008A;
  static constexpr uint16_t REG_CLOSED_LOOP3 = 0x008C;
  static constexpr uint16_t REG_CLOSED_LOOP4 = 0x008E;
  static constexpr uint16_t REG_GD_CONFIG2 = 0x00AE;
  static constexpr uint16_t REG_ISD_CONFIG = 0x0080;
  static constexpr uint16_t REG_REV_DRIVE_CONFIG = 0x0082;
  static constexpr uint16_t REG_DEVICE_CONFIG1 = 0x00A6;
  static constexpr uint16_t REG_DEVICE_CONFIG2 = 0x00A8;
  static constexpr uint16_t REG_PIN_CONFIG = 0x00A4;
  static constexpr uint16_t REG_PERI_CONFIG1 = 0x00AA;
  static constexpr uint16_t REG_VOLTAGE_GAIN_FEEDBACK = 0x0477;
  static constexpr uint16_t REG_VM_VOLTAGE = 0x047C;

  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_MASK = (0x3u << 10);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_BRAKE = (0x1u << 10);
  static constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_NO_BRAKE = (0x2u << 10);

  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_MASK = (0x3u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_HARDWARE = (0x0u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CW = (0x1u << 0);
  static constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CCW = (0x2u << 0);

  static constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_MASK = (0x3u << 0);
  static constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_15V = (0x0u << 0);
  static constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_30V = (0x1u << 0);
  static constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_40V = (0x2u << 0);
  static constexpr uint32_t DEVICE_CONFIG2_DYNAMIC_VOLTAGE_GAIN_EN_MASK = (1u << 12);

  static constexpr uint32_t ALGO_DEBUG1_OVERRIDE_MASK = (1u << 31);
  static constexpr uint32_t ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK = (0x7FFFu << 16);

  static constexpr uint32_t ALGO_CTRL1_CLR_FLT_MASK = (1u << 0);
  static constexpr uint32_t ALGO_CTRL1_WATCHDOG_TICKLE_MASK = (1u << 1);

  static constexpr uint32_t ALGO_DEBUG2_MPET_CMD_MASK = (1u << 5);
  static constexpr uint32_t ALGO_DEBUG2_MPET_R_MASK = (1u << 4);
  static constexpr uint32_t ALGO_DEBUG2_MPET_L_MASK = (1u << 3);
  static constexpr uint32_t ALGO_DEBUG2_MPET_KE_MASK = (1u << 2);
  static constexpr uint32_t ALGO_DEBUG2_MPET_MECH_MASK = (1u << 1);
  static constexpr uint32_t ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK = (1u << 0);
  static constexpr uint32_t ALGO_DEBUG2_MPET_ALL_MASK =
      ALGO_DEBUG2_MPET_CMD_MASK | ALGO_DEBUG2_MPET_R_MASK | ALGO_DEBUG2_MPET_L_MASK | ALGO_DEBUG2_MPET_KE_MASK |
      ALGO_DEBUG2_MPET_MECH_MASK | ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK;

  static constexpr uint32_t ALGO_STATUS_DUTY_CMD_MASK = 0x0FFFu;
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_MASK = (0x7FFFu << 16);
  static constexpr uint32_t ALGO_STATUS_VOLT_MAG_SHIFT = 16;
  static constexpr uint32_t ALGO_STATUS_SYS_ENABLE_FLAG_MASK = (1u << 15);
  static constexpr uint32_t ALGO_STATUS_MPET_R_DONE_MASK = (1u << 31);
  static constexpr uint32_t ALGO_STATUS_MPET_L_DONE_MASK = (1u << 30);
  static constexpr uint32_t ALGO_STATUS_MPET_KE_DONE_MASK = (1u << 29);
  static constexpr uint32_t ALGO_STATUS_MPET_MECH_DONE_MASK = (1u << 28);
  static constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_MASK = (0xFu << 24);
  static constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_SHIFT = 24;

  static constexpr uint32_t FAULT_CONFIG1_ILIMIT_MASK = (0xFu << 27);
  static constexpr uint32_t FAULT_CONFIG1_ILIMIT_SHIFT = 27;
  static constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK = (0xFu << 23);
  static constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT = 23;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MASK = (0xFu << 19);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_SHIFT = 19;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK = (0x7u << 15);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT = 15;
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK = (0xFu << 11);
  static constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT = 11;
  static constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_MASK = (0xFu << 7);
  static constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_SHIFT = 7;

  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK = (0xFu << 17);
  static constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT = 17;

  static constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_MASK = (0xFu << 27);
  static constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_SHIFT = 27;
  static constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK = (0x1Fu << 13);
  static constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT = 13;
  static constexpr uint32_t MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK = (0xFu << 4);
  static constexpr uint32_t MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT = 4;
  static constexpr uint32_t MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK = (1u << 3);

  static constexpr uint32_t STARTUP_TUNE_LOCK_ILIMIT_DEG = 6u;
  static constexpr uint32_t STARTUP_TUNE_LCK_RETRY = 2u;
  static constexpr uint32_t STARTUP_TUNE_ALIGN_OR_SLOW_CURRENT_ILIMIT = 6u;
  static constexpr uint32_t STARTUP_TUNE_OL_ILIMIT = 6u;
  static constexpr uint32_t STARTUP_TUNE_OPN_CL_HANDOFF_THR = 0x0Fu;
  static constexpr uint32_t STARTUP_TUNE_SLOW_FIRST_CYC_FREQ = 4u;
  static constexpr uint32_t STARTUP_TUNE_FIRST_CYCLE_FREQ_SEL = 1u;

  static constexpr uint32_t MTR_PARAMS_R_MASK = (0xFFu << 24);
  static constexpr uint32_t MTR_PARAMS_R_SHIFT = 24;
  static constexpr uint32_t MTR_PARAMS_KE_MASK = (0xFFu << 16);
  static constexpr uint32_t MTR_PARAMS_KE_SHIFT = 16;
  static constexpr uint32_t MTR_PARAMS_L_MASK = (0xFFu << 8);
  static constexpr uint32_t MTR_PARAMS_L_SHIFT = 8;

  static constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_MASK = (0xFFu << 8);
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_SHIFT = 8;
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_MASK = 0xFFu;
  static constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_SHIFT = 0;

  static constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK = (0xFFu << 23);
  static constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT = 23;
  static constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KP_MASK = (0x3FFu << 13);
  static constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KP_SHIFT = 13;
  static constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KI_MASK = (0x3FFu << 3);
  static constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KI_SHIFT = 3;
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
  static constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_BEMF = 0x01u;
  static constexpr uint32_t CLOSED_LOOP_SEED_SPD_KP = 0x01u;
  static constexpr uint32_t CLOSED_LOOP_SEED_SPD_KI = 0x01u;

  static constexpr uint32_t GD_CONFIG2_BUCK_PS_DIS_MASK = (1u << 24);
  static constexpr uint32_t GD_CONFIG2_BUCK_CL_MASK = (1u << 23);
  static constexpr uint32_t GD_CONFIG2_BUCK_SEL_MASK = (0x3u << 21);
  static constexpr uint32_t GD_CONFIG2_BUCK_SEL_SHIFT = 21;
  static constexpr uint32_t GD_CONFIG2_BUCK_DIS_MASK = (1u << 20);


  static constexpr uint32_t GATE_DRIVER_FAULT_ACTIVE_MASK = (1u << 31);
  static constexpr uint32_t GATE_FAULT_OCP = (1u << 28);
  static constexpr uint32_t GATE_FAULT_OVP = (1u << 26);
  static constexpr uint32_t GATE_FAULT_OTW = (1u << 23);
  static constexpr uint32_t GATE_FAULT_OTS = (1u << 22);
  static constexpr uint32_t GATE_FAULT_OCP_HC = (1u << 21);
  static constexpr uint32_t GATE_FAULT_OCP_LC = (1u << 20);
  static constexpr uint32_t GATE_FAULT_OCP_HB = (1u << 19);
  static constexpr uint32_t GATE_FAULT_OCP_LB = (1u << 18);
  static constexpr uint32_t GATE_FAULT_OCP_HA = (1u << 17);
  static constexpr uint32_t GATE_FAULT_OCP_LA = (1u << 16);
  static constexpr uint32_t GATE_FAULT_BUCK_OCP = (1u << 13);
  static constexpr uint32_t GATE_FAULT_BUCK_UV = (1u << 12);
  static constexpr uint32_t GATE_FAULT_VCP_UV = (1u << 11);
  static constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_40V = 0x0000;
  static constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_30V = 0x0001;
  static constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_15V = 0x0002;

  static constexpr uint32_t VM_VOLTAGE_ADC_MASK = (0xFFu << 16);
  static constexpr uint32_t VM_VOLTAGE_ADC_SHIFT = 16;
  static constexpr uint32_t VM_VOLTAGE_Q11_MASK = (0x7FFu << 16);
  static constexpr uint32_t VM_VOLTAGE_Q11_SHIFT = 16;

  static constexpr uint32_t CONTROLLER_FAULT_ACTIVE_MASK = (1u << 31);
  static constexpr uint32_t FAULT_IPD_FREQ = (1u << 29);
  static constexpr uint32_t FAULT_IPD_T1 = (1u << 28);
  static constexpr uint32_t FAULT_IPD_T2 = (1u << 27);
  static constexpr uint32_t FAULT_MPET_IPD = (1u << 25);
  static constexpr uint32_t FAULT_MPET_BEMF = (1u << 24);
  static constexpr uint32_t FAULT_WATCHDOG = (1u << 3);
  static constexpr uint32_t FAULT_NO_MTR = (1u << 21);
  static constexpr uint32_t FAULT_MTR_LCK = (1u << 20);
  static constexpr uint32_t FAULT_LOCK_LIMIT = (1u << 19);
  static constexpr uint32_t FAULT_HW_LOCK_LIMIT = (1u << 18);
  static constexpr uint32_t FAULT_ABN_SPEED = (1u << 23);
  static constexpr uint32_t FAULT_ABN_BEMF = (1u << 22);
  static constexpr uint32_t FAULT_MTR_UNDER_VOLTAGE = (1u << 17);
  static constexpr uint32_t FAULT_MTR_OVER_VOLTAGE = (1u << 16);
  static constexpr uint32_t FAULT_SPEED_LOOP_SATURATION = (1u << 15);
  static constexpr uint32_t FAULT_CURRENT_LOOP_SATURATION = (1u << 14);
  static constexpr uint32_t FAULT_MAX_SPEED_SATURATION = (1u << 13);
  static constexpr uint32_t FAULT_BUS_POWER_LIMIT_SATURATION = (1u << 12);
  static constexpr uint32_t FAULT_EEPROM_WRITE_LOCK_SET = (1u << 11);
  static constexpr uint32_t FAULT_EEPROM_READ_LOCK_SET = (1u << 10);
  static constexpr uint32_t FAULT_I2C_CRC = (1u << 6);
  static constexpr uint32_t FAULT_EEPROM_ERR = (1u << 5);
  static constexpr uint32_t FAULT_BOOT_STL = (1u << 4);
  static constexpr uint32_t FAULT_CPU_RESET = (1u << 2);
  static constexpr uint32_t FAULT_WWDT = (1u << 1);

  uint32_t inter_byte_delay_us_{100};
  bool auto_tickle_watchdog_{false};
  uint32_t last_watchdog_tickle_ms_{0};
  uint32_t last_lock_limit_diag_log_ms_{0};
  uint32_t last_buck_diag_log_ms_{0};
  uint32_t last_mpet_diag_log_ms_{0};
  uint32_t last_vm_diag_log_ms_{0};
  bool lock_limit_prev_active_{false};
  bool fault_latched_{false};
  std::string last_fault_summary_{"none"};

  MCF8316DBrakeSwitch *brake_switch_{nullptr};
  MCF8316DDirectionSelect *direction_select_{nullptr};
  MCF8316DSpeedNumber *speed_number_{nullptr};
  binary_sensor::BinarySensor *fault_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *sys_enable_binary_sensor_{nullptr};
  sensor::Sensor *vm_voltage_sensor_{nullptr};
  sensor::Sensor *duty_cmd_percent_sensor_{nullptr};
  sensor::Sensor *volt_mag_percent_sensor_{nullptr};
  text_sensor::TextSensor *fault_summary_text_sensor_{nullptr};
};

}  // namespace mcf8316d_manual
}  // namespace esphome
