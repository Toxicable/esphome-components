#if !defined(MCF8329A_EMBED_IMPL) || defined(MCF8329A_EMBED_IMPL_INCLUDE)

#include "mcf8329a_client.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcf8329a {

using namespace regs;

static const char* const CLIENT_TAG = "mcf8329a";
static constexpr uint32_t WRITE_TRANSACTION_DELAY_US = 100u;
static constexpr float CLIENT_VM_VOLTAGE_SCALE = 60.0f / 134217728.0f;  // 60 / 2^27
static constexpr float CLIENT_SPEED_Q27_SCALE = 1.0f / 134217728.0f;    // 1 / 2^27
static constexpr float CLIENT_OPEN_LOOP_ACCEL_HZ_PER_S_TABLE[16] = {
  0.01f,
  0.05f,
  1.0f,
  2.5f,
  5.0f,
  10.0f,
  25.0f,
  50.0f,
  75.0f,
  100.0f,
  250.0f,
  500.0f,
  750.0f,
  1000.0f,
  5000.0f,
  10000.0f,
};
static constexpr float CLIENT_OPEN_TO_CLOSED_HANDOFF_PERCENT_TABLE[32] = {
  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,  9.0f,  10.0f, 11.0f,
  12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 22.5f, 25.0f,
  27.5f, 30.0f, 32.5f, 35.0f, 37.5f, 40.0f, 42.5f, 45.0f, 47.5f, 50.0f,
};

bool MCF8329AClient::ensure_device_() const {
  if (this->device_ != nullptr) {
    return true;
  }
  ESP_LOGW(CLIENT_TAG, "MCF8329A client not initialized: missing I2C device");
  return false;
}

bool MCF8329AClient::read_reg32(uint16_t offset, uint32_t& value) const {
  if (!this->ensure_device_()) {
    return false;
  }

  const uint32_t control_word = this->build_control_word_(true, offset, true);
  const uint8_t cw[3] = {
    static_cast<uint8_t>((control_word >> 16) & 0xFF),
    static_cast<uint8_t>((control_word >> 8) & 0xFF),
    static_cast<uint8_t>(control_word & 0xFF),
  };

  uint8_t rx[4] = {0, 0, 0, 0};
  const i2c::ErrorCode err = this->device_->write_read(cw, sizeof(cw), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(CLIENT_TAG, "read_reg32(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }

  value = static_cast<uint32_t>(rx[0]) | (static_cast<uint32_t>(rx[1]) << 8) |
          (static_cast<uint32_t>(rx[2]) << 16) | (static_cast<uint32_t>(rx[3]) << 24);
  return true;
}

bool MCF8329AClient::read_reg16(uint16_t offset, uint16_t& value) const {
  if (!this->ensure_device_()) {
    return false;
  }

  const uint32_t control_word = this->build_control_word_(true, offset, false);
  const uint8_t cw[3] = {
    static_cast<uint8_t>((control_word >> 16) & 0xFF),
    static_cast<uint8_t>((control_word >> 8) & 0xFF),
    static_cast<uint8_t>(control_word & 0xFF),
  };

  uint8_t rx[2] = {0, 0};
  const i2c::ErrorCode err = this->device_->write_read(cw, sizeof(cw), rx, sizeof(rx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(CLIENT_TAG, "read_reg16(0x%04X) failed: i2c error %d", offset, static_cast<int>(err));
    return false;
  }

  value = static_cast<uint16_t>(rx[0]) | (static_cast<uint16_t>(rx[1]) << 8);
  return true;
}

bool MCF8329AClient::write_reg32(uint16_t offset, uint32_t value) const {
  if (!this->ensure_device_()) {
    return false;
  }

  const uint32_t control_word = this->build_control_word_(false, offset, true);
  const uint8_t cw[3] = {
    static_cast<uint8_t>((control_word >> 16) & 0xFF),
    static_cast<uint8_t>((control_word >> 8) & 0xFF),
    static_cast<uint8_t>(control_word & 0xFF),
  };
  const uint8_t payload[4] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 24) & 0xFF),
  };

  const uint8_t tx[7] = {cw[0], cw[1], cw[2], payload[0], payload[1], payload[2], payload[3]};
  const i2c::ErrorCode err = this->device_->write(tx, sizeof(tx));
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(
      CLIENT_TAG, "write_reg32(0x%04X, 0x%08X) failed: i2c error %d", offset, value, static_cast<int>(err)
    );
    return false;
  }

  delay_microseconds_safe(WRITE_TRANSACTION_DELAY_US);
  return true;
}

