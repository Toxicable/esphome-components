#include "bq25756.h"

#include <array>
#include <cstdio>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq25756 {

namespace {
static const char *const TAG = "bq25756";

constexpr uint8_t REG15_TIMER_CONTROL = 0x15;
constexpr uint8_t REG17_CHARGER_CONTROL = 0x17;
constexpr uint8_t REG18_PIN_CONTROL = 0x18;
constexpr uint8_t REG19_POWER_PATH_CONTROL = 0x19;
constexpr uint8_t REG21_CHARGER_STATUS_1 = 0x21;
constexpr uint8_t REG22_CHARGER_STATUS_2 = 0x22;
constexpr uint8_t REG23_CHARGER_STATUS_3 = 0x23;
constexpr uint8_t REG24_FAULT_STATUS = 0x24;
constexpr uint8_t REG2B_ADC_CONTROL = 0x2B;
constexpr uint8_t REG2C_ADC_CHANNEL_CONTROL = 0x2C;
constexpr uint8_t REG2D_IAC_ADC = 0x2D;
constexpr uint8_t REG2F_IBAT_ADC = 0x2F;
constexpr uint8_t REG31_VAC_ADC = 0x31;
constexpr uint8_t REG33_VBAT_ADC = 0x33;
constexpr uint8_t REG37_TS_ADC = 0x37;
constexpr uint8_t REG39_VFB_ADC = 0x39;
constexpr uint8_t REG3D_PART_INFORMATION = 0x3D;

constexpr uint8_t REG15_WATCHDOG_MASK = 0x30;
constexpr uint8_t REG17_WD_RST_MASK = 0x20;
constexpr uint8_t REG17_DIS_CE_PIN_MASK = 0x10;
constexpr uint8_t REG17_EN_HIZ_MASK = 0x04;
constexpr uint8_t REG17_EN_CHG_MASK = 0x01;
constexpr uint8_t REG18_EN_ICHG_PIN_MASK = 0x80;
constexpr uint8_t REG18_EN_ILIM_HIZ_PIN_MASK = 0x40;
constexpr uint8_t REG19_EN_REV_MASK = 0x01;

constexpr uint8_t REG2B_ADC_EN_MASK = 0x80;
constexpr uint8_t REG2B_ADC_RATE_MASK = 0x40;
constexpr uint8_t REG2C_IAC_ADC_DIS_MASK = 0x80;
constexpr uint8_t REG2C_IBAT_ADC_DIS_MASK = 0x40;
constexpr uint8_t REG2C_VAC_ADC_DIS_MASK = 0x20;
constexpr uint8_t REG2C_VBAT_ADC_DIS_MASK = 0x10;
constexpr uint8_t REG2C_TS_ADC_DIS_MASK = 0x04;
constexpr uint8_t REG2C_VFB_ADC_DIS_MASK = 0x02;

constexpr uint8_t PART_NUM_MASK = 0x78;
constexpr uint8_t BQ25756_PART_NUM_BITS = 0x10;

constexpr float IAC_CURRENT_LSB_MA = 0.8f;
constexpr float IBAT_CURRENT_LSB_MA = 2.0f;
constexpr float VOLTAGE_LSB_MV = 2.0f;
constexpr float TS_PERCENT_LSB = 0.09765625f;

constexpr uint16_t REG00_VFB_REG_MASK = 0x001F;
constexpr uint16_t REG02_ICHG_REG_MASK = 0x07FC;
constexpr uint16_t REG06_IAC_DPM_MASK = 0x07FC;
constexpr uint16_t REG08_VAC_DPM_MASK = 0x3FFC;
constexpr float VBAT_OV_RISING_MULTIPLIER = 1.04f;
constexpr float VBAT_OV_FALLING_MULTIPLIER = 1.02f;
}  // namespace

bool BQ25756Component::set_charge_enabled(bool enabled) {
  this->log_charge_enable_precheck_(enabled);
  return this->update_register_bits_(
    REG17_CHARGER_CONTROL, REG17_EN_CHG_MASK, enabled ? REG17_EN_CHG_MASK : 0x00
  );
}

bool BQ25756Component::set_hiz_mode(bool enabled) {
  return this->update_register_bits_(
    REG17_CHARGER_CONTROL, REG17_EN_HIZ_MASK, enabled ? REG17_EN_HIZ_MASK : 0x00
  );
}

bool BQ25756Component::set_reverse_mode(bool enabled) {
  return this->update_register_bits_(
    REG19_POWER_PATH_CONTROL, REG19_EN_REV_MASK, enabled ? REG19_EN_REV_MASK : 0x00
  );
}

bool BQ25756Component::set_watchdog_code(uint8_t code) {
  if (code > 0x03) {
    return false;
  }
  return this->update_register_bits_(
    REG15_TIMER_CONTROL, REG15_WATCHDOG_MASK, static_cast<uint8_t>(code << 4)
  );
}

