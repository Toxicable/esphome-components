#include "bq25756.h"

#include "bq25756_register_manifest.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq25756 {

namespace {
static const char *const TAG = "bq25756.managed";
}

::bq25756_core::Bq25756Configuration BQ25756ManagedComponent::build_configuration_() const {
  ::bq25756_core::Bq25756Configuration config{};

  if (this->has_charge_voltage_limit_mv_) {
    config.charge_voltage_limit =
        ::bq25756_core::encode_charge_voltage_limit_mv(this->charge_voltage_limit_mv_);
  }
  if (this->has_charge_current_limit_ma_) {
    config.charge_current_limit =
        ::bq25756_core::encode_charge_current_limit_ma(this->charge_current_limit_ma_);
  }
  if (this->has_input_current_dpm_limit_ma_) {
    config.input_current_dpm_limit =
        ::bq25756_core::encode_input_current_dpm_limit_ma(this->input_current_dpm_limit_ma_);
  }
  if (this->has_input_voltage_dpm_limit_mv_) {
    config.input_voltage_dpm_limit =
        ::bq25756_core::encode_input_voltage_dpm_limit_mv(this->input_voltage_dpm_limit_mv_);
  }

  if (this->disable_watchdog_) {
    config.timer_control = static_cast<uint8_t>(
        config.timer_control & static_cast<uint8_t>(~::bq25756_core::REG15_WATCHDOG_MASK));
  }
  if (this->disable_ce_pin_) {
    config.charger_control = static_cast<uint8_t>(
        config.charger_control | ::bq25756_core::REG17_DIS_CE_PIN_MASK);
  }
  if (this->disable_ilim_hiz_pin_) {
    config.pin_control = static_cast<uint8_t>(
        config.pin_control & static_cast<uint8_t>(~::bq25756_core::REG18_EN_ILIM_HIZ_PIN_MASK));
  }
  if (this->disable_ichg_pin_) {
    config.pin_control = static_cast<uint8_t>(
        config.pin_control & static_cast<uint8_t>(~::bq25756_core::REG18_EN_ICHG_PIN_MASK));
  }
  if (this->disable_pfm_) {
    config.power_path_control = static_cast<uint8_t>(
        config.power_path_control & static_cast<uint8_t>(~::bq25756_core::REG19_EN_PFM_MASK));
  }

  // Normal telemetry does not need VFB. Calibration temporarily enables it and
  // the next poll restores this explicit steady-state channel configuration.
  config.adc_control = ::bq25756_core::REG2B_ADC_CONTINUOUS_15_BIT;
  config.adc_channel_control = ::bq25756_core::REG2C_VFB_ADC_DIS_MASK;
  return config;
}

bool BQ25756ManagedComponent::restore_manifest_configuration_() {
  ::bq25756_core::ControlStates controls{};
  if (!this->service_.read_control_states(controls)) {
    ESP_LOGW(TAG, "Cannot restore configuration: control state is unavailable");
    return false;
  }

  const bool restore_charge_enable = controls.charge_enabled;
  if (restore_charge_enable && !this->service_.set_charge_enabled(false)) {
    ESP_LOGW(TAG, "Cannot restore configuration: failed to disable charging first");
    return false;
  }

  const auto image = ::bq25756_core::make_configuration_image(this->build_configuration_());
  ::bq25756_core::ConfigurationReconcileResult result{};
  const bool restored = this->service_.reconcile_configuration(image, true, result);
  if (!result.io_ok) {
    ESP_LOGW(TAG, "Configuration restore failed during register I/O; charging remains disabled");
    return false;
  }
  if (!restored || !result.matches) {
    ESP_LOGW(TAG,
             "Configuration restore verification failed: desired=0x%08X observed=0x%08X "
             "remaining=%u; charging remains disabled",
             static_cast<unsigned>(result.desired_fingerprint),
             static_cast<unsigned>(result.observed_fingerprint),
             static_cast<unsigned>(result.remaining_mismatch_count));
    return false;
  }

  if (restore_charge_enable && !this->service_.set_charge_enabled(true)) {
    ESP_LOGW(TAG, "Configuration restored but prior charge-enable state could not be resumed");
    return false;
  }

  if (result.repaired) {
    ESP_LOGI(TAG,
             "Restored %u configuration register(s): desired=0x%08X observed=0x%08X",
             static_cast<unsigned>(result.repaired_count),
             static_cast<unsigned>(result.desired_fingerprint),
             static_cast<unsigned>(result.observed_fingerprint));
  } else {
    ESP_LOGD(TAG, "Configuration image verified: fingerprint=0x%08X",
             static_cast<unsigned>(result.desired_fingerprint));
  }
  this->manifest_ready_ = true;
  this->next_manifest_audit_ms_ = millis() + MANIFEST_AUDIT_INTERVAL_MS;
  this->publish_configuration_status_("configured");
  return true;
}

