#include "bq769x0_component.h"

#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq769x0 {

namespace {
constexpr const char *TAG = "bq769x0";

constexpr uint8_t SYS_STAT_CC_READY = 0x80;
constexpr uint8_t SYS_STAT_DEVICE_XREADY = 0x20;
constexpr uint8_t SYS_STAT_UV = 0x08;
constexpr uint8_t SYS_STAT_OV = 0x04;
constexpr uint8_t SYS_STAT_SCD = 0x02;
constexpr uint8_t SYS_STAT_OCD = 0x01;

constexpr uint8_t SYS_CTRL2_CC_EN = 0x40;
constexpr uint8_t SYS_CTRL2_CC_ONESHOT = 0x20;
constexpr uint8_t SYS_CTRL2_DSG_ON = 0x02;
constexpr uint8_t SYS_CTRL2_CHG_ON = 0x01;
} // namespace

BQ769X0Component::BQ769X0Component() : driver_(this) {}

void BQ769X0Component::set_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index >= 1 && index <= cell_sensors_.size()) {
    cell_sensors_[index - 1] = sensor;
  }
}

void BQ769X0Component::setup() {
  driver_.set_crc_enabled(crc_enabled_);
  driver_.set_i2c_address(this->address_);

  if (!driver_.read_calibration(&cal_)) {
    ESP_LOGE(TAG, "Failed to read ADC calibration (error=%d)", static_cast<int>(driver_.last_error()));
  } else {
    cal_valid_ = true;
    ESP_LOGI(TAG, "ADC calibration: gain=%d uV/LSB, offset=%d mV", cal_.gain_uV_per_lsb, cal_.offset_mV);
  }

  if (!driver_.write_cc_cfg_0x19()) {
    ESP_LOGE(TAG, "Failed to write CC_CFG=0x19 (error=%d)", static_cast<int>(driver_.last_error()));
  }

  if (!driver_.set_adc_enabled(true)) {
    ESP_LOGE(TAG, "Failed to enable ADC (error=%d)", static_cast<int>(driver_.last_error()));
  }

  if (!driver_.set_temp_sel(false)) {
    ESP_LOGE(TAG, "Failed to set TEMP_SEL (error=%d)", static_cast<int>(driver_.last_error()));
  }

  if (current_sensor_ != nullptr || cc_ready_sensor_ != nullptr) {
    ensure_cc_enabled_();
  }

  delay(250);
}

