#pragma once

#include <cstdint>

#include "mcf8329a_registers.h"

namespace mcf8329a_core {

namespace regs {

inline constexpr uint32_t PIN_CONFIG_VDC_FILTER_DISABLE_MASK = (1u << 27);
inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_MASK = (0x3u << 2);
inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_BRAKE = (0x1u << 2);
inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_NO_BRAKE = (0x2u << 2);

inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_MASK = (0x3u << 19);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_HARDWARE = (0x0u << 19);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CW = (0x1u << 19);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CCW = (0x2u << 19);

inline constexpr uint32_t ALGO_DEBUG1_OVERRIDE_MASK = (1u << 31);
inline constexpr uint32_t ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK = (0x7FFFu << 16);

inline constexpr uint32_t ALGO_CTRL1_CLR_FLT_MASK = (1u << 29);
inline constexpr uint32_t ALGO_CTRL1_WATCHDOG_TICKLE_MASK = (1u << 10);
inline constexpr uint32_t ALGO_DEBUG2_MPET_CMD_MASK = (1u << 5);
inline constexpr uint32_t ALGO_DEBUG2_MPET_KE_MASK = (1u << 2);
inline constexpr uint32_t ALGO_DEBUG2_MPET_MECH_MASK = (1u << 1);
inline constexpr uint32_t ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK = (1u << 0);
inline constexpr uint32_t ALGO_DEBUG2_MPET_RUN_MASK =
  ALGO_DEBUG2_MPET_CMD_MASK | ALGO_DEBUG2_MPET_KE_MASK | ALGO_DEBUG2_MPET_MECH_MASK;
inline constexpr uint32_t ALGO_DEBUG2_MPET_ALL_MASK =
  ALGO_DEBUG2_MPET_RUN_MASK | ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK;

inline constexpr uint32_t ALGO_STATUS_DUTY_CMD_MASK = (0x0FFFu << 4);
inline constexpr uint32_t ALGO_STATUS_DUTY_CMD_SHIFT = 4;
inline constexpr uint32_t ALGO_STATUS_VOLT_MAG_MASK = (0xFFFFu << 16);
inline constexpr uint32_t ALGO_STATUS_VOLT_MAG_SHIFT = 16;
inline constexpr uint32_t ALGO_STATUS_SYS_ENABLE_FLAG_MASK = (1u << 2);
inline constexpr uint32_t ALGO_STATUS_MPET_KE_STATUS_MASK = (1u << 29);
inline constexpr uint32_t ALGO_STATUS_MPET_MECH_STATUS_MASK = (1u << 28);
inline constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_MASK = (0xFu << 24);
inline constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_SHIFT = 24;
inline constexpr uint32_t MTR_PARAMS_MOTOR_BEMF_CONST_MASK = (0xFFu << 16);
inline constexpr uint32_t MTR_PARAMS_MOTOR_BEMF_CONST_SHIFT = 16;

inline constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_MASK = (0x3u << 29);
inline constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_SHIFT = 29;
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_MASK = (0xFu << 21);
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_SHIFT = 21;
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK = (0xFu << 17);
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT = 17;
inline constexpr uint32_t MOTOR_STARTUP1_OL_ILIMIT_CONFIG_MASK = (1u << 3);
inline constexpr uint32_t MOTOR_STARTUP1_OL_ILIMIT_CONFIG_SHIFT = 3;
inline constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_MASK = (0xFu << 27);
inline constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_SHIFT = 27;
inline constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A1_MASK = (0xFu << 23);
inline constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A1_SHIFT = 23;
inline constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A2_MASK = (0xFu << 19);
inline constexpr uint32_t MOTOR_STARTUP2_OL_ACC_A2_SHIFT = 19;
inline constexpr uint32_t MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK = (1u << 18);
inline constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK = (0x1Fu << 13);
inline constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT = 13;
inline constexpr uint32_t MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_MASK = (0x7u << 0);
inline constexpr uint32_t MOTOR_STARTUP2_THETA_ERROR_RAMP_RATE_SHIFT = 0;

inline constexpr uint32_t CLOSED_LOOP2_MTR_STOP_MASK = (0x7u << 28);
inline constexpr uint32_t CLOSED_LOOP2_MTR_STOP_SHIFT = 28;
inline constexpr uint32_t CLOSED_LOOP2_MTR_STOP_BRK_TIME_MASK = (0xFu << 24);
inline constexpr uint32_t CLOSED_LOOP2_MTR_STOP_BRK_TIME_SHIFT = 24;
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_MASK = (0xFFu << 8);
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_SHIFT = 8;
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_MASK = (0xFFu << 0);
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_SHIFT = 0;
inline constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK = (0xFFu << 23);
inline constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT = 23;
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KP_MASK = (0x3FFu << 13);
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KI_MASK = (0x3FFu << 3);
inline constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK = 0x7u;
inline constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT = 0;
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK = (0x7Fu << 24);
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT = 24;
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_MASK = (0x3FFu << 14);
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_SHIFT = 14;
inline constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_MASK = 0x3FFFu;
inline constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_SHIFT = 0;

inline constexpr uint32_t GD_CONFIG1_CSA_GAIN_MASK = 0x3u;
inline constexpr uint32_t GD_CONFIG1_CSA_GAIN_SHIFT = 0;
inline constexpr uint32_t GD_CONFIG2_BASE_CURRENT_MASK = 0x7FFFu;
inline constexpr uint32_t GD_CONFIG2_BASE_CURRENT_SHIFT = 0;
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_CURR_REF_MASK = (0x7u << 8);
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_CURR_REF_SHIFT = 8;
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_SPEED_REF_MASK = (0x3u << 6);
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_SPEED_REF_SHIFT = 6;
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_SLEW_RATE_MASK = (0x7u << 3);
inline constexpr uint32_t INT_ALGO_1_MPET_OPEN_LOOP_SLEW_RATE_SHIFT = 3;
inline constexpr uint32_t INT_ALGO_2_MPET_KE_MEAS_PARAMETER_SELECT_MASK = (1u << 1);
inline constexpr uint32_t INT_ALGO_2_CL_SLOW_ACC_MASK = (0xFu << 6);
inline constexpr uint32_t INT_ALGO_2_CL_SLOW_ACC_SHIFT = 6;

inline constexpr uint32_t FAULT_CONFIG1_ILIMIT_MASK = (0xFu << 27);
inline constexpr uint32_t FAULT_CONFIG1_ILIMIT_SHIFT = 27;
inline constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK = (0xFu << 23);
inline constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT = 23;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MASK = (0xFu << 19);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_SHIFT = 19;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK = (0xFu << 15);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT = 15;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK = (0xFu << 11);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT = 11;
inline constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_MASK = (0xFu << 7);
inline constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_SHIFT = 7;
inline constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_MASK = (0xFu << 3);
inline constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_SHIFT = 3;

inline constexpr uint32_t FAULT_CONFIG2_LOCK1_EN_MASK = (1u << 30);
inline constexpr uint32_t FAULT_CONFIG2_LOCK2_EN_MASK = (1u << 29);
inline constexpr uint32_t FAULT_CONFIG2_LOCK3_EN_MASK = (1u << 28);
inline constexpr uint32_t FAULT_CONFIG2_LOCK_ABN_SPEED_MASK = (0x7u << 25);
inline constexpr uint32_t FAULT_CONFIG2_LOCK_ABN_SPEED_SHIFT = 25;
inline constexpr uint32_t FAULT_CONFIG2_ABNORMAL_BEMF_THR_MASK = (0x7u << 22);
inline constexpr uint32_t FAULT_CONFIG2_ABNORMAL_BEMF_THR_SHIFT = 22;
inline constexpr uint32_t FAULT_CONFIG2_NO_MTR_THR_MASK = (0x7u << 19);
inline constexpr uint32_t FAULT_CONFIG2_NO_MTR_THR_SHIFT = 19;
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK = (0xFu << 15);
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT = 15;
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK = (0x7u << 12);
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT = 12;

inline constexpr uint32_t GATE_DRIVER_FAULT_ACTIVE_MASK = (1u << 31);
inline constexpr uint32_t GATE_FAULT_OTS = (1u << 29);
inline constexpr uint32_t GATE_FAULT_OCP_VDS = (1u << 28);
inline constexpr uint32_t GATE_FAULT_OCP_SNS = (1u << 27);
inline constexpr uint32_t GATE_FAULT_BST_UV = (1u << 26);
inline constexpr uint32_t GATE_FAULT_GVDD_UV = (1u << 25);
inline constexpr uint32_t GATE_FAULT_DRV_OFF = (1u << 24);

inline constexpr uint32_t CONTROLLER_FAULT_ACTIVE_MASK = (1u << 31);
inline constexpr uint32_t FAULT_IPD_FREQ = (1u << 29);
inline constexpr uint32_t FAULT_IPD_T1 = (1u << 28);
inline constexpr uint32_t FAULT_BUS_CURRENT_LIMIT = (1u << 26);
inline constexpr uint32_t FAULT_MPET_BEMF = (1u << 24);
inline constexpr uint32_t FAULT_ABN_SPEED = (1u << 23);
inline constexpr uint32_t FAULT_ABN_BEMF = (1u << 22);
inline constexpr uint32_t FAULT_NO_MTR = (1u << 21);
inline constexpr uint32_t FAULT_MTR_LCK = (1u << 20);
inline constexpr uint32_t FAULT_LOCK_LIMIT = (1u << 19);
inline constexpr uint32_t FAULT_HW_LOCK_LIMIT = (1u << 18);
inline constexpr uint32_t FAULT_DCBUS_UNDER_VOLTAGE = (1u << 17);
inline constexpr uint32_t FAULT_DCBUS_OVER_VOLTAGE = (1u << 16);
inline constexpr uint32_t FAULT_SPEED_LOOP_SATURATION = (1u << 15);
inline constexpr uint32_t FAULT_CURRENT_LOOP_SATURATION = (1u << 14);
inline constexpr uint32_t FAULT_WATCHDOG = (1u << 3);

}  // namespace regs

enum class DirectionInputMode : uint8_t {
  HARDWARE = 0,
  CW = 1,
  CCW = 2,
};

float decode_vm_voltage(uint32_t raw);
float decode_max_speed_hz(uint16_t code);
float decode_speed_hz(int32_t raw, float max_speed_hz);
float decode_fg_speed_hz(uint32_t raw, float max_speed_hz);
float decode_open_loop_accel_hz_per_s(uint8_t code);
float decode_open_to_closed_handoff_percent(uint8_t code);
const char *mode_to_string(uint8_t mode);
const char *align_time_to_string(uint8_t code);
const char *brake_mode_to_string(uint8_t code);
const char *brake_time_to_string(uint8_t code);
const char *algorithm_state_to_string(uint16_t state);
const char *lock_mode_to_string(uint8_t mode);
const char *lock_retry_time_to_string(uint8_t code);
const char *brake_input_to_string(uint32_t brake_input_value);
const char *direction_input_to_string(uint32_t direction_input_value);

}  // namespace mcf8329a_core
