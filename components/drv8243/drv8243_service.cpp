#include "drv8243_service.h"

namespace drv8243_core {

static constexpr uint32_t SLEEP_FORCE_MS = 2;
static constexpr uint32_t READY_WAIT_TIMEOUT_US = 5000;
static constexpr uint32_t ACK_WAIT_TIMEOUT_US = 5000;
static constexpr uint32_t POLL_STEP_US = 10;
static constexpr uint32_t ACK_PULSE_US = 22;

HandshakeResult Drv8243Service::handshake() {
  if (this->bus_ == nullptr)
    return HandshakeResult::FAILED;

  this->bus_->write_nsleep(false);
  this->bus_->delay_ms(SLEEP_FORCE_MS);
  this->bus_->write_nsleep(true);

  bool saw_ready_low = false;
  for (uint32_t elapsed_us = 0; elapsed_us < READY_WAIT_TIMEOUT_US; elapsed_us += POLL_STEP_US) {
    bool nfault_high = true;
    if (this->bus_->read_nfault(&nfault_high) && !nfault_high) {
      saw_ready_low = true;
      break;
    }
    this->bus_->delay_us(POLL_STEP_US);
  }

  this->bus_->write_nsleep(false);
  this->bus_->delay_us(ACK_PULSE_US);
  this->bus_->write_nsleep(true);

  if (!saw_ready_low)
    return HandshakeResult::FAILED;

  for (uint32_t elapsed_us = 0; elapsed_us < ACK_WAIT_TIMEOUT_US; elapsed_us += POLL_STEP_US) {
    bool nfault_high = false;
    if (this->bus_->read_nfault(&nfault_high) && nfault_high)
      return HandshakeResult::SUCCESS;
    this->bus_->delay_us(POLL_STEP_US);
  }

  return HandshakeResult::FAILED;
}

void Drv8243Service::set_static_polarity(bool level) {
  if (this->bus_ != nullptr)
    this->bus_->write_out2(level);
}

}  // namespace drv8243_core
