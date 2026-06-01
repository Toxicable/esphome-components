#include "esc_higher.h"

#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
}

void ESCHigherComponent::update() {
  const i2c::ErrorCode err = this->write_read(nullptr, 0, nullptr, 0);
  if (err == i2c::ERROR_OK) {
    this->status_clear_warning();
    ESP_LOGV(TAG, "I2C ping ok (0x%02X)", this->address_);
    return;
  }

  this->status_set_warning();
  ESP_LOGW(TAG, "I2C ping failed (0x%02X), error=%d", this->address_, static_cast<int>(err));
}

void ESCHigherComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "esc_higher:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace esc_higher
}  // namespace esphome
