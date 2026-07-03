#include "mcf8316d_service.h"

#include <cmath>

namespace mcf8316d_core {

using namespace regs;

bool MCF8316DService::read_reg32(uint16_t offset, uint32_t &value) const {
  return this->bus_ != nullptr && this->bus_->read_register32(offset, &value);
}

bool MCF8316DService::read_reg16(uint16_t offset, uint16_t &value) const {
  return this->bus_ != nullptr && this->bus_->read_register16(offset, &value);
}

bool MCF8316DService::write_reg32(uint16_t offset, uint32_t value) const {
  return this->bus_ != nullptr && this->bus_->write_register32(offset, value);
}

bool MCF8316DService::update_bits32(uint16_t offset, uint32_t mask, uint32_t value) const {
  uint32_t current = 0;
  if (!this->read_reg32(offset, current)) {
    return false;
  }
  const uint32_t next = (current & ~mask) | (value & mask);
  if (next == current) {
    return true;
  }
  return this->write_reg32(offset, next);
}

bool MCF8316DService::set_brake_input(bool brake_on) const {
  const uint32_t value = brake_on ? PIN_CONFIG_BRAKE_INPUT_BRAKE : PIN_CONFIG_BRAKE_INPUT_NO_BRAKE;
  return this->update_bits32(REG_PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value);
}

bool MCF8316DService::read_brake_input(uint8_t &brake_input_code) const {
  uint32_t pin_config = 0;
  if (!this->read_reg32(REG_PIN_CONFIG, pin_config)) {
    return false;
  }
  brake_input_code = static_cast<uint8_t>((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 10);
  return true;
}

bool MCF8316DService::set_direction_input(DirectionInputMode mode) const {
  uint32_t value = PERI_CONFIG1_DIR_INPUT_HARDWARE;
  switch (mode) {
    case DirectionInputMode::CW:
      value = PERI_CONFIG1_DIR_INPUT_CW;
      break;
    case DirectionInputMode::CCW:
      value = PERI_CONFIG1_DIR_INPUT_CCW;
      break;
    case DirectionInputMode::HARDWARE:
    default:
      value = PERI_CONFIG1_DIR_INPUT_HARDWARE;
      break;
  }
  return this->update_bits32(REG_PERI_CONFIG1, PERI_CONFIG1_DIR_INPUT_MASK, value);
}

bool MCF8316DService::read_direction_input(uint8_t &direction_input_code) const {
  uint32_t peri_config1 = 0;
  if (!this->read_reg32(REG_PERI_CONFIG1, peri_config1)) {
    return false;
  }
  direction_input_code = static_cast<uint8_t>(peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK);
  return true;
}

bool MCF8316DService::write_speed_command_percent(float speed_percent) const {
  if (std::isnan(speed_percent)) {
    return false;
  }
  const float clamped = speed_percent < 0.0f ? 0.0f : (speed_percent > 100.0f ? 100.0f : speed_percent);
  const uint16_t digital_speed_ctrl = static_cast<uint16_t>(lroundf((clamped / 100.0f) * 32767.0f));
  uint32_t value = ALGO_DEBUG1_OVERRIDE_MASK;
  value |= (static_cast<uint32_t>(digital_speed_ctrl) << 16) & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK;
  return this->update_bits32(
    REG_ALGO_DEBUG1, ALGO_DEBUG1_OVERRIDE_MASK | ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK, value
  );
}

bool MCF8316DService::pulse_clear_faults() const {
  if (!this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, ALGO_CTRL1_CLR_FLT_MASK)) {
    return false;
  }
  this->bus_->delay_microseconds(2000u);
  const bool ok = this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, 0u);
  this->bus_->delay_microseconds(2000u);
  return ok;
}

bool MCF8316DService::pulse_watchdog_tickle() const {
  if (!this->update_bits32(
        REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK
      )) {
    return false;
  }
  return this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0u);
}

}  // namespace mcf8316d_core