bool MCF8329AClient::update_bits32(uint16_t offset, uint32_t mask, uint32_t value) const {
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

float MCF8329AClient::decode_vm_voltage(uint32_t raw) const {
  return static_cast<float>(static_cast<double>(raw) * CLIENT_VM_VOLTAGE_SCALE);
}

float MCF8329AClient::decode_max_speed_hz(uint16_t code) const {
  const uint16_t clamped = code & 0x3FFFu;
  if (clamped <= 9600u) {
    return static_cast<float>(clamped) / 6.0f;
  }
  return (static_cast<float>(clamped) / 4.0f) - 800.0f;
}

float MCF8329AClient::decode_speed_hz(int32_t raw, float max_speed_hz) const {
  return static_cast<float>(raw) * CLIENT_SPEED_Q27_SCALE * max_speed_hz;
}

float MCF8329AClient::decode_fg_speed_hz(uint32_t raw, float max_speed_hz) const {
  return static_cast<float>(raw) * CLIENT_SPEED_Q27_SCALE * max_speed_hz;
}

float MCF8329AClient::decode_open_loop_accel_hz_per_s(uint8_t code) const {
  return CLIENT_OPEN_LOOP_ACCEL_HZ_PER_S_TABLE[code & 0x0Fu];
}

float MCF8329AClient::decode_open_to_closed_handoff_percent(uint8_t code) const {
  return CLIENT_OPEN_TO_CLOSED_HANDOFF_PERCENT_TABLE[code & 0x1Fu];
}

bool MCF8329AClient::set_brake_input(bool brake_on) const {
  const uint32_t value = brake_on ? PIN_CONFIG_BRAKE_INPUT_BRAKE : PIN_CONFIG_BRAKE_INPUT_NO_BRAKE;
  return this->update_bits32(REG_PIN_CONFIG, PIN_CONFIG_BRAKE_INPUT_MASK, value);
}

bool MCF8329AClient::read_brake_input(uint8_t& brake_input_code) const {
  uint32_t pin_config = 0;
  if (!this->read_reg32(REG_PIN_CONFIG, pin_config)) {
    return false;
  }
  brake_input_code = static_cast<uint8_t>((pin_config & PIN_CONFIG_BRAKE_INPUT_MASK) >> 2);
  return true;
}

bool MCF8329AClient::set_direction_input(DirectionInputMode mode) const {
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

bool MCF8329AClient::read_direction_input(uint8_t& direction_input_code) const {
  uint32_t peri_config1 = 0;
  if (!this->read_reg32(REG_PERI_CONFIG1, peri_config1)) {
    return false;
  }
  direction_input_code = static_cast<uint8_t>((peri_config1 & PERI_CONFIG1_DIR_INPUT_MASK) >> 19);
  return true;
}

bool MCF8329AClient::write_speed_command_raw(uint16_t digital_speed_ctrl) const {
  const uint16_t clamped = digital_speed_ctrl & 0x7FFFu;
  const uint32_t value =
    ALGO_DEBUG1_OVERRIDE_MASK | ((static_cast<uint32_t>(clamped) << 16) & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK);
  return this->update_bits32(
    REG_ALGO_DEBUG1, ALGO_DEBUG1_OVERRIDE_MASK | ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK, value
  );
}

bool MCF8329AClient::set_mpet_characterization_bits() const {
  return this->update_bits32(REG_ALGO_DEBUG2, ALGO_DEBUG2_MPET_ALL_MASK, ALGO_DEBUG2_MPET_ALL_MASK);
}

bool MCF8329AClient::pulse_clear_faults() const {
  if (!this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, ALGO_CTRL1_CLR_FLT_MASK)) {
    return false;
  }
  delay_microseconds_safe(2000);
  bool ok = this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_CLR_FLT_MASK, 0u);
  delay_microseconds_safe(2000);
  return ok;
}

bool MCF8329AClient::pulse_watchdog_tickle() const {
  if (!this->update_bits32(
        REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, ALGO_CTRL1_WATCHDOG_TICKLE_MASK
      )) {
    return false;
  }
  return this->update_bits32(REG_ALGO_CTRL1, ALGO_CTRL1_WATCHDOG_TICKLE_MASK, 0u);
}

bool MCF8329AClient::clear_mpet_bits(bool* changed, uint32_t* before, uint32_t* after) const {
  uint32_t algo_debug2 = 0;
  if (!this->read_reg32(REG_ALGO_DEBUG2, algo_debug2)) {
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
  if (!this->write_reg32(REG_ALGO_DEBUG2, next)) {
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

uint32_t MCF8329AClient::build_control_word_(bool is_read, uint16_t offset, bool is_32bit) const {
  const uint32_t dlen = is_32bit ? 0x1u : 0x0u;
  return ((is_read ? 1u : 0u) << 23) | (dlen << 20) | (static_cast<uint32_t>(offset) & 0x0FFFu);
}

}  // namespace mcf8329a
}  // namespace esphome

#endif  // !MCF8329A_EMBED_IMPL || MCF8329A_EMBED_IMPL_INCLUDE
