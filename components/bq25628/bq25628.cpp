#include "bq25628.h"

#include "esphome/core/log.h"

namespace esphome {
namespace bq25628 {

static const char *const TAG = "bq25628";

void BQ25628Component::setup() {
  if (!this->service_.probe()) {
    ESP_LOGE(TAG, "BQ25628E not detected at address 0x%02X", this->address_);
    this->mark_failed();
    return;
  }
  if (!this->service_.enable_adc()) {
    ESP_LOGE(TAG, "Unable to enable the charger ADC");
    this->mark_failed();
    return;
  }
  ESP_LOGI(TAG, "BQ25628E detected; charger ADC enabled");
}

void BQ25628Component::update() {
  if (this->is_failed())
    return;

  float battery_voltage_v;
  if (!this->service_.read_battery_voltage_v(battery_voltage_v)) {
    ESP_LOGW(TAG, "Unable to read battery voltage");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  if (this->battery_voltage_sensor_ != nullptr)
    this->battery_voltage_sensor_->publish_state(battery_voltage_v);
}

void BQ25628Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ25628E:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed())
    ESP_LOGE(TAG, "Communication failed");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_sensor_);
}

bool BQ25628Component::read_registers(uint8_t reg, uint8_t *data, size_t len) {
  return this->read_bytes(reg, data, len);
}

bool BQ25628Component::write_registers(uint8_t reg, const uint8_t *data, size_t len) {
  return this->write_bytes(reg, data, len);
}

}  // namespace bq25628
}  // namespace esphome