bool BQ25756Component::reset_watchdog() {
  return this->update_register_bits_(REG17_CHARGER_CONTROL, REG17_WD_RST_MASK, REG17_WD_RST_MASK);
}

void BQ25756Component::setup() {
  this->initialized_ = false;
  this->next_init_retry_ms_ = 0;

  if (!this->initialize_()) {
    ESP_LOGW(TAG, "BQ25756 init not ready at startup; will retry");
    this->status_set_warning();
    this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
    return;
  }

  this->status_clear_warning();
}

bool BQ25756Component::initialize_() {
  uint8_t part_info = 0;
  if (!this->read_byte_(REG3D_PART_INFORMATION, part_info)) {
    ESP_LOGW(TAG, "Failed to read REG3D (part information); device not responding yet");
    return false;
  }

  if ((part_info & PART_NUM_MASK) != BQ25756_PART_NUM_BITS) {
    ESP_LOGW(TAG, "Unexpected REG3D=0x%02X; PART_NUM does not match BQ25756", part_info);
    return false;
  }

  ESP_LOGI(TAG, "BQ25756 detected, REG3D=0x%02X", part_info);

  if (this->disable_watchdog_) {
    uint8_t reg15 = 0;
    if (!this->read_byte_(REG15_TIMER_CONTROL, reg15)) {
      ESP_LOGW(TAG, "Failed to read REG15 (timer control)");
      return false;
    }
    if ((reg15 & REG15_WATCHDOG_MASK) != 0) {
      if (!this->set_watchdog_code(0)) {
        ESP_LOGW(TAG, "Failed to disable watchdog via REG15");
        return false;
      }
    }
  }

  if (!this->ensure_adc_enabled_()) {
    ESP_LOGW(TAG, "ADC configuration failed; readings may be stale or zero");
    return false;
  }
  if (!this->apply_configured_limits_()) {
    ESP_LOGW(TAG, "Failed to apply configured charge/input limits");
    return false;
  }
  if (!this->apply_configured_pin_overrides_()) {
    ESP_LOGW(TAG, "Failed to apply configured pin control overrides");
    return false;
  }

  this->publish_control_states_();
  this->initialized_ = true;
  return true;
}

