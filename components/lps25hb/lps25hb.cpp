#include "lps25hb.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lps25hb {

static const char *const TAG = "lps25hb";

static constexpr uint8_t REG_WHO_AM_I = 0x0F;
static constexpr uint8_t WHO_AM_I_LPS25HB = 0xBD;

static constexpr uint8_t REG_CTRL_REG1 = 0x20;
static constexpr uint8_t REG_CTRL_REG2 = 0x21;
static constexpr uint8_t REG_STATUS = 0x27;
static constexpr uint8_t REG_PRESS_OUT_XL = 0x28;
static constexpr uint8_t REG_TEMP_OUT_L = 0x2B;

static constexpr uint8_t CTRL1_PD = 0x80;
static constexpr uint8_t CTRL1_BDU = 0x04;
static constexpr uint8_t CTRL2_ONE_SHOT = 0x01;

void LPS25HBComponent::setup() {
  uint8_t who = 0;
  if (!this->read_who_am_i_(who)) {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I over I2C");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "0x%02X: WHO_AM_I = 0x%02X", this->address_, who);
  if (who != WHO_AM_I_LPS25HB) {
    ESP_LOGE(TAG, "Unexpected WHO_AM_I (0x%02X). Expected 0x%02X (LPS25HB).", who,
             WHO_AM_I_LPS25HB);
    this->mark_failed();
    return;
  }

  // Active mode, one-shot output data rate, and block-data update.
  const uint8_t ctrl1 = CTRL1_PD | CTRL1_BDU;
  if (!this->write_byte(REG_CTRL_REG1, ctrl1)) {
    ESP_LOGE(TAG, "Failed to write CTRL_REG1");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "LPS25HB detected and initialized in one-shot mode");
}

void LPS25HBComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "LPS25HB:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed())
    ESP_LOGE(TAG, "Communication failed");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Pressure", this->pressure_sensor_);
}

void LPS25HBComponent::update() {
  if (this->is_failed())
    return;

  if (!this->trigger_one_shot_()) {
    ESP_LOGW(TAG, "Failed to trigger one-shot");
    this->status_set_warning();
    return;
  }

  if (!this->wait_data_ready_(50)) {
    ESP_LOGW(TAG, "Timed out waiting for data-ready");
    this->status_set_warning();
    return;
  }

  float temperature_c = NAN;
  float pressure_hpa = NAN;
  if (!this->read_measurements_(temperature_c, pressure_hpa)) {
    ESP_LOGW(TAG, "Failed to read measurements");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temperature_c);
  if (this->pressure_sensor_ != nullptr)
    this->pressure_sensor_->publish_state(pressure_hpa);
}

bool LPS25HBComponent::read_who_am_i_(uint8_t &who) {
  return this->read_byte(REG_WHO_AM_I, &who);
}

bool LPS25HBComponent::trigger_one_shot_() {
  return this->write_byte(REG_CTRL_REG2, CTRL2_ONE_SHOT);
}

bool LPS25HBComponent::wait_data_ready_(uint32_t timeout_ms) {
  const uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    uint8_t status = 0;
    if (!this->read_byte(REG_STATUS, &status))
      return false;
    if ((status & 0x03) == 0x03)
      return true;
    delay(5);
  }
  return false;
}

bool LPS25HBComponent::read_measurements_(float &temperature_c,
                                          float &pressure_hpa) {
  uint8_t pressure_xl = 0;
  uint8_t pressure_l = 0;
  uint8_t pressure_h = 0;
  if (!this->read_byte(REG_PRESS_OUT_XL, &pressure_xl) ||
      !this->read_byte(REG_PRESS_OUT_XL + 1, &pressure_l) ||
      !this->read_byte(REG_PRESS_OUT_XL + 2, &pressure_h)) {
    return false;
  }

  int32_t pressure_raw = (static_cast<int32_t>(pressure_h) << 16) |
                         (static_cast<int32_t>(pressure_l) << 8) |
                         static_cast<int32_t>(pressure_xl);
  if (pressure_raw & 0x00800000)
    pressure_raw |= 0xFF000000;
  pressure_hpa = static_cast<float>(pressure_raw) / 4096.0f;

  uint8_t temperature_l = 0;
  uint8_t temperature_h = 0;
  if (!this->read_byte(REG_TEMP_OUT_L, &temperature_l) ||
      !this->read_byte(REG_TEMP_OUT_L + 1, &temperature_h)) {
    return false;
  }

  const int16_t temperature_raw = static_cast<int16_t>(
      (static_cast<uint16_t>(temperature_h) << 8) |
      static_cast<uint16_t>(temperature_l));
  temperature_c = 42.5f + static_cast<float>(temperature_raw) / 480.0f;
  return true;
}

}  // namespace lps25hb
}  // namespace esphome