void BQ769X0Component::update() {
  uint8_t sys_stat = 0;
  if (!check_i2c_(driver_.read_sys_stat(&sys_stat), "read SYS_STAT"))
    return;

  if (fault_sensor_ != nullptr) {
    bool fault = (sys_stat & (SYS_STAT_UV | SYS_STAT_OV | SYS_STAT_SCD | SYS_STAT_OCD)) != 0;
    fault_sensor_->publish_state(fault);
  }
  if (device_ready_sensor_ != nullptr) {
    device_ready_sensor_->publish_state((sys_stat & SYS_STAT_DEVICE_XREADY) != 0);
  }
  if (cc_ready_sensor_ != nullptr) {
    cc_ready_sensor_->publish_state((sys_stat & SYS_STAT_CC_READY) != 0);
  }

  bool need_cell_block = false;
  for (auto *sensor : cell_sensors_) {
    if (sensor != nullptr) {
      need_cell_block = true;
      break;
    }
  }
  if (need_cell_block) {
    std::array<uint8_t, 10> buffer{};
    if (!check_i2c_(read_cell_block_(buffer, cell_count_), "read cell block"))
      return;
    if (cal_valid_) {
      publish_cell_block_(buffer, cell_count_);
    }
  }

  if (pack_voltage_sensor_ != nullptr) {
    uint16_t bat16 = 0;
    if (!check_i2c_(driver_.read_bat16(&bat16), "read BAT"))
      return;
    if (cal_valid_) {
      int bat_mV = driver_.bat_mV_from_bat16(bat16, cell_count_, cal_);
      pack_voltage_sensor_->publish_state(static_cast<float>(bat_mV) / 1000.0f);
    }
  }

  if (board_temp_sensor_ != nullptr) {
    uint16_t ts_adc = 0;
    if (!check_i2c_(driver_.read_ts1_adc14(&ts_adc), "read TS1"))
      return;
    board_temp_sensor_->publish_state(driver_.die_temp_C_from_adc(ts_adc));
  }

  if ((current_sensor_ != nullptr || soc_sensor_ != nullptr) && rsense_milliohm_ > 0) {
    int16_t cc = 0;
    if (!check_i2c_(driver_.read_cc16(&cc), "read CC"))
      return;
    float sense_uV = driver_.sense_uV_from_cc(cc);
    float current_ma = sense_uV / static_cast<float>(rsense_milliohm_);
    if (current_sensor_ != nullptr) {
      current_sensor_->publish_state(current_ma);
    }
    update_soc_(current_ma);
  }

  if (mode_sensor_ != nullptr) {
    uint8_t sys_ctrl2 = 0;
    if (!check_i2c_(read_sys_ctrl2_(sys_ctrl2), "read SYS_CTRL2"))
      return;
    const bool device_ready = (sys_stat & SYS_STAT_DEVICE_XREADY) != 0;
    const bool chg_on = (sys_ctrl2 & SYS_CTRL2_CHG_ON) != 0;
    const bool dsg_on = (sys_ctrl2 & SYS_CTRL2_DSG_ON) != 0;
    if (!device_ready) {
      mode_sensor_->publish_state("safe");
    } else if (chg_on && dsg_on) {
      mode_sensor_->publish_state("charge+discharge");
    } else if (chg_on) {
      mode_sensor_->publish_state("charge");
    } else if (dsg_on) {
      mode_sensor_->publish_state("discharge");
    } else {
      mode_sensor_->publish_state("standby");
    }
  }
}

void BQ769X0Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ769X0:");
  ESP_LOGCONFIG(TAG, "  Cell count: %u", cell_count_);
  ESP_LOGCONFIG(TAG, "  CRC enabled: %s", crc_enabled_ ? "true" : "false");
  if (cal_valid_) {
    ESP_LOGCONFIG(TAG, "  Gain: %d uV/LSB", cal_.gain_uV_per_lsb);
    ESP_LOGCONFIG(TAG, "  Offset: %d mV", cal_.offset_mV);
  }
}


void BQ769X0Component::clear_faults() {
  uint8_t sys_stat = 0;
  if (!driver_.read_sys_stat(&sys_stat)) {
    ESP_LOGE(TAG, "Failed to read SYS_STAT (error=%d)", static_cast<int>(driver_.last_error()));
    return;
  }
  uint8_t mask = sys_stat & (SYS_STAT_UV | SYS_STAT_OV | SYS_STAT_SCD | SYS_STAT_OCD);
  if (mask == 0) {
    return;
  }
  if (!driver_.clear_sys_stat_bits(mask)) {
    ESP_LOGE(TAG, "Failed to clear SYS_STAT bits (error=%d)", static_cast<int>(driver_.last_error()));
  }
}

void BQ769X0Component::trigger_cc_oneshot() {
  if (!ensure_cc_enabled_()) {
    return;
  }
  uint8_t sys_ctrl2 = 0;
  if (!read_sys_ctrl2_(sys_ctrl2)) {
    ESP_LOGE(TAG, "Failed to read SYS_CTRL2 (error=%d)", static_cast<int>(driver_.last_error()));
    return;
  }
  sys_ctrl2 |= SYS_CTRL2_CC_ONESHOT;
  if (!write_sys_ctrl2_(sys_ctrl2)) {
    ESP_LOGE(TAG, "Failed to write SYS_CTRL2 (error=%d)", static_cast<int>(driver_.last_error()));
    return;
  }
  for (int attempt = 0; attempt < 50; attempt++) {
    uint8_t stat = 0;
    if (!driver_.read_sys_stat(&stat)) {
      ESP_LOGE(TAG, "Failed to read SYS_STAT after oneshot (error=%d)", static_cast<int>(driver_.last_error()));
      return;
    }
    if (stat & SYS_STAT_CC_READY) {
      return;
    }
    delay(10);
  }
  ESP_LOGW(TAG, "CC_READY not asserted after oneshot request");
}

