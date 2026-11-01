#include "toxic_solar.h"

#include "esphome/core/log.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

namespace esphome::toxic_solar {

static const char *const TAG = "toxic_solar";
static constexpr gpio_pin_t BATTERY_SENSE_ENABLE_PIN = 14;
static constexpr gpio_pin_t BATTERY_CHARGE_CURRENT_PIN = 13;
static constexpr gpio_pin_t BOOT_LED_PIN = 26;
static constexpr uint32_t BOOT_LED_STEP_MS = 150;

static const struct device *gpio0() { return DEVICE_DT_GET(DT_NODELABEL(gpio0)); }

void ToxicSolar::setup() {
  const struct device *port = gpio0();
  if (!device_is_ready(port)) {
    ESP_LOGE(TAG, "nRF GPIO port 0 is not ready");
    this->mark_failed();
    return;
  }

  int err = gpio_pin_configure(port, BATTERY_SENSE_ENABLE_PIN, GPIO_OUTPUT_LOW);
  if (err != 0) {
    ESP_LOGE(TAG, "Failed to enable the battery voltage divider: %d", err);
    this->mark_failed();
    return;
  }

  const gpio_flags_t charge_mode = this->battery_charge_current_ma_ == 100 ? GPIO_OUTPUT_LOW : GPIO_INPUT;
  err = gpio_pin_configure(port, BATTERY_CHARGE_CURRENT_PIN, charge_mode);
  if (err != 0) {
    ESP_LOGE(TAG, "Failed to set battery charge current selection: %d", err);
    this->mark_failed();
    return;
  }
  ESP_LOGI(TAG, "Battery charge current selection: %u mA", this->battery_charge_current_ma_);

  err = gpio_pin_configure(port, BOOT_LED_PIN, GPIO_OUTPUT_HIGH);
  if (err != 0) {
    ESP_LOGE(TAG, "Failed to configure boot indicator: %d", err);
    this->mark_failed();
    return;
  }

  if (this->boot_indicator_) {
    this->boot_blinks_remaining_ = 2;
    this->boot_led_on_();
  }
}

void ToxicSolar::boot_led_on_() {
  gpio_pin_set_raw(gpio0(), BOOT_LED_PIN, 0);
  this->set_timeout("boot_led", BOOT_LED_STEP_MS, [this]() { this->boot_led_off_(); });
}

void ToxicSolar::boot_led_off_() {
  gpio_pin_set_raw(gpio0(), BOOT_LED_PIN, 1);
  if (--this->boot_blinks_remaining_ > 0) {
    this->set_timeout("boot_led", BOOT_LED_STEP_MS, [this]() { this->boot_led_on_(); });
  }
}

}  // namespace esphome::toxic_solar
