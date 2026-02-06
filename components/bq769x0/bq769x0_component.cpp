#include "bq769x0_component.h"

#include <algorithm>
#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq769x0 {

namespace {
constexpr const char *TAG = "bq769x0";

constexpr uint8_t SYS_STAT_DEVICE_XREADY = 0x20;
constexpr uint8_t SYS_STAT_UV = 0x08;
constexpr uint8_t SYS_STAT_OV = 0x04;
constexpr uint8_t SYS_STAT_SCD = 0x02;
constexpr uint8_t SYS_STAT_OCD = 0x01;

constexpr uint8_t SYS_CTRL2_CC_EN = 0x40;
constexpr uint8_t SYS_CTRL2_DSG_ON = 0x02;
constexpr uint8_t SYS_CTRL2_CHG_ON = 0x01;

constexpr float CC_LSB_UV = 8.44f;
constexpr float CC_WARN_UV = 200000.0f;
constexpr uint32_t PERSIST_INTERVAL_MS = 60000;

constexpr int DEFAULT_RSENSE_MILLIOHM = 2;
constexpr float SOC_REST_CURRENT_THRESHOLD_MA = 200.0f;
constexpr float SOC_REST_MIN_SECONDS = 30.0f;
constexpr float SOC_REST_FULL_WEIGHT_SECONDS = 300.0f;
constexpr float SOC_REST_DVDT_THRESHOLD_MV_PER_S = 1.0f;
constexpr bool SOC_USE_MIN_CELL = true;

constexpr float SOC_FULL_CELL_MV = 4180.0f;
constexpr float SOC_FULL_HOLD_SECONDS = 10.0f;
constexpr float SOC_EMPTY_CELL_MV = 3300.0f;
constexpr float SOC_EMPTY_HOLD_SECONDS = 2.0f;
constexpr float SOC_EMPTY_DISCHARGE_CURRENT_MA = 1000.0f;

constexpr float SOC_COULOMBIC_EFF_DISCHARGE = 1.0f;
constexpr float SOC_COULOMBIC_EFF_CHARGE = 1.0f;
constexpr float SOC_LEARN_ALPHA = 0.2f;

constexpr float SOC_CHARGE_GATE_DV_MV = 2.0f;

static const OcvPoint LIION_LIPO_OCV_TABLE[] = {
    {3300, 0.0f},
    {3500, 10.0f},
    {3600, 20.0f},
    {3700, 40.0f},
    {3800, 60.0f},
    {3900, 75.0f},
    {4000, 85.0f},
    {4100, 95.0f},
    {4200, 100.0f},
};
} // namespace

BQ769X0Component::BQ769X0Component() : driver_(this) {}

void BQ769X0Component::set_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index >= 1 && index <= cell_sensors_.size()) {
    cell_sensors_[index - 1] = sensor;
  }
}

