#include "i2c_power.h"

#ifdef USE_I2C
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <cerrno>

#include "esphome/core/log.h"

namespace esphome::xiao_openthread_sed {

static bool set_i2c_state(pm_device_action action) {
  const auto* bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
  if (!device_is_ready(bus)) {
    ESP_LOGE("xiao_openthread_sed", "I2C controller is unavailable");
    return false;
  }
  // Zephyr's nrfx TWIM suspend disables the peripheral and applies its
  // low-power pinctrl state: SDA/SCL inputs, no pulls. Resume restores it.
  const int result = pm_device_action_run(bus, action);
  if (result != 0 && result != -EALREADY) {
    ESP_LOGE("xiao_openthread_sed", "I2C power transition failed: %d", result);
    return false;
  }
  return true;
}

bool suspend_i2c() {
  return set_i2c_state(PM_DEVICE_ACTION_SUSPEND);
}

bool resume_i2c() {
  return set_i2c_state(PM_DEVICE_ACTION_RESUME);
}

}  // namespace esphome::xiao_openthread_sed
#endif
