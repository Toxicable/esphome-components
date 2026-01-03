#include "mlx90614_esf.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mlx90614_esf {

static const char *const TAG = "mlx90614_esf";

void MLX90614ESFComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MLX90614...");
  // Basic comms check
  float ta_c;
  if (!this->read_temp_c_(RAM_TA_, &ta_c)) {
    ESP_LOGW(TAG, "Initial read failed (check wiring, address, and that device is in SMBus mode).");
  }
}

void MLX90614ESFComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MLX90614ESF:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Verify PEC: %s", verify_pec_ ? "true" : "false");
  LOG_UPDATE_INTERVAL(this);

  if (ambient_sensor_ != nullptr)
    LOG_SENSOR("  ", "Ambient", ambient_sensor_);
  if (object_sensor_ != nullptr)
    LOG_SENSOR("  ", "Object", object_sensor_);
  if (object2_sensor_ != nullptr)
    LOG_SENSOR("  ", "Object2", object2_sensor_);
}

void MLX90614ESFComponent::update() {
  float value;

  if (ambient_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TA_, &value)) {
      ambient_sensor_->publish_state(value);
    } else {
      ESP_LOGW(TAG, "Failed reading Ta (0x%02X)", RAM_TA_);
    }
  }

  if (object_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TOBJ1_, &value)) {
      object_sensor_->publish_state(value);
    } else {
      ESP_LOGW(TAG, "Failed reading Tobj1 (0x%02X)", RAM_TOBJ1_);
    }
  }

  if (object2_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TOBJ2_, &value)) {
      object2_sensor_->publish_state(value);
    } else {
      ESP_LOGW(TAG, "Failed reading Tobj2 (0x%02X)", RAM_TOBJ2_);
    }
  }
}

bool MLX90614ESFComponent::read_temp_c_(uint8_t ram_addr, float *out_c) {
  uint16_t word = 0;
  uint8_t pec = 0;

  if (!this->read_word_with_pec_(ram_addr, &word, &pec))
    return false;

  // Datasheet notes MSB is an error flag for linearized temperatures (Ta/To).
  if (word & 0x8000) {
    ESP_LOGW(TAG, "Error flag set for RAM 0x%02X: 0x%04X", ram_addr, word);
    return false;
  }

  // Conversion: 0.02 K/LSB, then C = K - 273.15
  const float temp_k = static_cast<float>(word) * 0.02f;
  *out_c = temp_k - 273.15f;
  return true;
}

bool MLX90614ESFComponent::read_word_with_pec_(uint8_t command, uint16_t *out_word, uint8_t *out_pec) {
  // SMBus "Read Word" returns: low byte, high byte, PEC
  uint8_t data[3]{0, 0, 0};

  // Many ESPHome I2C devices support "read_bytes(reg, buf, len)" which writes reg then reads.
  // If your build complains, replace with: this->write_read(&command, 1, data, 3)
  if (!this->read_bytes(command, data, 3)) {
    ESP_LOGW(TAG, "I2C read failed for command 0x%02X", command);
    return false;
  }

  const uint8_t low = data[0];
  const uint8_t high = data[1];
  const uint8_t pec = data[2];

  if (verify_pec_) {
    // PEC covers: (addr<<1 | W), command, (addr<<1 | R), low, high
    const uint8_t addr_w = (slave_address_ << 1) | 0;
    const uint8_t addr_r = (slave_address_ << 1) | 1;
    const uint8_t msg[] = {addr_w, command, addr_r, low, high};
    const uint8_t calc = this->crc8_(msg, sizeof(msg));
    if (calc != pec) {
      ESP_LOGW(TAG, "PEC mismatch cmd 0x%02X: got 0x%02X expected 0x%02X", command, pec, calc);
      return false;
    }
  }

  *out_word = (static_cast<uint16_t>(high) << 8) | low;
  *out_pec = pec;
  return true;
}

uint8_t MLX90614ESFComponent::crc8_(const uint8_t *data, size_t len) const {
  // SMBus PEC is CRC-8 poly 0x07 (x^8 + x^2 + x + 1), MSB-first.
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x07;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

}  // namespace mlx90614_esf
}  // namespace esphome
