#include "lps25hb.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lps25hb {

static const char *const TAG = "lps25hb";

// Registers (LPS25HB)
static constexpr uint8_t REG_WHO_AM_I = 0x0F;
static constexpr uint8_t WHO_AM_I_LPS25HB = 0xBD;

static constexpr uint8_t REG_CTRL_REG1 = 0x20;
static constexpr uint8_t REG_CTRL_REG2 = 0x21;
static constexpr uint8_t REG_STATUS = 0x27;

static constexpr uint8_t REG_PRESS_OUT_XL = 0x28; // 0x28..0x2A
static constexpr uint8_t REG_TEMP_OUT_L = 0x2B;   // 0x2B..0x2C

// CTRL_REG1 bits per datasheet: PD bit enables device; ODR=000 enables one-shot; BDU blocks partial updates.
// :contentReference[oaicite:1]{index=1}
static constexpr uint8_t CTRL1_PD = 0x80;
static constexpr uint8_t CTRL1_BDU = 0x04;
// ODR[2:0] are bits 6..4; leaving them 0 => one-shot mode enabled. :contentReference[oaicite:2]{index=2}

// CTRL_REG2: ONE_SHOT is bit 0; writing 1 triggers a new dataset in one-shot mode.
// :contentReference[oaicite:3]{index=3}
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
    ESP_LOGE(TAG, "Unexpected WHO_AM_I (0x%02X). Expected 0x%02X (LPS25HB).", who, WHO_AM_I_LPS25HB);
    this->mark_failed();
    return;
  }

  // Put device into active mode, keep one-shot (ODR=000), enable BDU.
  // CTRL_REG1 layout shown in datasheet; PD=1 => active. :contentReference[oaicite:4]{index=4}
  uint8_t ctrl1 = CTRL1_PD | CTRL1_BDU;
  if (!this->write_byte(REG_CTRL_REG1, ctrl1)) {
    ESP_LOGE(TAG, "Failed to write CTRL_REG1");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "LPS25HB detected and initialized (one-shot mode).");
}

void LPS25HBComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "LPS25HB:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
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

  float t_c = NAN;
  float p_hpa = NAN;
  if (!this->read_measurements_(t_c, p_hpa)) {
    ESP_LOGW(TAG, "Failed to read measurements");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();

  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(t_c);
  if (this->pressure_sensor_ != nullptr)
    this->pressure_sensor_->publish_state(p_hpa);
}

bool LPS25HBComponent::read_who_am_i_(uint8_t &who) {
  return this->read_byte(REG_WHO_AM_I, &who);
}

bool LPS25HBComponent::trigger_one_shot_() {
  // ONE_SHOT bit triggers a new dataset when ODR=000 (one-shot mode). :contentReference[oaicite:5]{index=5}
  return this->write_byte(REG_CTRL_REG2, CTRL2_ONE_SHOT);
}

bool LPS25HBComponent::wait_data_ready_(uint32_t timeout_ms) {
  const uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    uint8_t status = 0;
    if (!this->read_byte(REG_STATUS, &status)) {
      return false;
    }
    // STATUS_REG bits: P_DA and T_DA indicate new pressure/temp data; common ST pattern uses bits 1:0.
    if ((status & 0x03) == 0x03) {
      return true;
    }
    delay(5);
  }
  return false;
}

bool LPS25HBComponent::read_measurements_(float &temp_c, float &pressure_hpa) {
  // Pressure is 24-bit in registers 0x28..0x2A; temperature is 16-bit in 0x2B..0x2C.
  // :contentReference[oaicite:6]{index=6}
  uint8_t pxl = 0, pl = 0, ph = 0;
  if (!this->read_byte(REG_PRESS_OUT_XL, &pxl))
    return false;
  if (!this->read_byte(REG_PRESS_OUT_XL + 1, &pl))
    return false;
  if (!this->read_byte(REG_PRESS_OUT_XL + 2, &ph))
    return false;

  int32_t p_raw = (static_cast<int32_t>(ph) << 16) | (static_cast<int32_t>(pl) << 8) | static_cast<int32_t>(pxl);
  // Sign-extend 24-bit if needed (datasheet describes pressure output as 24-bit). :contentReference[oaicite:7]{index=7}
  if (p_raw & 0x00800000)
    p_raw |= 0xFF000000;

  // Convert: pressure(mbar/hPa) = raw / 4096. :contentReference[oaicite:8]{index=8}
  pressure_hpa = static_cast<float>(p_raw) / 4096.0f;

  uint8_t tl = 0, th = 0;
  if (!this->read_byte(REG_TEMP_OUT_L, &tl))
    return false;
  if (!this->read_byte(REG_TEMP_OUT_L + 1, &th))
    return false;

  int16_t t_raw = static_cast<int16_t>((static_cast<uint16_t>(th) << 8) | static_cast<uint16_t>(tl));
  // Convert: Temp(°C) = 42.5 + raw/480. :contentReference[oaicite:9]{index=9}
  temp_c = 42.5f + (static_cast<float>(t_raw) / 480.0f);

  return true;
}

} // namespace lps25hb
} // namespace esphome
