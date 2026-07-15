#include "bq25756.h"

#include <array>
#include <cmath>
#include <cstdio>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq25756 {

namespace {
static const char *const TAG = "bq25756";
static constexpr uint32_t CALIBRATION_PREFERENCE_KEY = 0xB2575601;
}  // namespace

BQ25756Component::BQ25756Component() : service_(this) {}

bool BQ25756Component::set_charge_enabled(bool enabled) {
  this->log_charge_enable_precheck_(enabled);
  return this->service_.set_charge_enabled(enabled);
}

bool BQ25756Component::set_watchdog_code(uint8_t code) {
  return this->service_.set_watchdog_code(code);
}

void BQ25756Component::setup() {
  this->initialized_ = false;
  this->next_init_retry_ms_ = 0;
  this->load_calibration_();
  this->publish_calibration_status_(
      !this->restore_calibration_ ? "configured"
                                  : (this->calibration_restored_ ? "restored"
                                                                  : "not_calibrated"));

  if (!this->initialize_()) {
    ESP_LOGW(TAG, "BQ25756 init not ready at startup; will retry");
    this->status_set_warning();
    this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
    return;
  }

  this->status_clear_warning();
}

bool BQ25756Component::load_calibration_() {
  this->calibration_restored_ = false;
  if (global_preferences == nullptr) return false;
  this->calibration_preference_ = global_preferences->make_preference<FeedbackCalibration>(CALIBRATION_PREFERENCE_KEY);
  this->calibration_preference_valid_ = true;
  if (!this->restore_calibration_) return true;
  FeedbackCalibration calibration{};
  if (!this->calibration_preference_.load(&calibration) || calibration.version != 1 ||
      !std::isfinite(calibration.feedback_to_battery_ratio) || calibration.feedback_to_battery_ratio <= 0.0f) return false;
  this->fb_to_pack_voltage_scale_ = calibration.feedback_to_battery_ratio;
  this->has_fb_to_pack_voltage_scale_ = true;
  this->calibration_restored_ = true;
  ESP_LOGI(TAG, "Restored battery feedback calibration");
  return true;
}

bool BQ25756Component::save_calibration_() {
  const FeedbackCalibration calibration{1, this->fb_to_pack_voltage_scale_};
  return this->calibration_preference_valid_ && this->calibration_preference_.save(&calibration);
}

void BQ25756Component::publish_calibration_status_(const char *status) {
  if (this->calibration_status_text_sensor_ != nullptr) {
    this->calibration_status_text_sensor_->publish_state(status);
  }
}

bool BQ25756Component::apply_battery_target_() {
  if (!std::isfinite(this->battery_target_voltage_v_) || this->battery_target_voltage_v_ <= 0.0f ||
      !std::isfinite(this->fb_to_pack_voltage_scale_) || this->fb_to_pack_voltage_scale_ <= 0.0f) {
    ESP_LOGW(TAG, "Cannot set battery target: target=%.3f V, feedback ratio=%.6f", this->battery_target_voltage_v_,
             this->fb_to_pack_voltage_scale_);
    return false;
  }
  const int target_mv = static_cast<int>(std::lround(this->battery_target_voltage_v_ * 1000.0f / this->fb_to_pack_voltage_scale_ / 2.0f)) * 2;
  if (target_mv < 1504 || target_mv > 1566) {
    ESP_LOGW(TAG, "Battery target %.3f V requires VFB target %d mV; valid range is 1504 to 1566 mV",
             this->battery_target_voltage_v_, target_mv);
    return false;
  }
  this->charge_voltage_limit_mv_ = static_cast<uint16_t>(target_mv);
  this->has_charge_voltage_limit_mv_ = true;
  if (!this->service_.write_u16_le(::bq25756_core::REG00_CHARGE_VOLTAGE_LIMIT,
                                   ::bq25756_core::encode_charge_voltage_limit_mv(this->charge_voltage_limit_mv_))) {
    ESP_LOGW(TAG, "Failed to write VFB target %d mV", target_mv);
    return false;
  }
  return true;
}

