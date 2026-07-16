#pragma once

#include <cstdint>

namespace mcf8316d_core {

namespace regs {

inline constexpr uint16_t REG_CONTROLLER_FAULT_STATUS = 0x00E2;
inline constexpr uint16_t REG_GATE_DRIVER_FAULT_STATUS = 0x00E0;
inline constexpr uint16_t REG_ALGO_STATUS = 0x00E4;
inline constexpr uint16_t REG_MTR_PARAMS = 0x00E6;
inline constexpr uint16_t REG_ALGO_STATUS_MPET = 0x00E8;
inline constexpr uint16_t REG_ALGO_CTRL1 = 0x00EA;
inline constexpr uint16_t REG_ALGO_DEBUG1 = 0x00EC;
inline constexpr uint16_t REG_ALGO_DEBUG2 = 0x00EE;
inline constexpr uint16_t REG_ALGORITHM_STATE = 0x018E;
inline constexpr uint16_t REG_FAULT_CONFIG1 = 0x0090;
inline constexpr uint16_t REG_FAULT_CONFIG2 = 0x0092;
inline constexpr uint16_t REG_MOTOR_STARTUP1 = 0x0084;
inline constexpr uint16_t REG_MOTOR_STARTUP2 = 0x0086;
inline constexpr uint16_t REG_CLOSED_LOOP1 = 0x0088;
inline constexpr uint16_t REG_CLOSED_LOOP2 = 0x008A;
inline constexpr uint16_t REG_CLOSED_LOOP3 = 0x008C;
inline constexpr uint16_t REG_CLOSED_LOOP4 = 0x008E;
inline constexpr uint16_t REG_GD_CONFIG1 = 0x00AC;
inline constexpr uint16_t REG_GD_CONFIG2 = 0x00AE;
inline constexpr uint16_t REG_ISD_CONFIG = 0x0080;
inline constexpr uint16_t REG_REV_DRIVE_CONFIG = 0x0082;
inline constexpr uint16_t REG_DEVICE_CONFIG1 = 0x00A6;
inline constexpr uint16_t REG_DEVICE_CONFIG2 = 0x00A8;
inline constexpr uint16_t REG_PIN_CONFIG = 0x00A4;
inline constexpr uint16_t REG_PERI_CONFIG1 = 0x00AA;
inline constexpr uint16_t REG_CSA_GAIN_FEEDBACK = 0x046C;
inline constexpr uint16_t REG_VOLTAGE_GAIN_FEEDBACK = 0x0477;
inline constexpr uint16_t REG_VM_VOLTAGE = 0x047C;

inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_MASK = (0x3u << 10);
inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_BRAKE = (0x1u << 10);
inline constexpr uint32_t PIN_CONFIG_BRAKE_INPUT_NO_BRAKE = (0x2u << 10);

inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_MASK = (0x3u << 0);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_HARDWARE = (0x0u << 0);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CW = (0x1u << 0);
inline constexpr uint32_t PERI_CONFIG1_DIR_INPUT_CCW = (0x2u << 0);

inline constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_MASK = (0x3u << 0);
inline constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_15V = (0x0u << 0);
inline constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_30V = (0x1u << 0);
inline constexpr uint32_t DEVICE_CONFIG1_BUS_VOLT_40V = (0x2u << 0);
inline constexpr uint32_t DEVICE_CONFIG2_DYNAMIC_CSA_GAIN_EN_MASK = (1u << 13);
inline constexpr uint32_t DEVICE_CONFIG2_DYNAMIC_VOLTAGE_GAIN_EN_MASK = (1u << 12);

inline constexpr uint32_t ALGO_DEBUG1_OVERRIDE_MASK = (1u << 31);
inline constexpr uint32_t ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK = (0x7FFFu << 16);
inline constexpr uint32_t ALGO_DEBUG1_CLOSED_LOOP_DIS_MASK = (1u << 15);
inline constexpr uint32_t ALGO_DEBUG1_FORCE_ALIGN_EN_MASK = (1u << 14);
inline constexpr uint32_t ALGO_DEBUG1_FORCE_SLOW_FIRST_CYCLE_EN_MASK = (1u << 13);
inline constexpr uint32_t ALGO_DEBUG1_FORCE_IPD_EN_MASK = (1u << 12);
inline constexpr uint32_t ALGO_DEBUG1_FORCE_ISD_EN_MASK = (1u << 11);
inline constexpr uint32_t ALGO_DEBUG1_FORCE_ALIGN_ANGLE_SRC_SEL_MASK = (1u << 10);

inline constexpr uint32_t ALGO_CTRL1_CLR_FLT_MASK = (1u << 0);
inline constexpr uint32_t ALGO_CTRL1_WATCHDOG_TICKLE_MASK = (1u << 1);

inline constexpr uint32_t ALGO_DEBUG2_MPET_CMD_MASK = (1u << 5);
inline constexpr uint32_t ALGO_DEBUG2_MPET_R_MASK = (1u << 4);
inline constexpr uint32_t ALGO_DEBUG2_MPET_L_MASK = (1u << 3);
inline constexpr uint32_t ALGO_DEBUG2_MPET_KE_MASK = (1u << 2);
inline constexpr uint32_t ALGO_DEBUG2_MPET_MECH_MASK = (1u << 1);
inline constexpr uint32_t ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK = (1u << 0);
inline constexpr uint32_t ALGO_DEBUG2_MPET_ALL_MASK =
  ALGO_DEBUG2_MPET_CMD_MASK | ALGO_DEBUG2_MPET_R_MASK | ALGO_DEBUG2_MPET_L_MASK |
  ALGO_DEBUG2_MPET_KE_MASK | ALGO_DEBUG2_MPET_MECH_MASK | ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK;

inline constexpr uint32_t ALGO_STATUS_DUTY_CMD_MASK = (0x0FFFu << 4);
inline constexpr uint32_t ALGO_STATUS_DUTY_CMD_SHIFT = 4;
inline constexpr uint32_t ALGO_STATUS_VOLT_MAG_MASK = (0x7FFFu << 16);
inline constexpr uint32_t ALGO_STATUS_VOLT_MAG_SHIFT = 16;
inline constexpr uint32_t ALGO_STATUS_SYS_ENABLE_FLAG_MASK = (1u << 2);
inline constexpr uint32_t ALGO_STATUS_MPET_R_DONE_MASK = (1u << 31);
inline constexpr uint32_t ALGO_STATUS_MPET_L_DONE_MASK = (1u << 30);
inline constexpr uint32_t ALGO_STATUS_MPET_KE_DONE_MASK = (1u << 29);
inline constexpr uint32_t ALGO_STATUS_MPET_MECH_DONE_MASK = (1u << 28);
inline constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_MASK = (0xFu << 24);
inline constexpr uint32_t ALGO_STATUS_MPET_PWM_FREQ_SHIFT = 24;

inline constexpr uint32_t FAULT_CONFIG1_ILIMIT_MASK = (0xFu << 27);
inline constexpr uint32_t FAULT_CONFIG1_ILIMIT_SHIFT = 27;
inline constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_MASK = (0xFu << 23);
inline constexpr uint32_t FAULT_CONFIG1_HW_LOCK_ILIMIT_SHIFT = 23;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MASK = (0xFu << 19);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_SHIFT = 19;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_MASK = (0x7u << 15);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_MODE_SHIFT = 15;
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_MASK = (0xFu << 11);
inline constexpr uint32_t FAULT_CONFIG1_LOCK_ILIMIT_DEG_SHIFT = 11;
inline constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_MASK = (0xFu << 7);
inline constexpr uint32_t FAULT_CONFIG1_LCK_RETRY_SHIFT = 7;
inline constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_MASK = (0x7u << 3);
inline constexpr uint32_t FAULT_CONFIG1_MTR_LCK_MODE_SHIFT = 3;

inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_MASK = (0x7u << 13);
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_DEG_SHIFT = 13;
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_MASK = (0x7u << 16);
inline constexpr uint32_t FAULT_CONFIG2_HW_LOCK_ILIMIT_MODE_SHIFT = 16;
inline constexpr uint32_t FAULT_CONFIG2_LOCK2_EN_MASK = (1u << 29);

inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_MASK = (0xFu << 17);
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_OR_SLOW_CURRENT_ILIMIT_SHIFT = 17;
inline constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_MASK = (0x3u << 29);
inline constexpr uint32_t MOTOR_STARTUP1_MTR_STARTUP_SHIFT = 29;
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_MASK = (0xFu << 21);
inline constexpr uint32_t MOTOR_STARTUP1_ALIGN_TIME_SHIFT = 21;

inline constexpr uint32_t ISD_CONFIG_ISD_EN_MASK = (1u << 30);
inline constexpr uint32_t ISD_CONFIG_BRAKE_EN_MASK = (1u << 29);
inline constexpr uint32_t ISD_CONFIG_HIZ_EN_MASK = (1u << 28);
inline constexpr uint32_t ISD_CONFIG_RESYNC_EN_MASK = (1u << 26);
inline constexpr uint32_t ISD_CONFIG_BRK_CONFIG_MASK = (1u << 20);
inline constexpr uint32_t ISD_CONFIG_BRK_TIME_MASK = (0xFu << 13);
inline constexpr uint32_t ISD_CONFIG_BRK_TIME_SHIFT = 13;

inline constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_MASK = (0xFu << 27);
inline constexpr uint32_t MOTOR_STARTUP2_OL_ILIMIT_SHIFT = 27;
inline constexpr uint32_t MOTOR_STARTUP2_AUTO_HANDOFF_EN_MASK = (1u << 18);
inline constexpr uint32_t MOTOR_STARTUP2_AUTO_HANDOFF_EN_SHIFT = 18;
inline constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_MASK = (0x1Fu << 13);
inline constexpr uint32_t MOTOR_STARTUP2_OPN_CL_HANDOFF_THR_SHIFT = 13;
inline constexpr uint32_t MOTOR_STARTUP2_ALIGN_ANGLE_MASK = (0x1Fu << 8);
inline constexpr uint32_t MOTOR_STARTUP2_ALIGN_ANGLE_SHIFT = 8;
inline constexpr uint32_t MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_MASK = (0xFu << 4);
inline constexpr uint32_t MOTOR_STARTUP2_SLOW_FIRST_CYC_FREQ_SHIFT = 4;
inline constexpr uint32_t MOTOR_STARTUP2_FIRST_CYCLE_FREQ_SEL_MASK = (1u << 3);

inline constexpr uint32_t CLOSED_LOOP1_PWM_FREQ_OUT_MASK = (0xFu << 15);
inline constexpr uint32_t CLOSED_LOOP1_PWM_FREQ_OUT_SHIFT = 15;
inline constexpr uint32_t GD_CONFIG1_CSA_GAIN_MASK = 0x3u;
inline constexpr uint32_t GD_CONFIG1_CSA_GAIN_SHIFT = 0;

inline constexpr uint32_t MTR_PARAMS_R_MASK = (0xFFu << 24);
inline constexpr uint32_t MTR_PARAMS_R_SHIFT = 24;
inline constexpr uint32_t MTR_PARAMS_KE_MASK = (0xFFu << 16);
inline constexpr uint32_t MTR_PARAMS_KE_SHIFT = 16;
inline constexpr uint32_t MTR_PARAMS_L_MASK = (0xFFu << 8);
inline constexpr uint32_t MTR_PARAMS_L_SHIFT = 8;

inline constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_MASK = (0xFFu << 8);
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_RES_SHIFT = 8;
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_MASK = 0xFFu;
inline constexpr uint32_t CLOSED_LOOP2_MOTOR_IND_SHIFT = 0;
inline constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_MASK = (0xFFu << 23);
inline constexpr uint32_t CLOSED_LOOP3_MOTOR_BEMF_CONST_SHIFT = 23;
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KP_MASK = (0x3FFu << 13);
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KP_SHIFT = 13;
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KI_MASK = (0x3FFu << 3);
inline constexpr uint32_t CLOSED_LOOP3_CURR_LOOP_KI_SHIFT = 3;
inline constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_MASK = 0x7u;
inline constexpr uint32_t CLOSED_LOOP3_SPD_LOOP_KP_MSB_SHIFT = 0;
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_MASK = (0x7Fu << 24);
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KP_LSB_SHIFT = 24;
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_MASK = (0x3FFu << 14);
inline constexpr uint32_t CLOSED_LOOP4_SPD_LOOP_KI_SHIFT = 14;
inline constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_MASK = 0x3FFFu;
inline constexpr uint32_t CLOSED_LOOP4_MAX_SPEED_SHIFT = 0;
inline constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_RES = 0x01u;
inline constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_IND = 0x01u;
inline constexpr uint32_t CLOSED_LOOP_SEED_MOTOR_BEMF = 0x01u;
inline constexpr uint32_t CLOSED_LOOP_SEED_SPD_KP = 0x01u;
inline constexpr uint32_t CLOSED_LOOP_SEED_SPD_KI = 0x01u;

inline constexpr uint32_t GD_CONFIG2_BUCK_PS_DIS_MASK = (1u << 24);
inline constexpr uint32_t GD_CONFIG2_BUCK_CL_MASK = (1u << 23);
inline constexpr uint32_t GD_CONFIG2_BUCK_SEL_MASK = (0x3u << 21);
inline constexpr uint32_t GD_CONFIG2_BUCK_SEL_SHIFT = 21;
inline constexpr uint32_t GD_CONFIG2_BUCK_DIS_MASK = (1u << 20);

inline constexpr uint32_t GATE_DRIVER_FAULT_ACTIVE_MASK = (1u << 31);
inline constexpr uint32_t GATE_FAULT_OCP = (1u << 28);
inline constexpr uint32_t GATE_FAULT_OVP = (1u << 26);
inline constexpr uint32_t GATE_FAULT_OTW = (1u << 23);
inline constexpr uint32_t GATE_FAULT_OTS = (1u << 22);
inline constexpr uint32_t GATE_FAULT_OCP_HC = (1u << 21);
inline constexpr uint32_t GATE_FAULT_OCP_LC = (1u << 20);
inline constexpr uint32_t GATE_FAULT_OCP_HB = (1u << 19);
inline constexpr uint32_t GATE_FAULT_OCP_LB = (1u << 18);
inline constexpr uint32_t GATE_FAULT_OCP_HA = (1u << 17);
inline constexpr uint32_t GATE_FAULT_OCP_LA = (1u << 16);
inline constexpr uint32_t GATE_FAULT_BUCK_OCP = (1u << 13);
inline constexpr uint32_t GATE_FAULT_BUCK_UV = (1u << 12);
inline constexpr uint32_t GATE_FAULT_VCP_UV = (1u << 11);
inline constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_40V = 0x0000;
inline constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_30V = 0x0001;
inline constexpr uint16_t VOLTAGE_GAIN_FEEDBACK_15V = 0x0002;

inline constexpr uint32_t VM_VOLTAGE_ADC_MASK = (0xFFu << 16);
inline constexpr uint32_t VM_VOLTAGE_ADC_SHIFT = 16;
inline constexpr uint32_t VM_VOLTAGE_Q11_MASK = (0x7FFu << 16);
inline constexpr uint32_t VM_VOLTAGE_Q11_SHIFT = 16;

inline constexpr uint32_t CONTROLLER_FAULT_ACTIVE_MASK = (1u << 31);
inline constexpr uint32_t FAULT_IPD_FREQ = (1u << 29);
inline constexpr uint32_t FAULT_IPD_T1 = (1u << 28);
inline constexpr uint32_t FAULT_IPD_T2 = (1u << 27);
inline constexpr uint32_t FAULT_MPET_IPD = (1u << 25);
inline constexpr uint32_t FAULT_MPET_BEMF = (1u << 24);
inline constexpr uint32_t FAULT_WATCHDOG = (1u << 3);
inline constexpr uint32_t FAULT_NO_MTR = (1u << 21);
inline constexpr uint32_t FAULT_MTR_LCK = (1u << 20);
inline constexpr uint32_t FAULT_LOCK_LIMIT = (1u << 19);
inline constexpr uint32_t FAULT_HW_LOCK_LIMIT = (1u << 18);
inline constexpr uint32_t FAULT_ABN_SPEED = (1u << 23);
inline constexpr uint32_t FAULT_ABN_BEMF = (1u << 22);
inline constexpr uint32_t FAULT_MTR_UNDER_VOLTAGE = (1u << 17);
inline constexpr uint32_t FAULT_MTR_OVER_VOLTAGE = (1u << 16);
inline constexpr uint32_t FAULT_SPEED_LOOP_SATURATION = (1u << 15);
inline constexpr uint32_t FAULT_CURRENT_LOOP_SATURATION = (1u << 14);
inline constexpr uint32_t FAULT_MAX_SPEED_SATURATION = (1u << 13);
inline constexpr uint32_t FAULT_BUS_POWER_LIMIT_SATURATION = (1u << 12);
inline constexpr uint32_t FAULT_EEPROM_WRITE_LOCK_SET = (1u << 11);
inline constexpr uint32_t FAULT_EEPROM_READ_LOCK_SET = (1u << 10);
inline constexpr uint32_t FAULT_I2C_CRC = (1u << 6);
inline constexpr uint32_t FAULT_EEPROM_ERR = (1u << 5);
inline constexpr uint32_t FAULT_BOOT_STL = (1u << 4);
inline constexpr uint32_t FAULT_CPU_RESET = (1u << 2);
inline constexpr uint32_t FAULT_WWDT = (1u << 1);

}  // namespace regs

enum class DirectionInputMode : uint8_t {
  HARDWARE = 0,
  CW = 1,
  CCW = 2,
};

const char *algorithm_state_to_string(uint16_t state);
const char *brake_input_to_string(uint32_t brake_input_value);
const char *direction_input_to_string(uint32_t direction_input_value);

}  // namespace mcf8316d_core