void BQ769X0Component::setup() {
  rsense_milliohm_ = DEFAULT_RSENSE_MILLIOHM;
  driver_.set_crc_enabled(false);
  driver_.set_i2c_address(this->address_);

  if (driver_.read_calibration(&cal_)) {
    crc_enabled_ = false;
    cal_valid_ = true;
  } else {
    driver_.set_crc_enabled(true);
    if (driver_.read_calibration(&cal_)) {
      crc_enabled_ = true;
      cal_valid_ = true;
    } else {
      ESP_LOGE(TAG, "Failed to read ADC calibration (error=%d)", static_cast<int>(driver_.last_error()));
    }
  }
  if (cal_valid_) {
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

  if (!ensure_cc_enabled_()) {
    ESP_LOGE(TAG, "Failed to enable CC (error=%d)", static_cast<int>(driver_.last_error()));
  }

  soc_cfg_ = SocConfig{};
  soc_cfg_.use_min_cell = SOC_USE_MIN_CELL;
  soc_cfg_.rest_current_threshold_ma = SOC_REST_CURRENT_THRESHOLD_MA;
  soc_cfg_.rest_min_seconds = SOC_REST_MIN_SECONDS;
  soc_cfg_.rest_full_weight_seconds = SOC_REST_FULL_WEIGHT_SECONDS;
  soc_cfg_.rest_dvdt_threshold_mv_per_s = SOC_REST_DVDT_THRESHOLD_MV_PER_S;
  soc_cfg_.full_cell_mv = SOC_FULL_CELL_MV;
  soc_cfg_.full_hold_seconds = SOC_FULL_HOLD_SECONDS;
  soc_cfg_.empty_cell_mv = SOC_EMPTY_CELL_MV;
  soc_cfg_.empty_hold_seconds = SOC_EMPTY_HOLD_SECONDS;
  soc_cfg_.empty_discharge_current_ma = SOC_EMPTY_DISCHARGE_CURRENT_MA;
  soc_cfg_.use_hw_fault_anchors = false;
  soc_cfg_.current_positive_is_discharge = true;
  soc_cfg_.coulombic_eff_discharge = SOC_COULOMBIC_EFF_DISCHARGE;
  soc_cfg_.coulombic_eff_charge = SOC_COULOMBIC_EFF_CHARGE;
  soc_cfg_.learn_alpha = SOC_LEARN_ALPHA;
  soc_cfg_.balance.enabled = false;
  soc_cfg_.ocv_table.clear();
  switch (chemistry_) {
    case Chemistry::LIION_LIPO:
      for (const auto &pt : LIION_LIPO_OCV_TABLE) {
        soc_cfg_.ocv_table.push_back(pt);
      }
      break;
  }

  soc_estimator_.configure(soc_cfg_);
  pref_ = global_preferences->make_preference<PersistedState>(this->get_object_id_hash());
  load_preferences_();

  delay(250);
}

void BQ769X0Component::update() {
  uint32_t now = millis();
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

  std::array<uint8_t, 10> cell_buffer{};
  if (!check_i2c_(read_cell_block_(cell_buffer, cell_count_), "read cell block"))
    return;

  int min_mv = INT32_MAX;
  int max_mv = 0;
  int sum_mv = 0;
  if (cal_valid_) {
    for (size_t i = 0; i < cell_count_; i++) {
      uint8_t hi = cell_buffer[i * 2];
      uint8_t lo = cell_buffer[i * 2 + 1];
      uint16_t adc14 = static_cast<uint16_t>(((hi & 0x3F) << 8) | lo);
      int mv = driver_.cell_mV_from_adc(adc14, cal_);
      sum_mv += mv;
      min_mv = std::min(min_mv, mv);
      max_mv = std::max(max_mv, mv);
      if (cell_sensors_[i] != nullptr) {
        cell_sensors_[i]->publish_state(static_cast<float>(mv) / 1000.0f);
      }
    }
  }

  float avg_mv = cell_count_ > 0 ? static_cast<float>(sum_mv) / cell_count_ : NAN;
  if (min_cell_sensor_ != nullptr && min_mv != INT32_MAX) {
    min_cell_sensor_->publish_state(min_mv);
  }
  if (avg_cell_sensor_ != nullptr && !std::isnan(avg_mv)) {
    avg_cell_sensor_->publish_state(avg_mv);
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

  float dt_s = 0.0f;
  if (last_cc_ms_ != 0) {
    dt_s = (now - last_cc_ms_) / 1000.0f;
  }
  last_cc_ms_ = now;

  int16_t cc = 0;
  if (!check_i2c_(driver_.read_cc16(&cc), "read CC"))
    return;
  float sense_uV = static_cast<float>(cc) * CC_LSB_UV;
  if (std::abs(sense_uV) > CC_WARN_UV) {
    ESP_LOGW(TAG, "CC sense voltage %.1f uV exceeds recommended range", sense_uV);
  }
  float current_ma_signed = sense_uV / static_cast<float>(rsense_milliohm_);

  if (current_sensor_ != nullptr) {
    current_sensor_->publish_state(current_ma_signed);
  }

  int balancing_cells = 0;
  if (soc_cfg_.balance.enabled) {
    uint8_t balance = 0;
    if (driver_.read_register8(Register::CELLBAL1, &balance)) {
      for (int i = 0; i < cell_count_; i++) {
        if (balance & (1 << i)) {
          balancing_cells++;
        }
      }
    }
  }

  SocInputs soc_inputs{};
  float soc_current_ma = std::abs(current_ma_signed);
  if (!std::isnan(last_vavg_mv_) && !std::isnan(avg_mv) && avg_mv > last_vavg_mv_ + SOC_CHARGE_GATE_DV_MV) {
    soc_current_ma = 0.0f;
  }
  last_vavg_mv_ = avg_mv;
  soc_inputs.current_ma = soc_current_ma;
  soc_inputs.vmin_cell_mv = min_mv == INT32_MAX ? NAN : static_cast<float>(min_mv);
  soc_inputs.vavg_cell_mv = avg_mv;
  soc_inputs.dt_s = dt_s;
  soc_inputs.sys_ov = (sys_stat & SYS_STAT_OV) != 0;
  soc_inputs.sys_uv = (sys_stat & SYS_STAT_UV) != 0;
  soc_inputs.is_discharging = soc_current_ma > 0.0f;
  soc_inputs.balancing_cells = balancing_cells;

  update_soc_(soc_inputs);

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

  if (now - last_persist_ms_ >= PERSIST_INTERVAL_MS) {
    save_preferences_();
    last_persist_ms_ = now;
  }
}

void BQ769X0Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ769X0:");
  ESP_LOGCONFIG(TAG, "  Cell count: %u", cell_count_);
  const char *chemistry = chemistry_ == Chemistry::LIION_LIPO ? "liion_lipo" : "unknown";
  ESP_LOGCONFIG(TAG, "  Chemistry: %s", chemistry);
  ESP_LOGCONFIG(TAG, "  CRC enabled (auto): %s", crc_enabled_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Rsense (mOhm): %d", rsense_milliohm_);
  if (cal_valid_) {
    ESP_LOGCONFIG(TAG, "  Gain: %d uV/LSB", cal_.gain_uV_per_lsb);
    ESP_LOGCONFIG(TAG, "  Offset: %d mV", cal_.offset_mV);
  }
}

void BQ769X0Component::update_soc_(const SocInputs &inputs) {
  const auto &outputs = soc_estimator_.update(inputs);
  publish_soc_outputs_(outputs);
}

void BQ769X0Component::publish_soc_outputs_(const SocOutputs &outputs) {
  if (soc_percent_sensor_ != nullptr && !std::isnan(outputs.soc_percent)) {
    soc_percent_sensor_->publish_state(outputs.soc_percent);
  }
}

void BQ769X0Component::load_preferences_() {
  PersistedState state{};
  if (pref_.load(&state)) {
    soc_estimator_.set_capacity_mah(state.capacity_mah);
    soc_estimator_.set_q_remaining_mah(state.q_remaining_mah);
    if (state.soc_valid) {
      soc_estimator_.set_soc_percent(state.soc_percent);
    }
  }
}

void BQ769X0Component::save_preferences_() {
  const auto &outputs = soc_estimator_.outputs();
  PersistedState state{};
  state.soc_percent = outputs.soc_percent;
  state.capacity_mah = outputs.capacity_mah;
  state.q_remaining_mah = outputs.q_remaining_mah;
  state.soc_valid = outputs.soc_valid;
  pref_.save(&state);
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
  if (count == 0 || count > 4) {
    return false;
  }
  size_t len = count * 2;
  return driver_.read_register_block(Register::VC1_HI, buffer.data(), len);
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

} // namespace bq769x0
} // namespace esphome