bool BQ25756Component::calibrate_feedback(float measured_battery_voltage_v) {
  if (!std::isfinite(measured_battery_voltage_v) || measured_battery_voltage_v <= 0.0f) {
    ESP_LOGW(TAG, "Calibration needs a positive measured battery voltage (got %.3f V)", measured_battery_voltage_v);
    this->publish_calibration_status_("failed");
    return false;
  }
  ::bq25756_core::ControlStates controls{};
  if (!this->service_.read_control_states(controls) || controls.charge_enabled) {
    ESP_LOGW(TAG, "Disable charging before calibrating battery feedback");
    this->publish_calibration_status_("failed");
    return false;
  }
  ::bq25756_core::Measurements measurements{};
  if (!this->service_.read_measurements(measurements, true)) {
    ESP_LOGW(TAG, "Failed to read VFB during battery feedback calibration");
    this->publish_calibration_status_("failed");
    return false;
  }
  if (!std::isfinite(measurements.vfb_mv) || measurements.vfb_mv <= 0.0f) {
    ESP_LOGW(TAG, "Calibration VFB reading is invalid: %.1f mV", measurements.vfb_mv);
    this->publish_calibration_status_("failed");
    return false;
  }
  const float ratio = measured_battery_voltage_v * 1000.0f / measurements.vfb_mv;
  if (!std::isfinite(ratio) || ratio <= 0.0f) {
    ESP_LOGW(TAG, "Calibration ratio is invalid: measured=%.3f V, VFB=%.1f mV", measured_battery_voltage_v,
             measurements.vfb_mv);
    this->publish_calibration_status_("failed");
    return false;
  }
  const float previous_ratio = this->fb_to_pack_voltage_scale_;
  this->fb_to_pack_voltage_scale_ = ratio;
  this->has_fb_to_pack_voltage_scale_ = true;
  if (!this->apply_battery_target_()) {
    this->fb_to_pack_voltage_scale_ = previous_ratio;
    this->publish_calibration_status_("failed");
    return false;
  }
  if (!this->save_calibration_()) {
    ESP_LOGW(TAG, "Failed to save battery feedback calibration");
    this->fb_to_pack_voltage_scale_ = previous_ratio;
    this->publish_calibration_status_("failed");
    return false;
  }
  ESP_LOGI(TAG, "Calibrated battery feedback ratio to %.6f", ratio);
  this->calibration_restored_ = true;
  this->publish_calibration_status_("calibrated");
  return true;
}

bool BQ25756Component::calibrate_from_configured_voltage() {
  return this->calibration_voltage_number_ != nullptr &&
         this->calibrate_feedback(this->calibration_voltage_number_->state);
}

