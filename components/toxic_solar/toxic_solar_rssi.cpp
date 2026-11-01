#include "toxic_solar_rssi.h"

#include <openthread/thread.h>

namespace esphome::toxic_solar {

void ParentLastRssiSensor::update() {
  auto lock = openthread::InstanceLock::try_acquire(10);
  if (!lock) {
    return;
  }

  int8_t rssi;
  if (otThreadGetParentLastRssi(lock.get_instance(), &rssi) == OT_ERROR_NONE) {
    this->publish_state(rssi);
  }
}

}  // namespace esphome::toxic_solar
