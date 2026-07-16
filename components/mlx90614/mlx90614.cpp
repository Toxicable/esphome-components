#include "mlx90614.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mlx90614 {

static const char *const TAG = "mlx90614";

void MLX90614Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MLX90614...");
  float ambient_c;
  if (!this->read_temp_c_(RAM_TA_, &ambient_c)) {
    ESP_LOGW(TAG, "Initial read failed (check wiring/address, and that device is in SMBus mode).");
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();
}

void MLX90614Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MLX90614:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  PEC/CRC: enabled (always verified)");
  LOG_UPDATE_INTERVAL(this);

  if (this->ambient_sensor_ != nullptr)
    LOG_SENSOR("  ", "Ambient", this->ambient_sensor_);
  if (this->object_sensor_ != nullptr)
    LOG_SENSOR("  ", "Object", this->object_sensor_);
  if (this->object2_sensor_ != nullptr)
    LOG_SENSOR("  ", "Object2", this->object2_sensor_);
}

void MLX90614Component::update() {
  bool success = true;
  float value_c;

  if (this->ambient_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TA_, &value_c)) {
      this->ambient_sensor_->publish_state(value_c);
    } else {
      ESP_LOGW(TAG, "Failed reading Ta (0x%02X)", RAM_TA_);
      success = false;
    }
  }

  if (this->object_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TOBJ1_, &value_c)) {
      this->object_sensor_->publish_state(value_c);
    } else {
      ESP_LOGW(TAG, "Failed reading Tobj1 (0x%02X)", RAM_TOBJ1_);
      success = false;
    }
  }

  if (this->object2_sensor_ != nullptr) {
    if (this->read_temp_c_(RAM_TOBJ2_, &value_c)) {
      this->object2_sensor_->publish_state(value_c);
    } else {
      ESP_LOGW(TAG, "Failed reading Tobj2 (0x%02X)", RAM_TOBJ2_);
      success = false;
    }
  }

  if (success) {
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

bool MLX90614Component::read_temp_c_(uint8_t ram_addr, float *out_c) {
  uint16_t word = 0;
  if (!this->read_word_with_pec_(ram_addr, &word))
    return false;

  // The most-significant bit is an error flag for linearized temperatures.
  if (word & 0x8000) {
    ESP_LOGW(TAG, "Error flag set for RAM 0x%02X: 0x%04X", ram_addr, word);
    return false;
  }

  // 0.02 K/LSB, then °C = K - 273.15.
  const float temp_k = static_cast<float>(word) * 0.02f;
  *out_c = temp_k - 273.15f;
  return true;
}

bool MLX90614Component::read_word_with_pec_(uint8_t command, uint16_t *out_word) {
  // SMBus Read Word: low, high, PEC.
  uint8_t data[3]{0, 0, 0};
  if (!this->read_bytes(command, data, 3)) {
    ESP_LOGW(TAG, "I2C read failed for cmd 0x%02X", command);
    return false;
  }

  const uint8_t low = data[0];
  const uint8_t high = data[1];
  const uint8_t pec = data[2];

  // PEC covers: (addr<<1|W), command, (addr<<1|R), low, high.
  const uint8_t addr_w = (this->slave_address_ << 1) | 0;
  const uint8_t addr_r = (this->slave_address_ << 1) | 1;
  const uint8_t message[] = {addr_w, command, addr_r, low, high};
  const uint8_t calculated = this->crc8_smbus_(message, sizeof(message));

  if (calculated != pec) {
    ESP_LOGW(TAG, "PEC mismatch cmd 0x%02X: got 0x%02X expected 0x%02X", command, pec, calculated);
    return false;
  }

  *out_word = (static_cast<uint16_t>(high) << 8) | low;
  return true;
}

uint8_t MLX90614Component::crc8_smbus_(const uint8_t *data, size_t len) const {
  // SMBus PEC: CRC-8 polynomial 0x07, most-significant bit first.
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x07;
      else
        crc <<= 1;
    }
  }
  return crc;
}

}  // namespace mlx90614
}  // namespace esphome