bool BQ25756Component::initialize_() {
  uint8_t part_info = 0;
  if (!this->service_.read_byte(::bq25756_core::REG3D_PART_INFORMATION, part_info)) {
    ESP_LOGW(TAG, "Failed to read REG3D (part information); device not responding yet");
    return false;
  }

  if ((part_info & ::bq25756_core::PART_NUM_MASK) != ::bq25756_core::BQ25756_PART_NUM_BITS) {
    ESP_LOGW(TAG, "Unexpected REG3D=0x%02X; PART_NUM does not match BQ25756", part_info);
    return false;
  }

  ESP_LOGI(TAG, "BQ25756 detected, REG3D=0x%02X", part_info);

  if (this->disable_watchdog_) {
    uint8_t reg15 = 0;
    if (!this->service_.read_byte(::bq25756_core::REG15_TIMER_CONTROL, reg15)) {
      ESP_LOGW(TAG, "Failed to read REG15 (timer control)");
      return false;
    }
    if ((reg15 & ::bq25756_core::REG15_WATCHDOG_MASK) != 0) {
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
  if (!this->apply_battery_target_()) {
    ESP_LOGW(TAG, "Failed to apply battery-profile charge voltage target");
    return false;
  }
  if (!this->apply_configured_pin_overrides_()) {
    ESP_LOGW(TAG, "Failed to apply configured pin control overrides");
    return false;
  }
  if (!this->set_charge_enabled(false)) {
    ESP_LOGW(TAG, "Failed to disable charging during initialization");
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

  ::bq25756_core::Status status;
  if (!this->service_.read_status(status)) {
    ESP_LOGW(TAG, "Failed reading charger status registers");
    this->status_set_warning();
    return;
  }

  ::bq25756_core::Measurements measurements;
  if (!this->service_.read_measurements(measurements, false)) {
    ESP_LOGW(TAG, "Failed reading one or more ADC registers");
    this->status_set_warning();
    return;
  }

  this->maybe_log_event_(
    status.status1, status.status2, status.status3, status.fault, measurements.iac_ma, measurements.ibat_ma,
    measurements.vac_mv, measurements.vbat_mv
  );

  ESP_LOGD(TAG, "STATUS[21..24]=%02X %02X %02X %02X", status.status1, status.status2, status.status3, status.fault);
  ESP_LOGD(
    TAG,
    "IAC=%.1f mA (0x%04X [%02X %02X]), IBAT=%.0f mA (0x%04X [%02X %02X]), "
    "VAC=%.0f mV (0x%04X [%02X %02X]), VBAT=%.0f mV (0x%04X [%02X %02X]), "
    "TS=%.3f%% (0x%04X [%02X %02X])%s",
    measurements.iac_ma,
    measurements.iac.raw_le,
    measurements.iac.lsb,
    measurements.iac.msb,
    measurements.ibat_ma,
    measurements.ibat.raw_le,
    measurements.ibat.lsb,
    measurements.ibat.msb,
    measurements.vac_mv,
    measurements.vac.raw_le,
    measurements.vac.lsb,
    measurements.vac.msb,
    measurements.vbat_mv,
    measurements.vbat.raw_le,
    measurements.vbat.lsb,
    measurements.vbat.msb,
    measurements.ts_percent,
    measurements.ts.raw_le,
    measurements.ts.lsb,
    measurements.ts.msb
  );

  if (this->iac_current_sensor_ != nullptr) {
    this->iac_current_sensor_->publish_state(measurements.iac_ma);
  }
  if (this->ibat_current_sensor_ != nullptr) {
    this->ibat_current_sensor_->publish_state(measurements.ibat_ma);
  }
  if (this->vac_voltage_sensor_ != nullptr) {
    this->vac_voltage_sensor_->publish_state(measurements.vac_mv);
  }
  if (this->vbat_voltage_sensor_ != nullptr) {
    this->vbat_voltage_sensor_->publish_state(measurements.vbat_mv);
  }
  if (this->ts_percent_sensor_ != nullptr) {
    this->ts_percent_sensor_->publish_state(measurements.ts_percent);
  }

  if (this->vfb_reg_target_sensor_ != nullptr || this->vbat_ov_rising_pack_sensor_ != nullptr ||
      this->vbat_ov_falling_pack_sensor_ != nullptr) {
    ::bq25756_core::Reg16Value vfb_reg{};
    if (!this->service_.read_u16_le(::bq25756_core::REG00_CHARGE_VOLTAGE_LIMIT, vfb_reg)) {
      ESP_LOGW(TAG, "Failed reading REG0x00 for VFB/VBAT_OV threshold diagnostics");
    } else {
      const float vfb_reg_mv = ::bq25756_core::vfb_reg_target_mv(vfb_reg.raw_le);
      const float vbat_ov_rising_fb_mv = vfb_reg_mv * ::bq25756_core::VBAT_OV_RISING_MULTIPLIER;
      const float vbat_ov_falling_fb_mv = vfb_reg_mv * ::bq25756_core::VBAT_OV_FALLING_MULTIPLIER;
      if (this->has_fb_to_pack_voltage_scale_) {
        const float target_pack_mv = vfb_reg_mv * this->fb_to_pack_voltage_scale_;
        const float rising_pack_mv = vbat_ov_rising_fb_mv * this->fb_to_pack_voltage_scale_;
        const float falling_pack_mv = vbat_ov_falling_fb_mv * this->fb_to_pack_voltage_scale_;
        if (this->vfb_reg_target_sensor_ != nullptr) {
          this->vfb_reg_target_sensor_->publish_state(target_pack_mv);
        }
        if (this->vbat_ov_rising_pack_sensor_ != nullptr) {
          this->vbat_ov_rising_pack_sensor_->publish_state(rising_pack_mv);
        }
        if (this->vbat_ov_falling_pack_sensor_ != nullptr) {
          this->vbat_ov_falling_pack_sensor_->publish_state(falling_pack_mv);
        }
      }
    }
  }

  this->publish_status_texts_(status);
  this->publish_control_states_();
  this->status_clear_warning();
}

void BQ25756Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ25756:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  disable_watchdog: %s", this->disable_watchdog_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  event_logging: %s", this->event_logging_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  charging.control: %s", this->disable_ce_pin_ ? "i2c" : "pins");
  if (this->has_charge_voltage_limit_mv_) {
    ESP_LOGCONFIG(TAG, "  charging battery feedback target: %u mV", this->charge_voltage_limit_mv_);
  }
  if (this->has_charge_current_limit_ma_) {
    ESP_LOGCONFIG(TAG, "  charging.battery_current_limit: %u mA", this->charge_current_limit_ma_);
  }
  if (this->has_input_current_dpm_limit_ma_) {
    ESP_LOGCONFIG(TAG, "  charging.input_current_limit: %u mA", this->input_current_dpm_limit_ma_);
  }
  if (this->has_input_voltage_dpm_limit_mv_) {
    ESP_LOGCONFIG(TAG, "  charging.input_voltage_dpm_threshold: %u mV", this->input_voltage_dpm_limit_mv_);
  }
  if (this->has_fb_to_pack_voltage_scale_) {
    ESP_LOGCONFIG(TAG, "  charging.battery_voltage.feedback_to_battery_ratio: %.6f", this->fb_to_pack_voltage_scale_);
  }
  LOG_SENSOR("  ", "IAC Current", this->iac_current_sensor_);
  LOG_SENSOR("  ", "IBAT Current", this->ibat_current_sensor_);
  LOG_SENSOR("  ", "VAC Voltage", this->vac_voltage_sensor_);
  LOG_SENSOR("  ", "VBAT Voltage", this->vbat_voltage_sensor_);
  LOG_SENSOR("  ", "TS Percent", this->ts_percent_sensor_);
  LOG_SENSOR("  ", "Charge Voltage Target", this->vfb_reg_target_sensor_);
  LOG_SENSOR("  ", "Battery Overvoltage Rising", this->vbat_ov_rising_pack_sensor_);
  LOG_SENSOR("  ", "Battery Overvoltage Falling", this->vbat_ov_falling_pack_sensor_);
  LOG_TEXT_SENSOR("  ", "Charge Status", this->charge_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "TS Status", this->ts_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "MPPT Status", this->mppt_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Status Flags", this->status_flags_text_sensor_);
  LOG_SWITCH("  ", "Charge Enable", this->charge_enable_switch_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
}

bool BQ25756Component::dump_registers_0x00_0x3D() {
  std::array<uint8_t, 0x3E> regs{};
  if (!this->service_.read_bytes(0x00, regs.data(), regs.size())) {
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

bool BQ25756Component::read_registers(uint8_t reg, uint8_t *data, size_t len) {
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

bool BQ25756Component::write_registers(uint8_t reg, const uint8_t *data, size_t len) {
  if (len == 0) {
    return true;
  }
  return this->write_bytes(reg, data, len);
}

void BQ25756Component::publish_status_texts_(const ::bq25756_core::Status &status) {
  const bool pg_good = (status.status2 & 0x80) != 0;
  const bool watchdog_expired = (status.status1 & 0x08) != 0;
  const bool iac_dpm_active = (status.status1 & 0x40) != 0;
  const bool vac_dpm_active = (status.status1 & 0x20) != 0;
  const bool reverse_active = (status.status3 & 0x04) != 0;
  const bool cv_timer_expired = (status.status3 & 0x08) != 0;
  const bool charge_timer_expired = (status.fault & 0x04) != 0;
  const bool vac_uv_fault = (status.fault & 0x80) != 0;
  const bool vac_ov_fault = (status.fault & 0x40) != 0;
  const bool ibat_ocp_fault = (status.fault & 0x20) != 0;
  const bool vbat_ov_fault = (status.fault & 0x10) != 0;
  const bool thermal_shutdown = (status.fault & 0x08) != 0;
  const bool drv_sup_fault = (status.fault & 0x02) != 0;

  if (this->charge_status_text_sensor_ != nullptr) {
    this->charge_status_text_sensor_->publish_state(::bq25756_core::charge_status_to_string(status.status1 & 0x07));
  }
  if (this->ts_status_text_sensor_ != nullptr) {
    this->ts_status_text_sensor_->publish_state(::bq25756_core::ts_status_to_string((status.status2 >> 4) & 0x07));
  }
  if (this->mppt_status_text_sensor_ != nullptr) {
    this->mppt_status_text_sensor_->publish_state(::bq25756_core::mppt_status_to_string(status.status2 & 0x03));
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
  if (this->charge_enable_switch_ == nullptr) {
    return;
  }

  ::bq25756_core::ControlStates states;
  if (!this->service_.read_control_states(states)) {
    ESP_LOGW(TAG, "Failed to refresh control states");
    return;
  }

  if (this->charge_enable_switch_ != nullptr) {
    this->charge_enable_switch_->publish_state(states.charge_enabled);
  }
}

bool BQ25756Component::ensure_adc_enabled_() {
  uint8_t reg2b = 0;
  uint8_t reg2b_new = 0;
  uint8_t reg2c = 0;
  uint8_t reg2c_new = 0;
  // Feedback calibration always needs this ADC channel, even when its optional
  // diagnostic sensor is not exposed to Home Assistant.
  if (!this->service_.ensure_adc_enabled(true, reg2b, reg2b_new, reg2c, reg2c_new)) {
    ESP_LOGW(TAG, "ADC configuration write failed");
    return false;
  }

  ESP_LOGI(TAG, "ADC config REG2B: 0x%02X -> 0x%02X, REG2C: 0x%02X -> 0x%02X", reg2b, reg2b_new, reg2c, reg2c_new);
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

  const char *const charge = ::bq25756_core::charge_status_to_string(status1 & 0x07);
  const char *const ts = ::bq25756_core::ts_status_to_string((status2 >> 4) & 0x07);
  const char *const mppt = ::bq25756_core::mppt_status_to_string(status2 & 0x03);
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
  if (!this->service_.apply_limits(
        this->has_charge_voltage_limit_mv_, this->charge_voltage_limit_mv_, this->has_charge_current_limit_ma_,
        this->charge_current_limit_ma_, this->has_input_current_dpm_limit_ma_, this->input_current_dpm_limit_ma_,
        this->has_input_voltage_dpm_limit_mv_, this->input_voltage_dpm_limit_mv_
      )) {
    ESP_LOGW(TAG, "Failed writing one or more configured charge/input limits");
    return false;
  }

  return true;
}

bool BQ25756Component::apply_configured_pin_overrides_() {
  if (!this->service_.apply_pin_overrides(this->disable_ce_pin_, this->disable_ilim_hiz_pin_, this->disable_ichg_pin_)) {
    ESP_LOGW(TAG, "Failed applying one or more pin control overrides");
    return false;
  }
  return true;
}

void BQ25756Component::log_charge_enable_precheck_(bool requested_on) {
  ::bq25756_core::ChargePrecheckSnapshot snapshot;
  if (!this->service_.read_charge_precheck(snapshot)) {
    ESP_LOGW(TAG, "Precheck: charge_enable->%s snapshot unavailable (read failed)", requested_on ? "on" : "off");
    return;
  }

  ESP_LOGI(
    TAG,
    "Precheck: req_charge_enable=%s reg17=0x%02X en_chg=%u en_hiz=%u dis_ce_pin=%u reg19=0x%02X en_rev=%u "
    "status=%02X/%02X/%02X fault=%02X vac=%.0fmV vbat=%.0fmV iac=%.1fmA ibat=%.0fmA",
    requested_on ? "on" : "off",
    snapshot.reg17,
    snapshot.en_chg ? 1 : 0,
    snapshot.en_hiz ? 1 : 0,
    snapshot.dis_ce_pin ? 1 : 0,
    snapshot.reg19,
    snapshot.en_rev ? 1 : 0,
    snapshot.status.status1,
    snapshot.status.status2,
    snapshot.status.status3,
    snapshot.status.fault,
    snapshot.measurements.vac_mv,
    snapshot.measurements.vbat_mv,
    snapshot.measurements.iac_ma,
    snapshot.measurements.ibat_ma
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

void BQ25756DumpRegistersButton::press_action() {
  ESP_LOGI(TAG, "Action: dump_registers");
  if (this->parent_ != nullptr && this->parent_->dump_registers_0x00_0x3D()) {
    return;
  }
  ESP_LOGW(TAG, "Failed register dump request");
}

void BQ25756CalibrateFeedbackButton::press_action() {
  if (this->parent_ != nullptr && !this->parent_->calibrate_from_configured_voltage()) {
    ESP_LOGW(TAG, "Battery feedback calibration failed");
  }
}

}  // namespace bq25756
}  // namespace esphome
