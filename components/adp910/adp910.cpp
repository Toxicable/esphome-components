#include "adp910.h"

#include "esphome/core/log.h"

namespace esphome {
namespace adp910 {

static const char *const TAG = "adp910";

bool ADP910Component::verify_product_type_() {
  const uint8_t command[] = {0xE2, 0x01};
  uint8_t response[3] = {0, 0, 0};
  if (this->write_read(command, sizeof(command), response, sizeof(response)) != i2c::ERROR_OK) {
    return false;
  }

  const uint8_t product_data[] = {response[0], response[1]};
  return response[0] == 0x41 && response[1] == 0x53 &&
         response[2] == crc8_(product_data, sizeof(product_data));
}

void ADP910Component::setup() {
  if (!this->verify_product_type_()) {
    ESP_LOGE(TAG, "ADP910 product-type check failed at I2C address 0x25");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "ADP910 detected (product type 0x4153)");
}

void ADP910Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ADP910:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Differential pressure", this->pressure_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  ESP_LOGCONFIG(TAG, "  Pressure scale: raw / 60 Pa");
  ESP_LOGCONFIG(TAG, "  Temperature scale: raw / 200 C");
}

void ADP910Component::update() {
  if (this->is_failed()) {
    return;
  }

  int16_t pressure_raw = 0;
  int16_t temperature_raw = 0;
  if (!this->read_measurement_(pressure_raw, temperature_raw)) {
    ESP_LOGW(TAG, "Failed to read or validate measurement");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  if (this->pressure_sensor_ != nullptr) {
    this->pressure_sensor_->publish_state(static_cast<float>(pressure_raw) / 60.0f);
  }
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(static_cast<float>(temperature_raw) / 200.0f);
  }
}

bool ADP910Component::read_measurement_(int16_t &pressure_raw, int16_t &temperature_raw) {
  const uint8_t command[] = {0x36, 0x1E};
  uint8_t response[6] = {0, 0, 0, 0, 0, 0};
  if (this->write_read(command, sizeof(command), response, sizeof(response)) != i2c::ERROR_OK) {
    return false;
  }

  const uint8_t pressure_data[] = {response[0], response[1]};
  const uint8_t temperature_data[] = {response[3], response[4]};
  if (response[2] != crc8_(pressure_data, sizeof(pressure_data)) ||
      response[5] != crc8_(temperature_data, sizeof(temperature_data))) {
    ESP_LOGW(TAG, "Measurement CRC mismatch");
    return false;
  }

  pressure_raw = static_cast<int16_t>(
      (static_cast<uint16_t>(response[0]) << 8) | static_cast<uint16_t>(response[1]));
  temperature_raw = static_cast<int16_t>(
      (static_cast<uint16_t>(response[3]) << 8) | static_cast<uint16_t>(response[4]));
  return true;
}

uint8_t ADP910Component::crc8_(const uint8_t *data, size_t length) {
  uint8_t crc = 0xFF;
  for (size_t index = 0; index < length; index++) {
    crc ^= data[index];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80u) != 0 ? static_cast<uint8_t>((crc << 1) ^ 0x31u)
                               : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

}  // namespace adp910
}  // namespace esphome