void BQ25756Component::update() {
  if (!this->initialized_) {
    const uint32_t now = millis();
    if (now < this->next_init_retry_ms_) {
      return;
    }
    if (!this->initialize_()) {
      this->status_set_warning();
      this->next_init_retry_ms_ = now + INIT_RETRY_INTERVAL_MS;
      return;
    }
    this->status_clear_warning();
  }

  uint8_t status1 = 0;
  uint8_t status2 = 0;
  uint8_t status3 = 0;
  uint8_t fault = 0;
  if (!this->read_byte_(REG21_CHARGER_STATUS_1, status1) || !this->read_byte_(REG22_CHARGER_STATUS_2, status2) ||
      !this->read_byte_(REG23_CHARGER_STATUS_3, status3) || !this->read_byte_(REG24_FAULT_STATUS, fault)) {
    ESP_LOGW(TAG, "Failed reading charger status registers");
    this->status_set_warning();
    return;
  }

  Reg16Value iac{};
  Reg16Value ibat{};
  Reg16Value vac{};
  Reg16Value vbat{};
  Reg16Value ts{};
  Reg16Value vfb{};
  const bool need_vfb = this->vfb_voltage_sensor_ != nullptr;

  if (!this->read_u16_le_(REG2D_IAC_ADC, iac) || !this->read_u16_le_(REG2F_IBAT_ADC, ibat) ||
      !this->read_u16_le_(REG31_VAC_ADC, vac) || !this->read_u16_le_(REG33_VBAT_ADC, vbat) ||
      !this->read_u16_le_(REG37_TS_ADC, ts) || (need_vfb && !this->read_u16_le_(REG39_VFB_ADC, vfb))) {
    ESP_LOGW(TAG, "Failed reading one or more ADC registers");
    this->status_set_warning();
    return;
  }

  const float iac_ma = static_cast<float>(static_cast<int16_t>(iac.raw_le)) * IAC_CURRENT_LSB_MA;
  const float ibat_ma = static_cast<float>(static_cast<int16_t>(ibat.raw_le)) * IBAT_CURRENT_LSB_MA;
  const float vac_mv = static_cast<float>(vac.raw_le) * VOLTAGE_LSB_MV;
  const float vbat_mv = static_cast<float>(vbat.raw_le) * VOLTAGE_LSB_MV;
  const float ts_percent = static_cast<float>(ts.raw_le) * TS_PERCENT_LSB;
  const float vfb_mv = static_cast<float>(vfb.raw_le);

  this->maybe_log_event_(status1, status2, status3, fault, iac_ma, ibat_ma, vac_mv, vbat_mv);

  ESP_LOGD(TAG, "STATUS[21..24]=%02X %02X %02X %02X", status1, status2, status3, fault);
  ESP_LOGD(
    TAG,
    "IAC=%.1f mA (0x%04X [%02X %02X]), IBAT=%.0f mA (0x%04X [%02X %02X]), "
    "VAC=%.0f mV (0x%04X [%02X %02X]), VBAT=%.0f mV (0x%04X [%02X %02X]), "
    "TS=%.3f%% (0x%04X [%02X %02X])%s",
    iac_ma,
    iac.raw_le,
    iac.lsb,
    iac.msb,
    ibat_ma,
    ibat.raw_le,
    ibat.lsb,
    ibat.msb,
    vac_mv,
    vac.raw_le,
    vac.lsb,
    vac.msb,
    vbat_mv,
    vbat.raw_le,
    vbat.lsb,
    vbat.msb,
    ts_percent,
    ts.raw_le,
    ts.lsb,
    ts.msb,
    need_vfb ? "" : ", VFB=disabled"
  );
  if (need_vfb) {
    ESP_LOGD(TAG, "VFB=%.0f mV (0x%04X [%02X %02X])", vfb_mv, vfb.raw_le, vfb.lsb, vfb.msb);
  }

  if (this->iac_current_sensor_ != nullptr) {
    this->iac_current_sensor_->publish_state(iac_ma);
  }
  if (this->ibat_current_sensor_ != nullptr) {
    this->ibat_current_sensor_->publish_state(ibat_ma);
  }
  if (this->vac_voltage_sensor_ != nullptr) {
    this->vac_voltage_sensor_->publish_state(vac_mv);
  }
  if (this->vbat_voltage_sensor_ != nullptr) {
    this->vbat_voltage_sensor_->publish_state(vbat_mv);
  }
  if (this->ts_percent_sensor_ != nullptr) {
    this->ts_percent_sensor_->publish_state(ts_percent);
  }
  if (this->vfb_voltage_sensor_ != nullptr) {
    this->vfb_voltage_sensor_->publish_state(vfb_mv);
  }

  if (this->vfb_reg_target_sensor_ != nullptr || this->vbat_ov_rising_fb_sensor_ != nullptr ||
      this->vbat_ov_falling_fb_sensor_ != nullptr || this->vbat_ov_rising_pack_sensor_ != nullptr ||
      this->vbat_ov_falling_pack_sensor_ != nullptr) {
    Reg16Value vfb_reg{};
    if (!this->read_u16_le_(0x00, vfb_reg)) {
      ESP_LOGW(TAG, "Failed reading REG0x00 for VFB/VBAT_OV threshold diagnostics");
    } else {
      const float vfb_reg_mv = 1504.0f + static_cast<float>(vfb_reg.raw_le & REG00_VFB_REG_MASK) * 2.0f;
      const float vbat_ov_rising_fb_mv = vfb_reg_mv * VBAT_OV_RISING_MULTIPLIER;
      const float vbat_ov_falling_fb_mv = vfb_reg_mv * VBAT_OV_FALLING_MULTIPLIER;
      if (this->vfb_reg_target_sensor_ != nullptr) {
        this->vfb_reg_target_sensor_->publish_state(vfb_reg_mv);
      }
      if (this->vbat_ov_rising_fb_sensor_ != nullptr) {
        this->vbat_ov_rising_fb_sensor_->publish_state(vbat_ov_rising_fb_mv);
      }
      if (this->vbat_ov_falling_fb_sensor_ != nullptr) {
        this->vbat_ov_falling_fb_sensor_->publish_state(vbat_ov_falling_fb_mv);
      }
      if (this->has_fb_to_pack_voltage_scale_) {
        const float rising_pack_mv = vbat_ov_rising_fb_mv * this->fb_to_pack_voltage_scale_;
        const float falling_pack_mv = vbat_ov_falling_fb_mv * this->fb_to_pack_voltage_scale_;
        if (this->vbat_ov_rising_pack_sensor_ != nullptr) {
          this->vbat_ov_rising_pack_sensor_->publish_state(rising_pack_mv);
        }
        if (this->vbat_ov_falling_pack_sensor_ != nullptr) {
          this->vbat_ov_falling_pack_sensor_->publish_state(falling_pack_mv);
        }
      }
    }
  }

  this->publish_status_texts_(status1, status2, status3, fault);
  this->publish_control_states_();
  this->status_clear_warning();
}

