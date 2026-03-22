#pragma once

#include <cstdint>

namespace esphome {
namespace mcf8329a {
namespace tables {

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

static constexpr uint8_t HW_LOCK_ILIMIT_DEGLITCH_US[8] = {
  0, 1, 2, 3, 4, 5, 6, 7,
};

static constexpr uint8_t MPET_OPEN_LOOP_CURR_REF_PERCENT[8] = {
  10, 20, 30, 40, 50, 60, 70, 80,
};

static constexpr uint8_t MPET_OPEN_LOOP_SPEED_REF_PERCENT[4] = {
  15, 25, 35, 50,
};

static constexpr float MPET_OPEN_LOOP_SLEW_HZ_PER_S[8] = {
  0.1f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f, 20.0f,
};

}  // namespace tables
}  // namespace mcf8329a
}  // namespace esphome
