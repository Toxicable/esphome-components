#pragma once

#include <cstdint>

namespace esphome {
namespace mcf8329a {
namespace tables {

// Key: FAULT_CONFIG1.ILIMIT field code [0..15]
// Value: current limit as percent of BASE_CURRENT.
static constexpr uint8_t LOCK_ILIMIT_PERCENT[16] = {
  5,
  10,
  15,
  20,
  25,
  30,
  40,
  50,
  60,
  65,
  70,
  75,
  80,
  85,
  90,
  95,
};

// Key: FAULT_CONFIG2.LOCK_ABN_SPEED field code [0..7]
// Value: human-readable threshold label.
static constexpr const char* const LOCK_ABN_SPEED_THRESHOLD_LABELS[8] = {
  "130%",
  "140%",
  "150%",
  "160%",
  "170%",
  "180%",
  "190%",
  "200%",
};

// Key: FAULT_CONFIG2.ABNORMAL_BEMF_THR field code [0..7]
// Value: human-readable threshold label.
static constexpr const char* const ABNORMAL_BEMF_THRESHOLD_LABELS[8] = {
  "40%",
  "45%",
  "50%",
  "55%",
  "60%",
  "65%",
  "67.5%",
  "70%",
};

// Key: FAULT_CONFIG2.NO_MTR_THR field code [0..7]
// Value: human-readable threshold label.
static constexpr const char* const NO_MOTOR_THRESHOLD_LABELS[8] = {
  "1%",
  "2%",
  "3%",
  "4%",
  "5%",
  "7.5%",
  "10%",
  "20%",
};

// Key: MOTOR_STARTUP2.OL_ACC_A1 field code [0..15]
// Value: acceleration in Hz/s.
static constexpr float OPEN_LOOP_ACCEL_HZ_PER_S[16] = {
  0.01f,
  0.05f,
  1.0f,
  2.5f,
  5.0f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  250.0f,
  500.0f,
  750.0f,
  1000.0f,
  5000.0f,
  10000.0f,
};

// Key: MOTOR_STARTUP2.OL_ACC_A2 field code [0..15]
// Value: acceleration in Hz/s^2.
static constexpr float OPEN_LOOP_ACCEL2_HZ_PER_S2[16] = {
  0.0f,
  0.05f,
  1.0f,
  2.5f,
  5.0f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  250.0f,
  500.0f,
  750.0f,
  1000.0f,
  5000.0f,
  10000.0f,
};

// Key: MOTOR_STARTUP2.OPN_CL_HANDOFF_THR field code [0..31]
// Value: threshold as percent of MAX_SPEED.
static constexpr float OPEN_TO_CLOSED_HANDOFF_PERCENT[32] = {
  1.0f,
  2.0f,
  3.0f,
  4.0f,
  5.0f,
  6.0f,
  7.0f,
  8.0f,
  9.0f,
  10.0f,
  11.0f,
  12.0f,
  13.0f,
  14.0f,
  15.0f,
  16.0f,
  17.0f,
  18.0f,
  19.0f,
  20.0f,
  22.5f,
  25.0f,
  27.5f,
  30.0f,
  32.5f,
  35.0f,
  37.5f,
  40.0f,
  42.5f,
  45.0f,
  47.5f,
  50.0f,
};

// Key: MOTOR_STARTUP2.THETA_ERROR_RAMP_RATE field code [0..7]
// Value: ramp rate in unitless datasheet scale.
static constexpr float THETA_ERROR_RAMP_RATE[8] = {
  0.01f,
  0.05f,
  0.1f,
  0.15f,
  0.2f,
  0.5f,
  1.0f,
  2.0f,
};

// Key: INT_ALGO_2.CL_SLOW_ACC field code [0..15]
// Value: acceleration in Hz/s.
static constexpr float CL_SLOW_ACC_HZ_PER_S[16] = {
  0.1f,
  1.0f,
  2.0f,
  3.0f,
  5.0f,
  10.0f,
  20.0f,
  30.0f,
  40.0f,
  50.0f,
  100.0f,
  200.0f,
  500.0f,
  750.0f,
  1000.0f,
  2000.0f,
};

// Key: FAULT_CONFIG1.LOCK_ILIMIT_DEG field code [0..15]
// Value: deglitch time in ms.
static constexpr float LOCK_ILIMIT_DEGLITCH_MS[16] = {
  0.0f,
  0.1f,
  0.2f,
  0.5f,
  1.0f,
  2.5f,
  5.0f,
  7.5f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  200.0f,
  500.0f,
  1000.0f,
};

// Key: FAULT_CONFIG2.HW_LOCK_ILIMIT_DEG field code [0..7]
// Value: deglitch time in us.
static constexpr uint8_t HW_LOCK_ILIMIT_DEGLITCH_US[8] = {
  0, 1, 2, 3, 4, 5, 6, 7,
};

// Key: INT_ALGO_1.MPET_OPEN_LOOP_CURR_REF field code [0..7]
// Value: current reference as percent.
static constexpr uint8_t MPET_OPEN_LOOP_CURR_REF_PERCENT[8] = {
  10, 20, 30, 40, 50, 60, 70, 80,
};

// Key: INT_ALGO_1.MPET_OPEN_LOOP_SPEED_REF field code [0..3]
// Value: speed reference as percent.
static constexpr uint8_t MPET_OPEN_LOOP_SPEED_REF_PERCENT[4] = {
  15, 25, 35, 50,
};

// Key: INT_ALGO_1.MPET_OPEN_LOOP_SLEW_RATE field code [0..7]
// Value: slew rate in Hz/s.
static constexpr float MPET_OPEN_LOOP_SLEW_HZ_PER_S[8] = {
  0.1f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f, 20.0f,
};

}  // namespace tables
}  // namespace mcf8329a
}  // namespace esphome
