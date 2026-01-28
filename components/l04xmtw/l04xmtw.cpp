#include "l04xmtw.h"

#include "esphome/components/uart/uart_component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace l04xmtw {

static const char *const TAG = "l04xmtw";

static constexpr uint8_t COMMAND_TRIGGER = 0x55;
static constexpr uint8_t RESPONSE_HEADER = 0xFF;
static constexpr uint8_t FRAME_LENGTH = 4;
static constexpr uint16_t MIN_DISTANCE_MM = 50;
static constexpr uint16_t MAX_DISTANCE_MM = 6000;

void L04XMTWComponent::setup() {
  this->check_uart_settings(115200);
}

void L04XMTWComponent::update() {
  uint8_t dump = 0;
  while (this->available() > 0) {
    this->read_byte(&dump);
  }
  this->buffer_index_ = 0;
  this->waiting_ = true;
  this->warned_this_cycle_ = false;
  this->trigger_ms_ = millis();
  this->rx_deadline_ms_ = this->trigger_ms_ + RX_WINDOW_MS;
  ESP_LOGD(TAG, "Sending trigger command 0x%02X", COMMAND_TRIGGER);
  this->write_byte(COMMAND_TRIGGER);
  this->flush();
}

void L04XMTWComponent::loop() {
  if (!this->waiting_) {
    uint8_t dump = 0;
    while (this->available() > 0) {
      this->read_byte(&dump);
    }
    return;
  }

  if (millis() > this->rx_deadline_ms_) {
    ESP_LOGW(TAG, "RX window timeout; resetting frame state");
    this->waiting_ = false;
    this->buffer_index_ = 0;
    uint8_t dump = 0;
    while (this->available() > 0) {
      this->read_byte(&dump);
    }
    this->status_set_warning();
    return;
  }

  if (this->buffer_index_ > 0 && (millis() - this->last_byte_time_ > INTERBYTE_TIMEOUT_MS)) {
    ESP_LOGD(TAG, "Inter-byte timeout after %u ms, resetting buffer", INTERBYTE_TIMEOUT_MS);
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
    ESP_LOGD(TAG, "Received frame: 0x%02X 0x%02X 0x%02X 0x%02X (checksum 0x%02X)",
             this->buffer_[0], this->buffer_[1], this->buffer_[2], this->buffer_[3], checksum);
    if (checksum != this->buffer_[3]) {
      if (!this->warned_this_cycle_) {
        ESP_LOGW(TAG, "Checksum mismatch: 0x%02X != 0x%02X", checksum, this->buffer_[3]);
        this->status_set_warning();
        this->warned_this_cycle_ = true;
      }
      this->buffer_index_ = 0;
      continue;
    }

    const uint16_t distance = (static_cast<uint16_t>(this->buffer_[1]) << 8) | this->buffer_[2];
    if (distance == 0xFFFF || distance < MIN_DISTANCE_MM || distance > MAX_DISTANCE_MM) {
      ESP_LOGW(TAG, "Invalid distance %u mm", distance);
      this->status_set_warning();
      this->waiting_ = false;
      this->buffer_index_ = 0;
      return;
    }
    ESP_LOGD(TAG, "Received distance %u mm", distance);
    if (this->distance_sensor_ != nullptr) {
      this->distance_sensor_->publish_state(distance);
    } else {
      ESP_LOGD(TAG, "No distance sensor configured; skipping publish");
    }

    this->status_clear_warning();
    this->waiting_ = false;
    this->buffer_index_ = 0;
    return;
  }
}

void L04XMTWComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "L04xMTW Ultrasonic Sensor:");
  // LOG_UART_DEVICE(this);
  LOG_SENSOR("  ", "Distance", this->distance_sensor_);
  LOG_UPDATE_INTERVAL(this);
}

} // namespace l04xmtw
} // namespace esphome
