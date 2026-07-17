#include "bq25756.h"

#include "bq25756_register_config.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq25756 {

namespace {
static const char *const TAG = "bq25756.connection";
}

::bq25756_core::Bq25756RegisterConfig
BQ25756ComponentImpl::build_register_config_() const {
  ::bq25756_core::Bq25756RegisterConfig config{};

  if (this->has_charge_voltage_limit_mv_) {
    config.charge_voltage_limit =
        ::bq25756_core::encode_charge_voltage_limit_mv(
            this->charge_voltage_limit_mv_);
  }
  if (this->has_charge_current_limit_ma_) {
    config.charge_current_limit =
        ::bq25756_core::encode_charge_current_limit_ma(
            this->charge_current_limit_ma_);
  }
  if (this->has_input_current_dpm_limit_ma_) {
    config.input_current_dpm_limit =
        ::bq25756_core::encode_input_current_dpm_limit_ma(
            this->input_current_dpm_limit_ma_);
  }
  if (this->has_input_voltage_dpm_limit_mv_) {
    config.input_voltage_dpm_limit =
        ::bq25756_core::encode_input_voltage_dpm_limit_mv(
            this->input_voltage_dpm_limit_mv_);
  }

  if (this->disable_watchdog_) {
    config.timer_control = static_cast<uint8_t>(
        config.timer_control &
        static_cast<uint8_t>(~::bq25756_core::REG15_WATCHDOG_MASK));
  }
  if (this->disable_ce_pin_) {
    config.charger_control = static_cast<uint8_t>(
        config.charger_control | ::bq25756_core::REG17_DIS_CE_PIN_MASK);
  }
  if (this->disable_ilim_hiz_pin_) {
    config.pin_control = static_cast<uint8_t>(
        config.pin_control & static_cast<uint8_t>(
                                 ~::bq25756_core::REG18_EN_ILIM_HIZ_PIN_MASK));
  }
  if (this->disable_ichg_pin_) {
    config.pin_control = static_cast<uint8_t>(
        config.pin_control & static_cast<uint8_t>(
                                 ~::bq25756_core::REG18_EN_ICHG_PIN_MASK));
  }
  if (this->disable_pfm_) {
    config.power_path_control = static_cast<uint8_t>(
        config.power_path_control &
        static_cast<uint8_t>(~::bq25756_core::REG19_EN_PFM_MASK));
  }

  // Normal telemetry does not need VFB. Calibration temporarily enables it.
  config.adc_control = ::bq25756_core::REG2B_ADC_CONTINUOUS_15_BIT;
  config.adc_channel_control = ::bq25756_core::REG2C_VFB_ADC_DIS_MASK;
  return config;
}

void BQ25756ComponentImpl::set_connection_state_(
    ::component_common::ConnectionState state) {
  if (this->connection_state_ == state) {
    return;
  }

  ESP_LOGI(TAG, "Connection state: %s -> %s",
           ::component_common::connection_state_to_string(this->connection_state_).data(),
           ::component_common::connection_state_to_string(state).data());
  this->connection_state_ = state;

  if (state == ::component_common::ConnectionState::DISCONNECTED) {
    this->publish_configuration_status_("disconnected");
  } else if (state == ::component_common::ConnectionState::CONNECTING) {
    this->publish_configuration_status_("connecting");
  }
}

bool BQ25756ComponentImpl::sync_register_config_() {
  // A newly connected device may be at reset defaults. Keep charging disabled
  // until the complete desired register configuration has been verified.
  if (!this->service_.set_charge_enabled(false)) {
    ESP_LOGW(TAG, "Cannot sync register config: failed to disable charging");
    return false;
  }

  const auto image = ::bq25756_core::make_register_config_image(
      this->build_register_config_());
  ::bq25756_core::ConfigurationReconcileResult result{};
  const bool synced =
      this->service_.reconcile_configuration(image, true, result);

  if (!result.io_ok) {
    ESP_LOGW(TAG, "Register config sync failed during register I/O");
    return false;
  }
  if (!synced || !result.matches) {
    ESP_LOGW(TAG,
             "Register config verification failed: desired=0x%08X "
             "observed=0x%08X remaining=%u",
             static_cast<unsigned>(result.desired_fingerprint),
             static_cast<unsigned>(result.observed_fingerprint),
             static_cast<unsigned>(result.remaining_mismatch_count));
    this->publish_configuration_status_("sync_failed");
    return false;
  }

  this->register_config_synced_ = true;
  this->publish_configuration_status_("configured");
  if (result.repaired) {
    ESP_LOGI(TAG,
             "Synced %u register(s) for the new connected session: "
             "fingerprint=0x%08X",
             static_cast<unsigned>(result.repaired_count),
             static_cast<unsigned>(result.desired_fingerprint));
  } else {
    ESP_LOGD(TAG, "Register config already matched: fingerprint=0x%08X",
             static_cast<unsigned>(result.desired_fingerprint));
  }
  return true;
}