void BQ769X0Component::update_soc_(float current_ma) {
  if (soc_sensor_ == nullptr || capacity_mah_ <= 0.0f) {
    return;
  }
  const uint32_t now = millis();
  if (std::isnan(soc_percent_)) {
    soc_percent_ = initial_soc_;
    last_soc_ms_ = now;
    soc_sensor_->publish_state(soc_percent_);
    return;
  }
  if (last_soc_ms_ == 0) {
    last_soc_ms_ = now;
    return;
  }
  const float dt_hours = (now - last_soc_ms_) / 3600000.0f;
  if (dt_hours <= 0.0f) {
    last_soc_ms_ = now;
    return;
  }
  float soc_mah = (soc_percent_ / 100.0f) * capacity_mah_;
  soc_mah -= current_ma * dt_hours;
  if (soc_mah < 0.0f) {
    soc_mah = 0.0f;
  } else if (soc_mah > capacity_mah_) {
    soc_mah = capacity_mah_;
  }
  soc_percent_ = (soc_mah / capacity_mah_) * 100.0f;
  last_soc_ms_ = now;
  soc_sensor_->publish_state(soc_percent_);
}

bool BQ769X0Component::ensure_cc_enabled_() {
  uint8_t sys_ctrl2 = 0;
  if (!read_sys_ctrl2_(sys_ctrl2)) {
    ESP_LOGE(TAG, "Failed to read SYS_CTRL2 (error=%d)", static_cast<int>(driver_.last_error()));
    return false;
  }
  if ((sys_ctrl2 & SYS_CTRL2_CC_EN) != 0) {
    return true;
  }
  sys_ctrl2 |= SYS_CTRL2_CC_EN;
  if (!write_sys_ctrl2_(sys_ctrl2)) {
    ESP_LOGE(TAG, "Failed to enable CC (error=%d)", static_cast<int>(driver_.last_error()));
    return false;
  }
  return true;
}

bool BQ769X0Component::read_sys_ctrl2_(uint8_t &value) { return driver_.read_register8(Register::SYS_CTRL2, &value); }

bool BQ769X0Component::write_sys_ctrl2_(uint8_t value) { return driver_.write_register8(Register::SYS_CTRL2, value); }

bool BQ769X0Component::read_cell_block_(std::array<uint8_t, 10> &buffer, size_t count) {
  if (count == 0 || count > 5) {
    return false;
  }
  size_t len = count * 2;
  return driver_.read_register_block(Register::VC1_HI, buffer.data(), len);
}

void BQ769X0Component::publish_cell_block_(const std::array<uint8_t, 10> &buffer, size_t count) {
  for (size_t i = 0; i < count; i++) {
    auto *sensor = cell_sensors_[i];
    if (sensor == nullptr) {
      continue;
    }
    uint8_t hi = buffer[i * 2];
    uint8_t lo = buffer[i * 2 + 1];
    uint16_t adc14 = static_cast<uint16_t>(((hi & 0x3F) << 8) | lo);
    int mv = driver_.cell_mV_from_adc(adc14, cal_);
    sensor->publish_state(static_cast<float>(mv) / 1000.0f);
  }
}

bool BQ769X0Component::check_i2c_(bool ok, const char *operation) {
  if (ok) {
    return true;
  }
  ESP_LOGE(TAG, "%s failed (error=%d)", operation, static_cast<int>(driver_.last_error()));
  return false;
}

void BQ769X0ClearFaultsButton::press_action() {
  if (parent_ != nullptr) {
    parent_->clear_faults();
  }
}

void BQ769X0CCOneshotButton::press_action() {
  if (parent_ != nullptr) {
    parent_->trigger_cc_oneshot();
  }
}

} // namespace bq769x0
} // namespace esphome