void BQ25756Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ25756:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  disable_watchdog: %s", this->disable_watchdog_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  event_logging: %s", this->event_logging_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  disable_ce_pin: %s", this->disable_ce_pin_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  disable_ilim_hiz_pin: %s", this->disable_ilim_hiz_pin_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  disable_ichg_pin: %s", this->disable_ichg_pin_ ? "true" : "false");
  if (this->has_charge_voltage_limit_mv_) {
    ESP_LOGCONFIG(TAG, "  charge_voltage_limit_mv: %u", this->charge_voltage_limit_mv_);
  }
  if (this->has_charge_current_limit_ma_) {
    ESP_LOGCONFIG(TAG, "  charge_current_limit_ma: %u", this->charge_current_limit_ma_);
  }
  if (this->has_input_current_dpm_limit_ma_) {
    ESP_LOGCONFIG(TAG, "  input_current_dpm_limit_ma: %u", this->input_current_dpm_limit_ma_);
  }
  if (this->has_input_voltage_dpm_limit_mv_) {
    ESP_LOGCONFIG(TAG, "  input_voltage_dpm_limit_mv: %u", this->input_voltage_dpm_limit_mv_);
  }
  if (this->has_fb_to_pack_voltage_scale_) {
    ESP_LOGCONFIG(TAG, "  fb_to_pack_voltage_scale: %.6f", this->fb_to_pack_voltage_scale_);
  }
  LOG_SENSOR("  ", "IAC Current", this->iac_current_sensor_);
  LOG_SENSOR("  ", "IBAT Current", this->ibat_current_sensor_);
  LOG_SENSOR("  ", "VAC Voltage", this->vac_voltage_sensor_);
  LOG_SENSOR("  ", "VBAT Voltage", this->vbat_voltage_sensor_);
  LOG_SENSOR("  ", "TS Percent", this->ts_percent_sensor_);
  LOG_SENSOR("  ", "VFB Voltage", this->vfb_voltage_sensor_);
  LOG_SENSOR("  ", "VFB REG Target", this->vfb_reg_target_sensor_);
  LOG_SENSOR("  ", "VBAT OV Rising (FB)", this->vbat_ov_rising_fb_sensor_);
  LOG_SENSOR("  ", "VBAT OV Falling (FB)", this->vbat_ov_falling_fb_sensor_);
  LOG_SENSOR("  ", "VBAT OV Rising (Pack)", this->vbat_ov_rising_pack_sensor_);
  LOG_SENSOR("  ", "VBAT OV Falling (Pack)", this->vbat_ov_falling_pack_sensor_);
  LOG_TEXT_SENSOR("  ", "Charge Status", this->charge_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "TS Status", this->ts_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "MPPT Status", this->mppt_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Status Flags", this->status_flags_text_sensor_);
  LOG_SWITCH("  ", "Charge Enable", this->charge_enable_switch_);
  LOG_SWITCH("  ", "HIZ Mode", this->hiz_mode_switch_);
  LOG_SWITCH("  ", "Reverse Mode", this->reverse_mode_switch_);
  LOG_SELECT("  ", "Watchdog", this->watchdog_select_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
}

bool BQ25756Component::dump_registers_0x00_0x3D() {
  std::array<uint8_t, 0x3E> regs{};
  if (!this->read_bytes_(0x00, regs.data(), regs.size())) {
    ESP_LOGW(TAG, "Failed register dump read for 0x00..0x3D");
    return false;
  }

  ESP_LOGD(TAG, "Register dump 0x00..0x3D:");
  for (size_t offset = 0; offset < regs.size(); offset += 16) {
    char line[80];
    int n = std::snprintf(line, sizeof(line), "0x%02X:", static_cast<unsigned>(offset));
    for (size_t i = 0; i < 16 && (offset + i) < regs.size(); i++) {
      if (n <= 0 || static_cast<size_t>(n) >= sizeof(line)) {
        break;
      }
      const int written =
        std::snprintf(line + n, sizeof(line) - static_cast<size_t>(n), " %02X", regs[offset + i]);
      if (written <= 0) {
        break;
      }
      n += written;
    }
    ESP_LOGD(TAG, "%s", line);
  }
  return true;
}

bool BQ25756Component::read_byte_(uint8_t reg, uint8_t &value) {
  return this->read_bytes_(reg, &value, 1);
}

bool BQ25756Component::read_bytes_(uint8_t reg, uint8_t *data, size_t len) {
  if (len == 0) {
    return true;
  }
  uint8_t reg_addr = reg;
  if (this->write_read(&reg_addr, 1, data, len) == i2c::ERROR_OK) {
    return true;
  }

  if (this->write(&reg_addr, 1) != i2c::ERROR_OK) {
    return false;
  }
  return this->read(data, len) == i2c::ERROR_OK;
}

bool BQ25756Component::write_byte_(uint8_t reg, uint8_t value) {
  return this->write_bytes(reg, &value, 1);
}

bool BQ25756Component::write_u16_le_(uint8_t reg, uint16_t value) {
  const uint8_t raw[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  return this->write_bytes(reg, raw, sizeof(raw));
}

bool BQ25756Component::read_u16_le_(uint8_t reg, Reg16Value &value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_bytes_(reg, raw, sizeof(raw))) {
    return false;
  }
  value.lsb = raw[0];
  value.msb = raw[1];
  value.raw_le = static_cast<uint16_t>(raw[0]) | (static_cast<uint16_t>(raw[1]) << 8);
  return true;
}

bool BQ25756Component::update_register_bits_(uint8_t reg, uint8_t mask, uint8_t value_bits) {
  uint8_t current = 0;
  if (!this->read_byte_(reg, current)) {
    return false;
  }
  const uint8_t updated = static_cast<uint8_t>((current & ~mask) | (value_bits & mask));
  if (updated == current) {
    return true;
  }
  return this->write_byte_(reg, updated);
}

bool BQ25756Component::read_control_states_(
  bool &charge_enabled, bool &hiz_mode, bool &reverse_mode, uint8_t &watchdog_code
) {
  uint8_t reg15 = 0;
  uint8_t reg17 = 0;
  uint8_t reg19 = 0;
  if (!this->read_byte_(REG15_TIMER_CONTROL, reg15) || !this->read_byte_(REG17_CHARGER_CONTROL, reg17) ||
      !this->read_byte_(REG19_POWER_PATH_CONTROL, reg19)) {
    return false;
  }

  charge_enabled = (reg17 & REG17_EN_CHG_MASK) != 0;
  hiz_mode = (reg17 & REG17_EN_HIZ_MASK) != 0;
  reverse_mode = (reg19 & REG19_EN_REV_MASK) != 0;
  watchdog_code = static_cast<uint8_t>((reg15 & REG15_WATCHDOG_MASK) >> 4);
  return true;
}

void BQ25756Component::publish_status_texts_(uint8_t status1, uint8_t status2, uint8_t status3, uint8_t fault) {
  const bool pg_good = (status2 & 0x80) != 0;
  const bool watchdog_expired = (status1 & 0x08) != 0;
  const bool iac_dpm_active = (status1 & 0x40) != 0;
  const bool vac_dpm_active = (status1 & 0x20) != 0;
  const bool reverse_active = (status3 & 0x04) != 0;
  const bool cv_timer_expired = (status3 & 0x08) != 0;
  const bool charge_timer_expired = (fault & 0x04) != 0;
  const bool vac_uv_fault = (fault & 0x80) != 0;
  const bool vac_ov_fault = (fault & 0x40) != 0;
  const bool ibat_ocp_fault = (fault & 0x20) != 0;
  const bool vbat_ov_fault = (fault & 0x10) != 0;
  const bool thermal_shutdown = (fault & 0x08) != 0;
  const bool drv_sup_fault = (fault & 0x02) != 0;

  if (this->charge_status_text_sensor_ != nullptr) {
    this->charge_status_text_sensor_->publish_state(this->charge_status_to_string_(status1 & 0x07));
  }
  if (this->ts_status_text_sensor_ != nullptr) {
    this->ts_status_text_sensor_->publish_state(this->ts_status_to_string_((status2 >> 4) & 0x07));
  }
  if (this->mppt_status_text_sensor_ != nullptr) {
    this->mppt_status_text_sensor_->publish_state(this->mppt_status_to_string_(status2 & 0x03));
  }

  if (this->status_flags_text_sensor_ != nullptr) {
    char flags[192];
    size_t n = 0;
    auto append_flag = [&](const char *name, bool active) {
      if (!active || n >= sizeof(flags)) {
        return;
      }
      if (n > 0) {
        int written = std::snprintf(flags + n, sizeof(flags) - n, ",");
        if (written <= 0 || static_cast<size_t>(written) >= (sizeof(flags) - n)) {
          n = sizeof(flags) - 1;
          return;
        }
        n += static_cast<size_t>(written);
      }
      int written = std::snprintf(flags + n, sizeof(flags) - n, "%s", name);
      if (written <= 0 || static_cast<size_t>(written) >= (sizeof(flags) - n)) {
        n = sizeof(flags) - 1;
        return;
      }
      n += static_cast<size_t>(written);
    };

    append_flag("wd_expired", watchdog_expired);
    append_flag("iac_dpm", iac_dpm_active);
    append_flag("vac_dpm", vac_dpm_active);
    append_flag("reverse", reverse_active);
    append_flag("cv_timer", cv_timer_expired);
    append_flag("charge_timer", charge_timer_expired);
    append_flag("vac_uv", vac_uv_fault);
    append_flag("vac_ov", vac_ov_fault);
    append_flag("ibat_ocp", ibat_ocp_fault);
    append_flag("vbat_ov", vbat_ov_fault);
    append_flag("tshut", thermal_shutdown);
    append_flag("drv_sup", drv_sup_fault);
    if (!pg_good) {
      append_flag("pg_low", true);
    }

    if (n == 0) {
      this->status_flags_text_sensor_->publish_state("none");
    } else {
      this->status_flags_text_sensor_->publish_state(flags);
    }
  }
}

void BQ25756Component::publish_control_states_() {
  if (this->charge_enable_switch_ == nullptr && this->hiz_mode_switch_ == nullptr &&
      this->reverse_mode_switch_ == nullptr && this->watchdog_select_ == nullptr) {
    return;
  }

  bool charge_enabled = false;
  bool hiz_mode = false;
  bool reverse_mode = false;
  uint8_t watchdog_code = 0;
  if (!this->read_control_states_(charge_enabled, hiz_mode, reverse_mode, watchdog_code)) {
    ESP_LOGW(TAG, "Failed to refresh control states");
    return;
  }

  if (this->charge_enable_switch_ != nullptr) {
    this->charge_enable_switch_->publish_state(charge_enabled);
  }
  if (this->hiz_mode_switch_ != nullptr) {
    this->hiz_mode_switch_->publish_state(hiz_mode);
  }
  if (this->reverse_mode_switch_ != nullptr) {
    this->reverse_mode_switch_->publish_state(reverse_mode);
  }
  if (this->watchdog_select_ != nullptr) {
    this->watchdog_select_->publish_state(static_cast<size_t>(watchdog_code));
  }
}

const char *BQ25756Component::charge_status_to_string_(uint8_t charge_status) const {
  switch (charge_status) {
    case 0:
      return "not_charging";
    case 1:
      return "trickle";
    case 2:
      return "precharge";
    case 3:
      return "fast_cc";
    case 4:
      return "taper_cv";
    case 5:
      return "reserved_5";
    case 6:
      return "topoff";
    case 7:
      return "termination_done";
    default:
      return "unknown";
  }
}

const char *BQ25756Component::ts_status_to_string_(uint8_t ts_status) const {
  switch (ts_status) {
    case 0:
      return "normal";
    case 1:
      return "warm";
    case 2:
      return "cool";
    case 3:
      return "cold";
    case 4:
      return "hot";
    default:
      return "reserved";
  }
}

const char *BQ25756Component::mppt_status_to_string_(uint8_t mppt_status) const {
  switch (mppt_status) {
    case 0:
      return "disabled";
    case 1:
      return "enabled_idle";
    case 2:
      return "sweeping";
    case 3:
      return "max_power_detected";
    default:
      return "unknown";
  }
}

bool BQ25756Component::ensure_adc_enabled_() {
  uint8_t reg2b = 0;
  if (!this->read_byte_(REG2B_ADC_CONTROL, reg2b)) {
    ESP_LOGW(TAG, "Failed to read REG2B (ADC control)");
    return false;
  }

  const uint8_t reg2b_new =
    static_cast<uint8_t>((reg2b | REG2B_ADC_EN_MASK) & ~REG2B_ADC_RATE_MASK);
  if (reg2b_new != reg2b && !this->write_byte_(REG2B_ADC_CONTROL, reg2b_new)) {
    ESP_LOGW(TAG, "Failed to write REG2B (0x%02X -> 0x%02X)", reg2b, reg2b_new);
    return false;
  }

  uint8_t reg2c = 0;
  if (!this->read_byte_(REG2C_ADC_CHANNEL_CONTROL, reg2c)) {
    ESP_LOGW(TAG, "Failed to read REG2C (ADC channel control)");
    return false;
  }

  uint8_t reg2c_new = static_cast<uint8_t>(
    reg2c &
    static_cast<uint8_t>(
      ~(REG2C_IAC_ADC_DIS_MASK | REG2C_IBAT_ADC_DIS_MASK | REG2C_VAC_ADC_DIS_MASK | REG2C_VBAT_ADC_DIS_MASK |
        REG2C_TS_ADC_DIS_MASK)
    )
  );
  if (this->vfb_voltage_sensor_ != nullptr) {
    reg2c_new = static_cast<uint8_t>(reg2c_new & ~REG2C_VFB_ADC_DIS_MASK);
  } else {
    reg2c_new = static_cast<uint8_t>(reg2c_new | REG2C_VFB_ADC_DIS_MASK);
  }

  if (reg2c_new != reg2c && !this->write_byte_(REG2C_ADC_CHANNEL_CONTROL, reg2c_new)) {
    ESP_LOGW(TAG, "Failed to write REG2C (0x%02X -> 0x%02X)", reg2c, reg2c_new);
    return false;
  }

  ESP_LOGD(TAG, "ADC config REG2B: 0x%02X -> 0x%02X, REG2C: 0x%02X -> 0x%02X", reg2b, reg2b_new, reg2c, reg2c_new);
  return true;
}

void BQ25756Component::maybe_log_event_(
  uint8_t status1, uint8_t status2, uint8_t status3, uint8_t fault, float iac_ma, float ibat_ma, float vac_mv, float vbat_mv
) {
  if (!this->event_logging_) {
    return;
  }

  const bool changed = !this->has_last_event_status_ || status1 != this->last_status1_ || status2 != this->last_status2_ ||
                       status3 != this->last_status3_ || fault != this->last_fault_;
  if (!changed) {
    return;
  }

  const char *const charge = this->charge_status_to_string_(status1 & 0x07);
  const char *const ts = this->ts_status_to_string_((status2 >> 4) & 0x07);
  const char *const mppt = this->mppt_status_to_string_(status2 & 0x03);
  const bool pg_good = (status2 & 0x80) != 0;

  ESP_LOGI(
    TAG,
    "Event: status=%02X/%02X/%02X fault=%02X charge=%s ts=%s mppt=%s pg=%d iac=%.1fmA ibat=%.0fmA vac=%.0fmV vbat=%.0fmV",
    status1,
    status2,
    status3,
    fault,
    charge,
    ts,
    mppt,
    pg_good ? 1 : 0,
    iac_ma,
    ibat_ma,
    vac_mv,
    vbat_mv
  );

  this->has_last_event_status_ = true;
  this->last_status1_ = status1;
  this->last_status2_ = status2;
  this->last_status3_ = status3;
  this->last_fault_ = fault;
}

bool BQ25756Component::apply_configured_limits_() {
  if (this->has_charge_voltage_limit_mv_) {
    const uint16_t code = static_cast<uint16_t>((this->charge_voltage_limit_mv_ - 1504) / 2);
    const uint16_t raw = static_cast<uint16_t>(code & REG00_VFB_REG_MASK);
    if (!this->write_u16_le_(0x00, raw)) {
      ESP_LOGW(TAG, "Failed writing REG0x00 (charge_voltage_limit_mv)");
      return false;
    }
  }

  if (this->has_charge_current_limit_ma_) {
    const uint16_t code = static_cast<uint16_t>(this->charge_current_limit_ma_ / 50);
    const uint16_t raw = static_cast<uint16_t>((code << 2) & REG02_ICHG_REG_MASK);
    if (!this->write_u16_le_(0x02, raw)) {
      ESP_LOGW(TAG, "Failed writing REG0x02 (charge_current_limit_ma)");
      return false;
    }
  }

  if (this->has_input_current_dpm_limit_ma_) {
    const uint16_t code = static_cast<uint16_t>(this->input_current_dpm_limit_ma_ / 50);
    const uint16_t raw = static_cast<uint16_t>((code << 2) & REG06_IAC_DPM_MASK);
    if (!this->write_u16_le_(0x06, raw)) {
      ESP_LOGW(TAG, "Failed writing REG0x06 (input_current_dpm_limit_ma)");
      return false;
    }
  }

  if (this->has_input_voltage_dpm_limit_mv_) {
    const uint16_t code = static_cast<uint16_t>(this->input_voltage_dpm_limit_mv_ / 20);
    const uint16_t raw = static_cast<uint16_t>((code << 2) & REG08_VAC_DPM_MASK);
    if (!this->write_u16_le_(0x08, raw)) {
      ESP_LOGW(TAG, "Failed writing REG0x08 (input_voltage_dpm_limit_mv)");
      return false;
    }
  }

  return true;
}

bool BQ25756Component::apply_configured_pin_overrides_() {
  if (this->disable_ce_pin_) {
    if (!this->update_register_bits_(REG17_CHARGER_CONTROL, REG17_DIS_CE_PIN_MASK, REG17_DIS_CE_PIN_MASK)) {
      ESP_LOGW(TAG, "Failed setting REG17.DIS_CE_PIN");
      return false;
    }
  }

  if (this->disable_ilim_hiz_pin_) {
    if (!this->update_register_bits_(REG18_PIN_CONTROL, REG18_EN_ILIM_HIZ_PIN_MASK, 0x00)) {
      ESP_LOGW(TAG, "Failed clearing REG18.EN_ILIM_HIZ_PIN");
      return false;
    }
  }

  if (this->disable_ichg_pin_) {
    if (!this->update_register_bits_(REG18_PIN_CONTROL, REG18_EN_ICHG_PIN_MASK, 0x00)) {
      ESP_LOGW(TAG, "Failed clearing REG18.EN_ICHG_PIN");
      return false;
    }
  }
  return true;
}

void BQ25756Component::log_charge_enable_precheck_(bool requested_on) {
  uint8_t reg17 = 0;
  uint8_t reg19 = 0;
  uint8_t status1 = 0;
  uint8_t status2 = 0;
  uint8_t status3 = 0;
  uint8_t fault = 0;
  Reg16Value iac{};
  Reg16Value ibat{};
  Reg16Value vac{};
  Reg16Value vbat{};

  const bool ok = this->read_byte_(REG17_CHARGER_CONTROL, reg17) && this->read_byte_(REG19_POWER_PATH_CONTROL, reg19) &&
                  this->read_byte_(REG21_CHARGER_STATUS_1, status1) && this->read_byte_(REG22_CHARGER_STATUS_2, status2) &&
                  this->read_byte_(REG23_CHARGER_STATUS_3, status3) && this->read_byte_(REG24_FAULT_STATUS, fault) &&
                  this->read_u16_le_(REG2D_IAC_ADC, iac) && this->read_u16_le_(REG2F_IBAT_ADC, ibat) &&
                  this->read_u16_le_(REG31_VAC_ADC, vac) && this->read_u16_le_(REG33_VBAT_ADC, vbat);

  if (!ok) {
    ESP_LOGW(TAG, "Precheck: charge_enable->%s snapshot unavailable (read failed)", requested_on ? "on" : "off");
    return;
  }

  const float iac_ma = static_cast<float>(static_cast<int16_t>(iac.raw_le)) * IAC_CURRENT_LSB_MA;
  const float ibat_ma = static_cast<float>(static_cast<int16_t>(ibat.raw_le)) * IBAT_CURRENT_LSB_MA;
  const float vac_mv = static_cast<float>(vac.raw_le) * VOLTAGE_LSB_MV;
  const float vbat_mv = static_cast<float>(vbat.raw_le) * VOLTAGE_LSB_MV;
  const bool en_chg = (reg17 & REG17_EN_CHG_MASK) != 0;
  const bool en_hiz = (reg17 & REG17_EN_HIZ_MASK) != 0;
  const bool dis_ce_pin = (reg17 & REG17_DIS_CE_PIN_MASK) != 0;
  const bool en_rev = (reg19 & REG19_EN_REV_MASK) != 0;

  ESP_LOGI(
    TAG,
    "Precheck: req_charge_enable=%s reg17=0x%02X en_chg=%u en_hiz=%u dis_ce_pin=%u reg19=0x%02X en_rev=%u "
    "status=%02X/%02X/%02X fault=%02X vac=%.0fmV vbat=%.0fmV iac=%.1fmA ibat=%.0fmA",
    requested_on ? "on" : "off",
    reg17,
    en_chg ? 1 : 0,
    en_hiz ? 1 : 0,
    dis_ce_pin ? 1 : 0,
    reg19,
    en_rev ? 1 : 0,
    status1,
    status2,
    status3,
    fault,
    vac_mv,
    vbat_mv,
    iac_ma,
    ibat_ma
  );
}

void BQ25756ChargeEnableSwitch::write_state(bool state) {
  ESP_LOGI(TAG, "Action: charge_enable -> %s", state ? "on" : "off");
  if (this->parent_ != nullptr && this->parent_->set_charge_enabled(state)) {
    this->publish_state(state);
    ESP_LOGI(TAG, "Action result: charge_enable=%s", state ? "on" : "off");
    return;
  }
  ESP_LOGW(TAG, "Failed to set charge_enable to %s", state ? "on" : "off");
}

void BQ25756HizModeSwitch::write_state(bool state) {
  ESP_LOGI(TAG, "Action: hiz_mode -> %s", state ? "on" : "off");
  if (this->parent_ != nullptr && this->parent_->set_hiz_mode(state)) {
    this->publish_state(state);
    ESP_LOGI(TAG, "Action result: hiz_mode=%s", state ? "on" : "off");
    return;
  }
  ESP_LOGW(TAG, "Failed to set hiz_mode to %s", state ? "on" : "off");
}

void BQ25756ReverseModeSwitch::write_state(bool state) {
  ESP_LOGI(TAG, "Action: reverse_mode -> %s", state ? "on" : "off");
  if (this->parent_ != nullptr && this->parent_->set_reverse_mode(state)) {
    this->publish_state(state);
    ESP_LOGI(TAG, "Action result: reverse_mode=%s", state ? "on" : "off");
    return;
  }
  ESP_LOGW(TAG, "Failed to set reverse_mode to %s", state ? "on" : "off");
}

void BQ25756WatchdogSelect::control(size_t index) {
  if (index > 0x03) {
    return;
  }
  ESP_LOGI(TAG, "Action: watchdog -> %u", static_cast<unsigned>(index));
  if (this->parent_ != nullptr && this->parent_->set_watchdog_code(static_cast<uint8_t>(index))) {
    this->publish_state(index);
    ESP_LOGI(TAG, "Action result: watchdog=%u", static_cast<unsigned>(index));
    return;
  }
  ESP_LOGW(TAG, "Failed to set watchdog option index %u", static_cast<unsigned>(index));
}

void BQ25756WatchdogResetButton::press_action() {
  ESP_LOGI(TAG, "Action: watchdog_reset");
  if (this->parent_ != nullptr && this->parent_->reset_watchdog()) {
    ESP_LOGI(TAG, "Watchdog reset requested");
    return;
  }
  ESP_LOGW(TAG, "Failed to reset watchdog");
}

void BQ25756DumpRegistersButton::press_action() {
  ESP_LOGI(TAG, "Action: dump_registers");
  if (this->parent_ != nullptr && this->parent_->dump_registers_0x00_0x3D()) {
    return;
  }
  ESP_LOGW(TAG, "Failed register dump request");
}

}  // namespace bq25756
}  // namespace esphome
