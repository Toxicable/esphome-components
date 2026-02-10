#include "fdc1004.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace fdc1004 {

static const char *const TAG = "fdc1004";

static constexpr uint8_t REG_MEAS1_MSB = 0x00;
static constexpr uint8_t REG_CONF_MEAS1 = 0x08;
static constexpr uint8_t REG_FDC_CONF = 0x0C;
static constexpr uint8_t REG_MANUFACTURER_ID = 0xFE;
static constexpr uint8_t REG_DEVICE_ID = 0xFF;

static constexpr uint16_t EXPECTED_MANUFACTURER_ID = 0x5449;
static constexpr uint16_t EXPECTED_DEVICE_ID = 0x1004;

static constexpr float CAPDAC_STEP_PF = 3.125f;
static constexpr float MEAS_LSB_PF = 1.0f / 524288.0f; // 2^19

void FDC1004Component::set_channel_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index >= this->channel_sensors_.size()) {
    return;
  }
  this->channel_sensors_[index] = sensor;
  if (sensor != nullptr) {
    this->enabled_mask_ |= static_cast<uint8_t>(1U << index);
  } else {
    this->enabled_mask_ &= static_cast<uint8_t>(~(1U << index));
  }
}

void FDC1004Component::set_channel_capdac(uint8_t index, uint8_t capdac_steps) {
  if (index >= this->capdac_steps_.size()) {
    return;
  }
  this->capdac_steps_[index] = capdac_steps > 31 ? 31 : capdac_steps;
}

void FDC1004Component::set_offset_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index >= this->offset_sensors_.size()) {
    return;
  }
  this->offset_sensors_[index] = sensor;
}

void FDC1004Component::set_sample_rate(uint16_t sample_rate_sps) {
  this->sample_rate_sps_ = sample_rate_sps;
  switch (sample_rate_sps) {
    case 100:
      this->rate_bits_ = 0b01;
      break;
    case 200:
      this->rate_bits_ = 0b10;
      break;
    case 400:
      this->rate_bits_ = 0b11;
      break;
    default:
      this->sample_rate_sps_ = 100;
      this->rate_bits_ = 0b01;
      break;
  }
}

void FDC1004Component::tare_to_current() {
  if (this->is_failed()) {
    ESP_LOGW(TAG, "Cannot tare while component is failed");
    return;
  }

  if (!this->initialized_) {
    ESP_LOGW(TAG, "Cannot tare: FDC1004 is not initialized yet");
    this->status_set_warning();
    return;
  }

  bool any_channel = false;
  bool all_ok = true;
  for (uint8_t i = 0; i < this->channel_sensors_.size(); i++) {
    if ((this->enabled_mask_ & static_cast<uint8_t>(1U << i)) == 0) {
      continue;
    }

    float capacitance_pf = 0.0f;
    if (!this->read_measurement_pf_(i, capacitance_pf)) {
      ESP_LOGW(TAG, "Tare read failed for measurement %u", i + 1);
      all_ok = false;
      continue;
    }

    this->tare_offsets_pf_[i] = capacitance_pf;
    if (this->offset_sensors_[i] != nullptr) {
      this->offset_sensors_[i]->publish_state(this->tare_offsets_pf_[i]);
    }
    any_channel = true;
    if (this->channel_sensors_[i] != nullptr) {
      this->channel_sensors_[i]->publish_state(0.0f);
    }
  }

  if (!any_channel || !all_ok) {
    this->status_set_warning();
    return;
  }

  ESP_LOGI(TAG, "Captured tare offsets from current readings");
  this->status_clear_warning();
}

void FDC1004Component::setup() {
  if (this->enabled_mask_ == 0) {
    ESP_LOGE(TAG, "No measurements enabled");
    this->mark_failed();
    return;
  }

  this->initialized_ = false;
  this->next_init_retry_ms_ = 0;

  if (!this->initialize_()) {
    ESP_LOGW(TAG, "FDC1004 not ready at startup; will retry initialization");
    this->status_set_warning();
    this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
  } else {
    this->status_clear_warning();
  }

  for (uint8_t i = 0; i < this->offset_sensors_.size(); i++) {
    if (this->offset_sensors_[i] != nullptr) {
      this->offset_sensors_[i]->publish_state(this->tare_offsets_pf_[i]);
    }
  }
}

