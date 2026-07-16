#pragma once

#include "esphome/core/automation.h"

namespace esphome {
namespace programmable_load {

template<typename... Ts> class ApplyCalibrationAction : public Action<Ts...> {
 public:
  explicit ApplyCalibrationAction(ProgrammableLoadComponent *parent)
      : parent_(parent) {}

  TEMPLATABLE_VALUE(float, current_scale)
  TEMPLATABLE_VALUE(float, current_offset)
  TEMPLATABLE_VALUE(float, voltage_scale)
  TEMPLATABLE_VALUE(float, voltage_offset)
  TEMPLATABLE_VALUE(float, output_zero_level)
  TEMPLATABLE_VALUE(float, output_full_scale_current)
  TEMPLATABLE_VALUE(bool, persist)

  void play(Ts... x) override {
    if (this->parent_ == nullptr) return;
    Calibration calibration{};
    calibration.version = ::programmable_load_core::CALIBRATION_VERSION;
    calibration.current.scale = this->current_scale_.value(x...);
    calibration.current.offset = this->current_offset_.value(x...);
    calibration.voltage.scale = this->voltage_scale_.value(x...);
    calibration.voltage.offset = this->voltage_offset_.value(x...);
    calibration.output.zero_level = this->output_zero_level_.value(x...);
    calibration.output.full_scale_current_a =
        this->output_full_scale_current_.value(x...);
    this->parent_->apply_calibration(calibration, this->persist_.value(x...));
  }

 protected:
  ProgrammableLoadComponent *parent_{nullptr};
};

template<typename... Ts> class ResetCalibrationAction : public Action<Ts...> {
 public:
  explicit ResetCalibrationAction(ProgrammableLoadComponent *parent)
      : parent_(parent) {}

  TEMPLATABLE_VALUE(bool, persist)

  void play(Ts... x) override {
    if (this->parent_ != nullptr)
      this->parent_->reset_calibration(this->persist_.value(x...));
  }

 protected:
  ProgrammableLoadComponent *parent_{nullptr};
};

}  // namespace programmable_load
}  // namespace esphome
