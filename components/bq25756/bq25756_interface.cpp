#include "bq25756.h"

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
  return this->charger_snapshot_;
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