void FDC1004Component::dump_config() {
  ESP_LOGCONFIG(TAG, "FDC1004:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Sample rate: %u S/s", this->sample_rate_sps_);
  ESP_LOGCONFIG(TAG, "  Initialized: %s", this->initialized_ ? "yes" : "no (waiting for device)");

  LOG_SENSOR("  ", "CIN1", this->channel_sensors_[0]);
  LOG_SENSOR("  ", "CIN2", this->channel_sensors_[1]);
  LOG_SENSOR("  ", "CIN3", this->channel_sensors_[2]);
  LOG_SENSOR("  ", "CIN4", this->channel_sensors_[3]);
  LOG_SENSOR("  ", "CIN1 Offset", this->offset_sensors_[0]);
  LOG_SENSOR("  ", "CIN2 Offset", this->offset_sensors_[1]);
  LOG_SENSOR("  ", "CIN3 Offset", this->offset_sensors_[2]);
  LOG_SENSOR("  ", "CIN4 Offset", this->offset_sensors_[3]);

  if (this->channel_sensors_[0] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN1 CAPDAC: %.3f pF", this->capdac_pf_(0));
  if (this->channel_sensors_[1] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN2 CAPDAC: %.3f pF", this->capdac_pf_(1));
  if (this->channel_sensors_[2] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN3 CAPDAC: %.3f pF", this->capdac_pf_(2));
  if (this->channel_sensors_[3] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN4 CAPDAC: %.3f pF", this->capdac_pf_(3));
  if (this->channel_sensors_[0] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN1 Tare offset: %.4f pF", this->tare_offsets_pf_[0]);
  if (this->channel_sensors_[1] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN2 Tare offset: %.4f pF", this->tare_offsets_pf_[1]);
  if (this->channel_sensors_[2] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN3 Tare offset: %.4f pF", this->tare_offsets_pf_[2]);
  if (this->channel_sensors_[3] != nullptr)
    ESP_LOGCONFIG(TAG, "  CIN4 Tare offset: %.4f pF", this->tare_offsets_pf_[3]);
}

void FDC1004Component::update() {
  if (this->is_failed()) {
    return;
  }

  const uint32_t now = millis();
  if (!this->initialized_) {
    if (now < this->next_init_retry_ms_) {
      return;
    }

    if (!this->initialize_()) {
      this->status_set_warning();
      this->next_init_retry_ms_ = now + INIT_RETRY_INTERVAL_MS;
      return;
    }

    this->status_clear_warning();
    return;
  }

  uint16_t fdc_conf = 0;
  if (!this->read_register16_(REG_FDC_CONF, fdc_conf)) {
    ESP_LOGW(TAG, "Lost communication with FDC1004; reinitializing");
    this->initialized_ = false;
    this->status_set_warning();
    this->next_init_retry_ms_ = now + INIT_RETRY_INTERVAL_MS;
    return;
  }

  uint8_t done_mask = 0;
  for (uint8_t i = 0; i < 4; i++) {
    const uint8_t done_bit = static_cast<uint8_t>(3U - i);
    if ((fdc_conf & static_cast<uint16_t>(1U << done_bit)) != 0) {
      done_mask |= static_cast<uint8_t>(1U << i);
    }
  }

  done_mask &= this->enabled_mask_;
  if (done_mask == 0) {
    ESP_LOGV(TAG, "No completed measurements yet");
    return;
  }

  bool all_reads_ok = true;
  for (uint8_t i = 0; i < this->channel_sensors_.size(); i++) {
    if ((done_mask & static_cast<uint8_t>(1U << i)) == 0) {
      continue;
    }

    float capacitance_pf = 0.0f;
    if (!this->read_measurement_pf_(i, capacitance_pf)) {
      ESP_LOGW(TAG, "Failed reading measurement %u", i + 1);
      all_reads_ok = false;
      continue;
    }

    if (this->channel_sensors_[i] != nullptr) {
      this->channel_sensors_[i]->publish_state(capacitance_pf - this->tare_offsets_pf_[i]);
    }
  }

  if (all_reads_ok) {
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

bool FDC1004Component::initialize_() {
  uint16_t manufacturer_id = 0;
  if (!this->read_register16_(REG_MANUFACTURER_ID, manufacturer_id)) {
    ESP_LOGD(TAG, "Init probe: failed to read manufacturer ID");
    return false;
  }

  uint16_t device_id = 0;
  if (!this->read_register16_(REG_DEVICE_ID, device_id)) {
    ESP_LOGD(TAG, "Init probe: failed to read device ID");
    return false;
  }

  if (manufacturer_id != EXPECTED_MANUFACTURER_ID || device_id != EXPECTED_DEVICE_ID) {
    ESP_LOGW(TAG, "Init probe IDs mismatch: manufacturer=0x%04X device_id=0x%04X", manufacturer_id, device_id);
    return false;
  }

  for (uint8_t i = 0; i < this->channel_sensors_.size(); i++) {
    if ((this->enabled_mask_ & static_cast<uint8_t>(1U << i)) == 0) {
      continue;
    }
    if (!this->configure_measurement_(i)) {
      ESP_LOGD(TAG, "Init probe: failed to configure measurement %u", i + 1);
      return false;
    }
  }

  if (!this->write_fdc_conf_()) {
    ESP_LOGD(TAG, "Init probe: failed to write FDC_CONF");
    return false;
  }

  this->initialized_ = true;
  ESP_LOGI(TAG, "FDC1004 initialized (%u S/s, mask=0x%X)", this->sample_rate_sps_, this->enabled_mask_);
  return true;
}

bool FDC1004Component::read_register16_(uint8_t reg, uint16_t &value) {
  uint8_t data[2] = {0, 0};
  if (!this->read_bytes(reg, data, sizeof(data))) {
    return false;
  }
  value = static_cast<uint16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);
  return true;
}

bool FDC1004Component::write_register16_(uint8_t reg, uint16_t value) {
  uint8_t data[2] = {
      static_cast<uint8_t>(value >> 8),
      static_cast<uint8_t>(value & 0xFF),
  };
  return this->write_bytes(reg, data, sizeof(data));
}

bool FDC1004Component::configure_measurement_(uint8_t measurement_index) {
  if (measurement_index >= 4) {
    return false;
  }

  const uint8_t cha = measurement_index;
  const uint8_t capdac_steps = this->capdac_steps_[measurement_index] & 0x1F;
  const uint8_t chb = capdac_steps > 0 ? 0b100 : 0b111;

  const uint16_t conf = static_cast<uint16_t>((cha << 13) | (chb << 10) | (capdac_steps << 5));
  return this->write_register16_(static_cast<uint8_t>(REG_CONF_MEAS1 + measurement_index), conf);
}

bool FDC1004Component::write_fdc_conf_() {
  uint16_t meas_bits = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if ((this->enabled_mask_ & static_cast<uint8_t>(1U << i)) == 0) {
      continue;
    }
    meas_bits |= static_cast<uint16_t>(1U << (7U - i));
  }

  const uint16_t fdc_conf = static_cast<uint16_t>((this->rate_bits_ << 10) | (1U << 8) | meas_bits);
  return this->write_register16_(REG_FDC_CONF, fdc_conf);
}

bool FDC1004Component::read_measurement_pf_(uint8_t measurement_index, float &capacitance_pf) {
  if (measurement_index >= 4) {
    return false;
  }

  const uint8_t meas_msb_reg = static_cast<uint8_t>(REG_MEAS1_MSB + (measurement_index * 2));
  const uint8_t meas_lsb_reg = static_cast<uint8_t>(meas_msb_reg + 1);

  uint16_t msb = 0;
  uint16_t lsb = 0;
  if (!this->read_register16_(meas_msb_reg, msb)) {
    return false;
  }
  if (!this->read_register16_(meas_lsb_reg, lsb)) {
    return false;
  }

  const uint32_t raw24 = (static_cast<uint32_t>(msb) << 8) | (static_cast<uint32_t>(lsb) >> 8);
  int32_t signed24 = static_cast<int32_t>(raw24);
  if ((signed24 & 0x00800000) != 0) {
    signed24 |= 0xFF000000;
  }

  capacitance_pf = static_cast<float>(signed24) * MEAS_LSB_PF + this->capdac_pf_(measurement_index);
  return true;
}

float FDC1004Component::capdac_pf_(uint8_t measurement_index) const {
  if (measurement_index >= 4) {
    return 0.0f;
  }
  return static_cast<float>(this->capdac_steps_[measurement_index]) * CAPDAC_STEP_PF;
}

void FDC1004ZeroButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->tare_to_current();
  }
}

} // namespace fdc1004
} // namespace esphome
