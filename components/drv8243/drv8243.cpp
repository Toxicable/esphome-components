#include "drv8243.h"

#include "esphome/components/ledc/ledc_output.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace drv8243 {

static const char* const TAG = "drv8243";

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
    ESP_LOGCONFIG(
      TAG, "  Polarity pin: %s (direction=%s)", pin_summary, flip_polarity_ ? "HIGH" : "LOW"
    );
  } else if (two_channel) {
    ESP_LOGCONFIG(TAG, "  Polarity pin: not used (ch2 configured)");
  } else {
    ESP_LOGCONFIG(TAG, "  Polarity pin: NOT SET");
  }

  ESP_LOGCONFIG(TAG, "  Handshake: %s", ::drv8243_core::handshake_result_to_string(handshake_result_));
}

void DRV8243Output::setup() {
  if (!out1_output_ || !nsleep_pin_ || !nfault_pin_ || (!out2_pin_ && !out2_output_)) {
    mark_failed();
    return;
  }

  if (nsleep_pin_) {
    nsleep_pin_->setup();
    nsleep_pin_->pin_mode(gpio::FLAG_OUTPUT);
    nsleep_pin_->digital_write(true);
  }

  nfault_pin_->setup();
  nfault_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);

  // out2 pin is only used in 1 ch mode
  if (out2_pin_) {
    out2_pin_->setup();
    out2_pin_->pin_mode(gpio::FLAG_OUTPUT);
    this->service_.set_static_polarity(flip_polarity_);
  }

  handshake_result_ = this->service_.handshake();
  if (handshake_result_ != ::drv8243_core::HandshakeResult::SUCCESS) {
    mark_failed();
  }
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

void DRV8243Output::write_to_output_(output::FloatOutput* out, float state) {
  if (this->is_failed() || out == nullptr)
    return;

  // Only drive OUT2 polarity if configured as a GPIO pin.
  if (out2_pin_) {
    this->service_.set_static_polarity(flip_polarity_);
  }

  out->set_level(::drv8243_core::shaped_output_level(state, min_level_, exponent_));
}

void DRV8243Output::write_nsleep(bool level) {
  if (this->nsleep_pin_ != nullptr)
    this->nsleep_pin_->digital_write(level);
}

bool DRV8243Output::read_nfault(bool *level) {
  if (this->nfault_pin_ == nullptr || level == nullptr)
    return false;
  *level = this->nfault_pin_->digital_read();
  return true;
}

void DRV8243Output::write_out2(bool level) {
  if (this->out2_pin_ != nullptr)
    this->out2_pin_->digital_write(level);
}

void DRV8243Output::delay_ms(uint32_t ms) {
  delay(ms);
}

void DRV8243Output::delay_us(uint32_t us) {
  delayMicroseconds(us);
}

void DRV8243ChannelOutput::write_state(float state) {
  if (parent_ == nullptr)
    return;
  parent_->write_channel(channel_, state);
}

}  // namespace drv8243
}  // namespace esphome
