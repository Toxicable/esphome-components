#include "mcf8329a_service.h"

namespace mcf8329a_core {

using namespace regs;

bool MCF8329AService::read_reg32(RegisterId id, uint32_t &value) const {
  return this->registers_.read32(register_address(id), value);
}

bool MCF8329AService::read_reg16(RegisterId id, uint16_t &value) const {
  return this->registers_.read16(register_address(id), value);
}

bool MCF8329AService::write_reg32(RegisterId id, uint32_t value) const {
  return this->registers_.write32(register_address(id), value);
}

bool MCF8329AService::update_bits32(RegisterId id, uint32_t mask, uint32_t value) const {
  return this->registers_.update_bits32(register_address(id), mask, value);
}

float MCF8329AService::decode_vm_voltage(uint32_t raw) const {
  return ::mcf8329a_core::decode_vm_voltage(raw);
}

float MCF8329AService::decode_max_speed_hz(uint16_t code) const {
  return ::mcf8329a_core::decode_max_speed_hz(code);
}

float MCF8329AService::decode_speed_hz(int32_t raw, float max_speed_hz) const {
  return ::mcf8329a_core::decode_speed_hz(raw, max_speed_hz);
}

float MCF8329AService::decode_fg_speed_hz(uint32_t raw, float max_speed_hz) const {
  return ::mcf8329a_core::decode_fg_speed_hz(raw, max_speed_hz);
}

float MCF8329AService::decode_open_loop_accel_hz_per_s(uint8_t code) const {
  return ::mcf8329a_core::decode_open_loop_accel_hz_per_s(code);
}

float MCF8329AService::decode_open_to_closed_handoff_percent(uint8_t code) const {
  return ::mcf8329a_core::decode_open_to_closed_handoff_percent(code);
}

bool MCF8329AService::set_brake_input(bool brake_on) const {
  const uint32_t value = brake_on ? PIN_CONFIG_BRAKE_INPUT_BRAKE : PIN_CONFIG_BRAKE_INPUT_NO_BRAKE;
  return this->update_bits32(RegisterId::PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value);
}

bool MCF8329AService::read_brake_input(uint8_t &brake_input_code) const {
  uint32_t pin_config = 0;
  if (!this->read_reg32(RegisterId::PIN_CONFIG, pin_config)) {
    return false;
  }
  brake_input_code = static_cast<uint8_t>((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 2);
  return true;
}

bool MCF8329AService::set_direction_input(DirectionInputMode mode) const {
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
  return this->update_bits32(RegisterId::PERI_CONFIG1, PERI_CONFIG1_DIR_INPUT_MASK, value);
}

bool MCF8329AService::read_direction_input(uint8_t &direction_input_code) const {
  uint32_t peri_config1 = 0;
  if (!this->read_reg32(RegisterId::PERI_CONFIG1, peri_config1)) {
    return false;
  }
  direction_input_code = static_cast<uint8_t>((peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK) >> 19);
  return true;
}

bool MCF8329AService::write_speed_command_raw(uint16_t digital_speed_ctrl) const {
  const uint16_t clamped = digital_speed_ctrl & 0x7FFFu;
  const uint32_t value =
    ALGO_DEBUG1_OVERRIDE_MASK | ((static_cast<uint32_t>(clamped) << 16) & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK);
  return this->update_bits32(
    register_address(RegisterId::ALGO_DEBUG1), ALGO_DEBUG1_OVERRIDE_MASK | ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK, value
  );
}

bool MCF8329AService::release_speed_override() const {
  return this->update_bits32(RegisterId::ALGO_DEBUG1, ALGO_DEBUG1_OVERRIDE_MASK, 0u);
}

bool MCF8329AService::set_mpet_characterization_bits() const {
  return this->update_bits32(RegisterId::ALGO_DEBUG2, ALGO_DEBUG2_MPET_RUN_MASK, ALGO_DEBUG2_MPET_RUN_MASK);
}

bool MCF8329AService::write_mpet_results_to_shadow() const {
  return this->registers_.pulse_bits32(
      register_address(RegisterId::ALGO_DEBUG2), ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK, 2000U);
}

bool MCF8329AService::pulse_clear_faults() const {
  return this->registers_.pulse_bits32(register_address(RegisterId::ALGO_CTRL1), ALGO_CTRL1_CLR_FLT_MASK, 2000U, 2000U);
}

bool MCF8329AService::pulse_watchdog_tickle() const {
  return this->registers_.pulse_bits32(register_address(RegisterId::ALGO_CTRL1), ALGO_CTRL1_WATCHDOG_TICKLE_MASK);
}

bool MCF8329AService::clear_mpet_bits(bool *changed, uint32_t *before, uint32_t *after) const {
  uint32_t algo_debug2 = 0;
  if (!this->read_reg32(RegisterId::ALGO_DEBUG2, algo_debug2)) {
    return false;
  }
  if (before != nullptr) {
    *before = algo_debug2;
  }

  const uint32_t mpet_bits = algo_debug2 & ALGO_DEBUG2_MPET_ALL_MASK;
  if (mpet_bits == 0u) {
    if (changed != nullptr) {
      *changed = false;
    }
    if (after != nullptr) {
      *after = algo_debug2;
    }
    return true;
  }

  const uint32_t next = algo_debug2 & ~ALGO_DEBUG2_MPET_ALL_MASK;
  if (!this->write_reg32(RegisterId::ALGO_DEBUG2, next)) {
    return false;
  }
  if (changed != nullptr) {
    *changed = true;
  }
  if (after != nullptr) {
    *after = next;
  }
  return true;
}

}  // namespace mcf8329a_core
