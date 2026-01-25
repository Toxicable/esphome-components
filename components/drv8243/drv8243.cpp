#include "drv8243.h"

#include <cmath>

#include "esphome/components/ledc/ledc_output.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace drv8243 {

static const char *const TAG = "drv8243";

// Handshake timings
static constexpr uint32_t SLEEP_FORCE_MS = 2;
static constexpr uint32_t READY_WAIT_TIMEOUT_US = 5000;
static constexpr uint32_t ACK_WAIT_TIMEOUT_US = 5000;
static constexpr uint32_t POLL_STEP_US = 10;
// ACK pulse: keep close to lower edge to reduce risk of stretching above ~40us
static constexpr uint32_t ACK_PULSE_US = 22;

const char *DRV8243Output::handshake_result_str_(DRV8243Output::HandshakeResult r) const {
  switch (r) {
  case DRV8243Output::HandshakeResult::NOT_RUN:
    return "not_run";
  case DRV8243Output::HandshakeResult::SUCCESS:
    return "success";
  case DRV8243Output::HandshakeResult::FAILED:
    return "failed";
  default:
    return "unknown";
  }
}

void DRV8243Output::dump_config() {
  ESP_LOGCONFIG(TAG, "DRV8243 Output");
  const bool two_channel = out2_output_ != nullptr;
  ESP_LOGCONFIG(TAG, "  Mode: %s", two_channel ? "2-channel" : "1-channel");
  ESP_LOGCONFIG(TAG, "  Channel 1: %s", out1_output_ ? "configured" : "NOT SET");
  if (out1_component_ != nullptr && out1_component_ != this) {
    ESP_LOGCONFIG(TAG, "  Channel 1 config:");
    out1_component_->dump_config();
  }
  if (out1_output_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Channel 1 output: %s", ch1_output_ != nullptr ? "separate" : "primary");
  }

  ESP_LOGCONFIG(TAG, "  Channel 2: %s", two_channel ? "configured" : "not configured");
  if (two_channel && out2_component_ != nullptr && out2_component_ != this) {
    ESP_LOGCONFIG(TAG, "  Channel 2 config:");
    out2_component_->dump_config();
  }
  if (two_channel) {
    ESP_LOGCONFIG(TAG, "  Channel 2 output: %s", ch2_output_ != nullptr ? "separate" : "mirrored");
  }
  char pin_summary[64];
  if (nsleep_pin_) {
    nsleep_pin_->dump_summary(pin_summary, sizeof(pin_summary));
    ESP_LOGCONFIG(TAG, "  nSLEEP pin: %s", pin_summary);
  } else {
    ESP_LOGCONFIG(TAG, "  nSLEEP pin: NOT SET");
  }

  if (nfault_pin_) {
    nfault_pin_->dump_summary(pin_summary, sizeof(pin_summary));
    ESP_LOGCONFIG(TAG, "  nFAULT pin: %s", pin_summary);
  } else {
    ESP_LOGCONFIG(TAG, "  nFAULT pin: NOT SET");
  }

  if (out2_pin_ != nullptr) {
    out2_pin_->dump_summary(pin_summary, sizeof(pin_summary));
    ESP_LOGCONFIG(TAG, "  Polarity pin: %s (direction=%s)", pin_summary, flip_polarity_ ? "HIGH" : "LOW");
  } else if (two_channel) {
    ESP_LOGCONFIG(TAG, "  Polarity pin: not used (ch2 configured)");
  } else {
    ESP_LOGCONFIG(TAG, "  Polarity pin: NOT SET");
  }

  ESP_LOGCONFIG(TAG, "  Handshake: %s", handshake_result_str_(handshake_result_));
}

void DRV8243Output::setup() {
  if (!out1_output_ || !nsleep_pin_ || (!out2_pin_ && !out2_output_)) {
    mark_failed();
    return;
  }

  if (nsleep_pin_) {
    nsleep_pin_->setup();
    nsleep_pin_->pin_mode(gpio::FLAG_OUTPUT);
    nsleep_pin_->digital_write(true);
  }

  if (nfault_pin_) {
    nfault_pin_->setup();
    nfault_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  }

  // out2 pin is only used in 1 ch mode
  if (out2_pin_) {
    out2_pin_->setup();
    out2_pin_->pin_mode(gpio::FLAG_OUTPUT);
    out2_pin_->digital_write(flip_polarity_);
  }

  handshake_result_ = do_handshake_();
  if (handshake_result_ != DRV8243Output::HandshakeResult::SUCCESS) {
    mark_failed();
  }
}

DRV8243Output::HandshakeResult DRV8243Output::do_handshake_() {
  if (!nsleep_pin_)
    return DRV8243Output::HandshakeResult::FAILED;

  // Force sleep then wake
  nsleep_pin_->digital_write(false);
  delay(SLEEP_FORCE_MS);
  nsleep_pin_->digital_write(true);

  // Wait for nFAULT LOW if available (device-ready indication)
  bool saw_ready_low = false;
  int checks = 0;
  if (nfault_pin_) {
    uint32_t start = micros();
    while ((micros() - start) < READY_WAIT_TIMEOUT_US) {
      ++checks;
      if (!nfault_pin_->digital_read()) { // LOW
        saw_ready_low = true;
        break;
      }
      delayMicroseconds(POLL_STEP_US);
    }
  }

  // ACK pulse
  nsleep_pin_->digital_write(false);
  delayMicroseconds(ACK_PULSE_US);
  nsleep_pin_->digital_write(true);

  if (!nfault_pin_)
    return DRV8243Output::HandshakeResult::FAILED;

  if (!saw_ready_low)
    return DRV8243Output::HandshakeResult::FAILED;

  // Confirm nFAULT HIGH after ACK
  uint32_t start = micros();
  while ((micros() - start) < ACK_WAIT_TIMEOUT_US) {
    if (nfault_pin_->digital_read()) // HIGH
      return DRV8243Output::HandshakeResult::SUCCESS;
    delayMicroseconds(POLL_STEP_US);
  }

  return DRV8243Output::HandshakeResult::FAILED;
}

void DRV8243Output::write_state(float state) {
  this->write_to_output_(out1_output_, state);
  if (out2_output_ != nullptr && ch2_output_ == nullptr) {
    this->write_to_output_(out2_output_, state);
  }
}

void DRV8243Output::write_channel(uint8_t channel, float state) {
  if (channel == 2) {
    this->write_to_output_(out2_output_, state);
    return;
  }

  this->write_to_output_(out1_output_, state);
}

void DRV8243Output::write_to_output_(output::FloatOutput *out, float state) {
  if (this->is_failed() || out == nullptr)
    return;

  // Only drive OUT2 polarity if configured as a GPIO pin.
  if (out2_pin_) {
    out2_pin_->digital_write(flip_polarity_);
  }

  if (state <= 0.0005f) {
    out->set_level(0.0f);
    return;
  }

  float x = state;
  if (x < 0.0f) {
    x = 0.0f;
  }
  if (x > 1.0f)
    x = 1.0f;

  float y;
  if (exponent_ <= 0.0f) {
    y = min_level_ + (1.0f - min_level_) * x;
  } else {
    y = min_level_ + (1.0f - min_level_) * powf(x, exponent_);
  }

  if (y < 0.0f)
    y = 0.0f;
  if (y > 1.0f)
    y = 1.0f;

  out->set_level(y);
}

void DRV8243ChannelOutput::write_state(float state) {
  if (parent_ == nullptr)
    return;
  parent_->write_channel(channel_, state);
}

} // namespace drv8243
} // namespace esphome