void BQ25756ComponentImpl::set_disconnected_() {
  if (this->connection_state_ !=
      ::component_common::ConnectionState::DISCONNECTED) {
    ESP_LOGW(TAG,
             "BQ25756 disconnected after %u failed poll cycle(s); the next "
             "connected session will be configured again",
             static_cast<unsigned>(this->consecutive_failed_cycles_));
  }

  this->initialized_ = false;
  this->register_config_synced_ = false;
  this->charger_snapshot_.valid = false;
  this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
  this->set_connection_state_(
      ::component_common::ConnectionState::DISCONNECTED);
  this->status_set_warning();
}

void BQ25756ComponentImpl::setup() {
  this->io_failed_this_cycle_ = false;
  this->register_config_synced_ = false;
  this->consecutive_failed_cycles_ = 0;
  this->connection_state_ =
      ::component_common::ConnectionState::CONNECTING;
  this->publish_configuration_status_("connecting");

  BQ25756Component::setup();

  if (this->io_failed_this_cycle_ || !this->initialized_) {
    this->consecutive_failed_cycles_ = this->io_failed_this_cycle_ ? 1 : 0;
    this->set_disconnected_();
    return;
  }

  this->set_connection_state_(
      ::component_common::ConnectionState::CONNECTED);
  if (!this->sync_register_config_()) {
    if (this->io_failed_this_cycle_) {
      this->set_disconnected_();
    } else {
      this->initialized_ = false;
      this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
      this->status_set_warning();
    }
    return;
  }
  this->status_clear_warning();
}

void BQ25756ComponentImpl::update() {
  this->io_failed_this_cycle_ = false;

  if (this->connection_state_ ==
          ::component_common::ConnectionState::DISCONNECTED &&
      static_cast<int32_t>(millis() - this->next_init_retry_ms_) >= 0) {
    this->set_connection_state_(
        ::component_common::ConnectionState::CONNECTING);
  }

  BQ25756Component::update();

  if (this->io_failed_this_cycle_) {
    if (this->consecutive_failed_cycles_ < UINT8_MAX) {
      this->consecutive_failed_cycles_++;
    }
    if (this->consecutive_failed_cycles_ >=
        COMMUNICATION_FAILURE_THRESHOLD) {
      this->set_disconnected_();
    }
    return;
  }

  this->consecutive_failed_cycles_ = 0;
  if (!this->initialized_) {
    return;
  }

  if (this->connection_state_ !=
      ::component_common::ConnectionState::CONNECTED) {
    this->set_connection_state_(
        ::component_common::ConnectionState::CONNECTED);
    this->register_config_synced_ = false;
  }

  if (!this->register_config_synced_) {
    if (!this->sync_register_config_()) {
      if (this->io_failed_this_cycle_) {
        this->set_disconnected_();
      } else {
        this->initialized_ = false;
        this->next_init_retry_ms_ = millis() + INIT_RETRY_INTERVAL_MS;
        this->status_set_warning();
      }
      return;
    }
    this->status_clear_warning();
  }
}

bool BQ25756ComponentImpl::read_registers(uint8_t reg, uint8_t *data,
                                         size_t len) {
  const bool ok = BQ25756Component::read_registers(reg, data, len);
  if (!ok) {
    this->io_failed_this_cycle_ = true;
  }
  return ok;
}

bool BQ25756ComponentImpl::write_registers(uint8_t reg,
                                          const uint8_t *data, size_t len) {
  const bool ok = BQ25756Component::write_registers(reg, data, len);
  if (!ok) {
    this->io_failed_this_cycle_ = true;
  }
  return ok;
}

}  // namespace bq25756
}  // namespace esphome