bool BQ25756ManagedComponent::audit_manifest_configuration_() {
  const auto image = ::bq25756_core::make_configuration_image(this->build_configuration_());
  ::bq25756_core::ConfigurationReconcileResult audit{};
  const bool matches = this->service_.reconcile_configuration(image, false, audit);
  if (!audit.io_ok) {
    ESP_LOGW(TAG, "Unable to audit complete charger configuration image");
    return false;
  }
  if (matches && audit.matches) {
    this->publish_configuration_status_("configured");
    return true;
  }

  ESP_LOGW(TAG,
           "Configuration image drift detected: registers=%u first=0x%02X "
           "desired=0x%08X observed=0x%08X",
           static_cast<unsigned>(audit.mismatch_count),
           static_cast<unsigned>(audit.first_mismatch_address),
           static_cast<unsigned>(audit.desired_fingerprint),
           static_cast<unsigned>(audit.observed_fingerprint));
  return this->restore_manifest_configuration_();
}

void BQ25756ManagedComponent::mark_communication_lost_() {
  if (this->initialized_ || this->manifest_ready_) {
    ESP_LOGW(TAG,
             "BQ25756 communication lost after %u failed poll cycle(s); full safe "
             "initialization will run after recovery",
             static_cast<unsigned>(this->consecutive_failed_cycles_));
  }
  this->initialized_ = false;
  this->manifest_ready_ = false;
  this->charger_snapshot_.valid = false;
  this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
  this->publish_configuration_status_("disconnected");
  this->status_set_warning();
}

void BQ25756ManagedComponent::setup() {
  this->io_failed_this_cycle_ = false;
  this->manifest_ready_ = false;
  this->consecutive_failed_cycles_ = 0;
  this->next_manifest_audit_ms_ = 0;

  BQ25756Component::setup();

  if (this->io_failed_this_cycle_) {
    this->consecutive_failed_cycles_ = 1;
    return;
  }
  if (!this->initialized_) {
    return;
  }
  if (!this->restore_manifest_configuration_()) {
    this->initialized_ = false;
    this->manifest_ready_ = false;
    this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
    this->publish_configuration_status_("repair_failed");
    this->status_set_warning();
  }
}

void BQ25756ManagedComponent::update() {
  const bool was_initialized = this->initialized_;
  this->io_failed_this_cycle_ = false;

  BQ25756Component::update();

  if (this->io_failed_this_cycle_) {
    if (this->consecutive_failed_cycles_ < UINT8_MAX) {
      this->consecutive_failed_cycles_++;
    }
    if (this->consecutive_failed_cycles_ >= COMMUNICATION_FAILURE_THRESHOLD) {
      this->mark_communication_lost_();
    }
    return;
  }

  this->consecutive_failed_cycles_ = 0;
  if (!this->initialized_) {
    return;
  }

  if (!this->manifest_ready_ || !was_initialized) {
    if (!this->restore_manifest_configuration_()) {
      this->initialized_ = false;
      this->manifest_ready_ = false;
      this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
      this->publish_configuration_status_("repair_failed");
      this->status_set_warning();
      return;
    }
    this->status_clear_warning();
    return;
  }

  const uint32_t now = millis();
  if (static_cast<int32_t>(now - this->next_manifest_audit_ms_) < 0) {
    return;
  }
  this->next_manifest_audit_ms_ = now + MANIFEST_AUDIT_INTERVAL_MS;
  if (!this->audit_manifest_configuration_()) {
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();
}

bool BQ25756ManagedComponent::read_registers(uint8_t reg, uint8_t *data, size_t len) {
  const bool ok = BQ25756Component::read_registers(reg, data, len);
  if (!ok) {
    this->io_failed_this_cycle_ = true;
  }
  return ok;
}

bool BQ25756ManagedComponent::write_registers(uint8_t reg, const uint8_t *data, size_t len) {
  const bool ok = BQ25756Component::write_registers(reg, data, len);
  if (!ok) {
    this->io_failed_this_cycle_ = true;
  }
  return ok;
}

}  // namespace bq25756
}  // namespace esphome
