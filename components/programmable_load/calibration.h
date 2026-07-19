#pragma once

#include <cstdint>

namespace programmable_load_core {

static constexpr uint16_t CALIBRATION_VERSION = 1;

enum class CalibrationSource : uint8_t {
  CONFIGURED = 0,
  RESTORED,
  APPLIED,
};

struct LinearCalibration {
  float scale{1.0f};
  float offset{0.0f};

  float apply(float raw_value) const {
    return raw_value * this->scale + this->offset;
  }
};

struct OutputCalibration {
  // Normalized DAC level that corresponds to zero requested current.
  float zero_level{0.0f};

  // Calibrated current represented by DAC level 1.0 before safety clamping.
  // Keep the old member spelling as a storage alias for the in-tree action
  // template until all generated call sites have moved to the unit-suffixed name.
  union {
    float full_scale_current_a{0.0f};
    float full_scale_current;
  };
};

struct Calibration {
  uint16_t version{CALIBRATION_VERSION};
  LinearCalibration current{};
  LinearCalibration voltage{};
  OutputCalibration output{};
};

}  // namespace programmable_load_core
