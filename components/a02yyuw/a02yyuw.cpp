#include "a02yyuw.h"

#include "esphome/components/uart/uart_component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace a02yyuw {

static const char *const TAG = "a02yyuw";

static constexpr uint8_t COMMAND_TRIGGER = 0x55;
static constexpr uint8_t RESPONSE_HEADER = 0xFF;
static constexpr uint8_t FRAME_LENGTH = 4;
static constexpr uint32_t INTERBYTE_TIMEOUT_MS = 10;

void A02YYUWComponent::update() {
  this->write_byte(COMMAND_TRIGGER);
}

void A02YYUWComponent::loop() {
  if (this->buffer_index_ > 0 && (millis() - this->last_byte_time_ > INTERBYTE_TIMEOUT_MS)) {
    this->buffer_index_ = 0;
  }

  while (this->available()) {
    uint8_t byte = 0;
    if (!this->read_byte(&byte)) {
      return;
    }

    this->last_byte_time_ = millis();

    if (this->buffer_index_ == 0 && byte != RESPONSE_HEADER) {
      continue;
    }

    this->buffer_[this->buffer_index_++] = byte;

    if (this->buffer_index_ < FRAME_LENGTH) {
      continue;
    }

    const uint8_t checksum = this->buffer_[0] + this->buffer_[1] + this->buffer_[2];
    if (checksum != this->buffer_[3]) {
      ESP_LOGW(TAG, "Checksum mismatch: 0x%02X != 0x%02X", checksum, this->buffer_[3]);
      this->status_set_warning();
      this->buffer_index_ = 0;
      continue;
    }

    const uint16_t distance = (static_cast<uint16_t>(this->buffer_[1]) << 8) | this->buffer_[2];
    if (this->distance_sensor_ != nullptr) {
      this->distance_sensor_->publish_state(distance);
    }

    this->status_clear_warning();
    this->buffer_index_ = 0;
  }
}

void A02YYUWComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "A02YYUW Ultrasonic Sensor:");
  // LOG_UART_DEVICE(this);
  LOG_SENSOR("  ", "Distance", this->distance_sensor_);
  LOG_UPDATE_INTERVAL(this);
}

} // namespace a02yyuw
} // namespace esphome
