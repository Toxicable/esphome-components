#include "mlx90614.h"

#include "esphome/core/log.h"

namespace esphome {
namespace mlx90614 {

using namespace ::mlx90614_core::registers;

static const char *const TAG = "mlx90614";

void MLX90614Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MLX90614...");
  float ambient_c = 0.0f;
  if (!this->read_temp_c_(RegisterId::AMBIENT_TEMPERATURE, &ambient_c)) {
    ESP_LOGW(TAG, "Initial read failed (check wiring/address and SMBus mode)");
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
  LOG_SENSOR("  ", "Ambient", this->ambient_sensor_);
  LOG_SENSOR("  ", "Object", this->object_sensor_);
  LOG_SENSOR("  ", "Object2", this->object2_sensor_);
}

void MLX90614Component::update() {
  bool success = true;
  float value_c = 0.0f;

  const auto publish = [&](RegisterId id, sensor::Sensor *sensor) {
    if (sensor == nullptr) return;
    if (this->read_temp_c_(id, &value_c)) {
      sensor->publish_state(value_c);
    } else {
      const auto &info = register_info(id);
      ESP_LOGW(TAG, "Failed reading %s (0x%02X)", info.name, static_cast<unsigned>(info.address));
      success = false;
    }
  };

  publish(RegisterId::AMBIENT_TEMPERATURE, this->ambient_sensor_);
  publish(RegisterId::OBJECT1_TEMPERATURE, this->object_sensor_);
  publish(RegisterId::OBJECT2_TEMPERATURE, this->object2_sensor_);

  if (success) this->status_clear_warning();
  else this->status_set_warning();
}

bool MLX90614Component::read_temp_c_(RegisterId id, float *out_c) {
  if (out_c == nullptr) return false;

  uint16_t word = 0;
  if (!this->read_word_with_pec_(id, &word)) return false;

  if ((word & 0x8000u) != 0) {
    const auto &info = register_info(id);
    ESP_LOGW(TAG, "Error flag set for %s (0x%02X): 0x%04X", info.name,
             static_cast<unsigned>(info.address), word);
    return false;
  }

  *out_c = static_cast<float>(word) * 0.02f - 273.15f;
  return true;
}

bool MLX90614Component::read_word_with_pec_(RegisterId id, uint16_t *out_word) {
  if (out_word == nullptr) return false;

  const uint8_t command = register_address(id);
  uint8_t data[3]{0, 0, 0};
  if (!this->read_bytes(command, data, sizeof(data))) {
    ESP_LOGW(TAG, "I2C read failed for %s (0x%02X)", register_info(id).name, command);
    return false;
  }

  const uint8_t low = data[0];
  const uint8_t high = data[1];
  const uint8_t pec = data[2];
  const uint8_t addr_w = static_cast<uint8_t>(this->slave_address_ << 1);
  const uint8_t addr_r = static_cast<uint8_t>((this->slave_address_ << 1) | 1u);
  const uint8_t message[]{addr_w, command, addr_r, low, high};
  const uint8_t calculated = this->crc8_smbus_(message, sizeof(message));

  if (calculated != pec) {
    ESP_LOGW(TAG, "PEC mismatch for %s: got 0x%02X expected 0x%02X", register_info(id).name, pec,
             calculated);
    return false;
  }

  *out_word = static_cast<uint16_t>((static_cast<uint16_t>(high) << 8) | low);
  return true;
}

uint8_t MLX90614Component::crc8_smbus_(const uint8_t *data, size_t len) const {
  uint8_t crc = 0x00;
  for (size_t index = 0; index < len; index++) {
    crc ^= data[index];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80u) != 0 ? static_cast<uint8_t>((crc << 1) ^ 0x07u)
                               : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

}  // namespace mlx90614
}  // namespace esphome
