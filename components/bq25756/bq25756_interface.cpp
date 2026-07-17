#include "bq25756.h"

#include "esphome/core/hal.h"

namespace esphome {
namespace bq25756 {

::component_common::ChargerCapabilities BQ25756Component::capabilities() const {
  return {
      .enable_control = true,
      .battery_current = true,
      .battery_voltage = true,
      .charge_state = true,
      .power_good = true,
      .fault_status = true,
  };
}

::component_common::ChargerSnapshot BQ25756Component::snapshot() const {
  auto *self = const_cast<BQ25756Component *>(this);
  ::bq25756_core::Status status{};
  ::bq25756_core::Measurements measurements{};
  ::bq25756_core::ControlStates controls{};
  ::bq25756_core::AdcConfigurationState adc_state{};

  const auto measurement_result = self->service_.read_measurements(
      measurements, false, ::bq25756_core::REG2B_ADC_CONTINUOUS_15_BIT,
      adc_state);
  if (!self->service_.read_status(status) ||
      measurement_result != ::bq25756_core::MeasurementReadResult::OK ||
      !self->service_.read_control_states(controls)) {
    auto unavailable = self->charger_snapshot_;
    unavailable.valid = false;
    return unavailable;
  }

  self->charger_snapshot_ = ::bq25756_core::make_charger_snapshot(
      status, measurements, controls, self->charger_snapshot_.sequence + 1U, millis());
  return self->charger_snapshot_;
}

bool BQ25756Component::request_enabled(bool enabled) {
  if (!this->set_charge_enabled(enabled)) {
    return false;
  }
  this->charger_snapshot_.enabled = enabled;
  return true;
}

}  // namespace bq25756
}  // namespace esphome
