#include "toxic_solar_battery.h"

#include <cmath>

namespace esphome::toxic_solar {

// Samsung INR18650-30Q OCV/SOC model points at 25 C. This is a voltage
// estimate, not coulomb counting; load, temperature, age, and cell variation
// affect the actual state of charge.
static constexpr float VOLTAGE_POINTS[] = {2.8277f, 3.2019f, 3.3513f, 3.5200f, 3.6322f, 3.7336f,
                                           3.8204f, 3.9168f, 4.0157f, 4.0765f, 4.1682f};
static constexpr float PERCENT_POINTS[] = {0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f,
                                           60.0f, 70.0f, 80.0f, 90.0f, 100.0f};
static constexpr size_t NUM_VOLTAGE_POINTS = sizeof(VOLTAGE_POINTS) / sizeof(VOLTAGE_POINTS[0]);

void BatteryPercentSensor::setup() {
  if (this->voltage_sensor_ == nullptr) {
    this->mark_failed();
    return;
  }
  this->voltage_sensor_->add_on_state_callback([this](float voltage) { this->publish_voltage_(voltage); });
  if (this->voltage_sensor_->has_state()) {
    this->publish_voltage_(this->voltage_sensor_->get_state());
  }
}

void BatteryPercentSensor::publish_voltage_(float voltage) {
  if (!std::isfinite(voltage)) {
    this->publish_state(NAN);
    return;
  }
  if (voltage <= VOLTAGE_POINTS[0]) {
    this->publish_state(0.0f);
    return;
  }
  for (size_t i = 0; i + 1 < NUM_VOLTAGE_POINTS; i++) {
    if (voltage <= VOLTAGE_POINTS[i + 1]) {
      const float fraction = (voltage - VOLTAGE_POINTS[i]) / (VOLTAGE_POINTS[i + 1] - VOLTAGE_POINTS[i]);
      this->publish_state(PERCENT_POINTS[i] + fraction * (PERCENT_POINTS[i + 1] - PERCENT_POINTS[i]));
      return;
    }
  }
  this->publish_state(100.0f);
}

}  // namespace esphome::toxic_solar
