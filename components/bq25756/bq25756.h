#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bq25756_bus.h"
#include "bq25756_service.h"

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq25756 {

class BQ25756Component : public PollingComponent, public i2c::I2CDevice, public ::bq25756_core::RegisterBus {
 public:
  BQ25756Component();

  void set_disable_watchdog(bool disable_watchdog) {
    disable_watchdog_ = disable_watchdog;
  }
  void set_event_logging(bool event_logging) {
    event_logging_ = event_logging;
  }
  void set_disable_ce_pin(bool disable_ce_pin) {
    disable_ce_pin_ = disable_ce_pin;
  }
  void set_disable_ilim_hiz_pin(bool disable_ilim_hiz_pin) {
    disable_ilim_hiz_pin_ = disable_ilim_hiz_pin;
  }
  void set_disable_ichg_pin(bool disable_ichg_pin) {
    disable_ichg_pin_ = disable_ichg_pin;
  }
  void set_charge_voltage_limit_mv(uint16_t charge_voltage_limit_mv) {
    charge_voltage_limit_mv_ = charge_voltage_limit_mv;
    has_charge_voltage_limit_mv_ = true;
  }
  void set_charge_current_limit_ma(uint16_t charge_current_limit_ma) {
    charge_current_limit_ma_ = charge_current_limit_ma;
    has_charge_current_limit_ma_ = true;
  }
  void set_input_current_dpm_limit_ma(uint16_t input_current_dpm_limit_ma) {
    input_current_dpm_limit_ma_ = input_current_dpm_limit_ma;
    has_input_current_dpm_limit_ma_ = true;
  }
  void set_input_voltage_dpm_limit_mv(uint16_t input_voltage_dpm_limit_mv) {
    input_voltage_dpm_limit_mv_ = input_voltage_dpm_limit_mv;
    has_input_voltage_dpm_limit_mv_ = true;
  }
  void set_fb_to_pack_voltage_scale(float fb_to_pack_voltage_scale) {
    fb_to_pack_voltage_scale_ = fb_to_pack_voltage_scale;
    has_fb_to_pack_voltage_scale_ = true;
  }

  void set_iac_current_sensor(sensor::Sensor *sensor) {
    iac_current_sensor_ = sensor;
  }
  void set_ibat_current_sensor(sensor::Sensor *sensor) {
    ibat_current_sensor_ = sensor;
  }
  void set_vac_voltage_sensor(sensor::Sensor *sensor) {
    vac_voltage_sensor_ = sensor;
  }
  void set_vbat_voltage_sensor(sensor::Sensor *sensor) {
    vbat_voltage_sensor_ = sensor;
  }
  void set_ts_percent_sensor(sensor::Sensor *sensor) {
    ts_percent_sensor_ = sensor;
  }
  void set_vfb_voltage_sensor(sensor::Sensor *sensor) {
    vfb_voltage_sensor_ = sensor;
  }
  void set_vfb_reg_target_sensor(sensor::Sensor *sensor) {
    vfb_reg_target_sensor_ = sensor;
  }
  void set_vbat_ov_rising_fb_sensor(sensor::Sensor *sensor) {
    vbat_ov_rising_fb_sensor_ = sensor;
  }
  void set_vbat_ov_falling_fb_sensor(sensor::Sensor *sensor) {
    vbat_ov_falling_fb_sensor_ = sensor;
  }
  void set_vbat_ov_rising_pack_sensor(sensor::Sensor *sensor) {
    vbat_ov_rising_pack_sensor_ = sensor;
  }
  void set_vbat_ov_falling_pack_sensor(sensor::Sensor *sensor) {
    vbat_ov_falling_pack_sensor_ = sensor;
  }

  void set_charge_status_text_sensor(text_sensor::TextSensor *sensor) {
    charge_status_text_sensor_ = sensor;
  }
  void set_ts_status_text_sensor(text_sensor::TextSensor *sensor) {
    ts_status_text_sensor_ = sensor;
  }
  void set_mppt_status_text_sensor(text_sensor::TextSensor *sensor) {
    mppt_status_text_sensor_ = sensor;
  }
  void set_status_flags_text_sensor(text_sensor::TextSensor *sensor) {
    status_flags_text_sensor_ = sensor;
  }

  void set_charge_enable_switch(switch_::Switch *sw) {
    charge_enable_switch_ = sw;
  }
  void set_hiz_mode_switch(switch_::Switch *sw) {
    hiz_mode_switch_ = sw;
  }
  void set_reverse_mode_switch(switch_::Switch *sw) {
    reverse_mode_switch_ = sw;
  }
  void set_watchdog_select(select::Select *sel) {
    watchdog_select_ = sel;
  }

  bool set_charge_enabled(bool enabled);
  bool set_hiz_mode(bool enabled);
  bool set_reverse_mode(bool enabled);
  bool set_watchdog_code(uint8_t code);
  bool reset_watchdog();
  void log_charge_enable_precheck_(bool requested_on);

  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_registers(uint8_t reg, uint8_t *data, size_t len) override;
  bool write_registers(uint8_t reg, const uint8_t *data, size_t len) override;

  bool dump_registers_0x00_0x3D();

 protected:
  void publish_status_texts_(const ::bq25756_core::Status &status);
  void publish_control_states_();
  bool initialize_();
  bool apply_configured_limits_();
  bool apply_configured_pin_overrides_();
  bool ensure_adc_enabled_();
  void maybe_log_event_(
    uint8_t status1, uint8_t status2, uint8_t status3, uint8_t fault, float iac_ma, float ibat_ma, float vac_mv, float vbat_mv
  );

  ::bq25756_core::Bq25756Service service_;

  sensor::Sensor *iac_current_sensor_{nullptr};
  sensor::Sensor *ibat_current_sensor_{nullptr};
  sensor::Sensor *vac_voltage_sensor_{nullptr};
  sensor::Sensor *vbat_voltage_sensor_{nullptr};
  sensor::Sensor *ts_percent_sensor_{nullptr};
  sensor::Sensor *vfb_voltage_sensor_{nullptr};
  sensor::Sensor *vfb_reg_target_sensor_{nullptr};
  sensor::Sensor *vbat_ov_rising_fb_sensor_{nullptr};
  sensor::Sensor *vbat_ov_falling_fb_sensor_{nullptr};
  sensor::Sensor *vbat_ov_rising_pack_sensor_{nullptr};
  sensor::Sensor *vbat_ov_falling_pack_sensor_{nullptr};

  text_sensor::TextSensor *charge_status_text_sensor_{nullptr};
  text_sensor::TextSensor *ts_status_text_sensor_{nullptr};
  text_sensor::TextSensor *mppt_status_text_sensor_{nullptr};
  text_sensor::TextSensor *status_flags_text_sensor_{nullptr};

  switch_::Switch *charge_enable_switch_{nullptr};
  switch_::Switch *hiz_mode_switch_{nullptr};
  switch_::Switch *reverse_mode_switch_{nullptr};
  select::Select *watchdog_select_{nullptr};

  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};
  static constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;

  bool disable_watchdog_{true};
  bool event_logging_{true};
  bool disable_ce_pin_{false};
  bool disable_ilim_hiz_pin_{false};
  bool disable_ichg_pin_{false};
  bool has_charge_voltage_limit_mv_{false};
  bool has_charge_current_limit_ma_{false};
  bool has_input_current_dpm_limit_ma_{false};
  bool has_input_voltage_dpm_limit_mv_{false};
  uint16_t charge_voltage_limit_mv_{1536};
  uint16_t charge_current_limit_ma_{20000};
  uint16_t input_current_dpm_limit_ma_{20000};
  uint16_t input_voltage_dpm_limit_mv_{4200};
  bool has_fb_to_pack_voltage_scale_{false};
  float fb_to_pack_voltage_scale_{1.0f};
  bool has_last_event_status_{false};
  uint8_t last_status1_{0};
  uint8_t last_status2_{0};
  uint8_t last_status3_{0};
  uint8_t last_fault_{0};
};

class BQ25756ChargeEnableSwitch : public switch_::Switch, public Parented<BQ25756Component> {
 protected:
  void write_state(bool state) override;
};

class BQ25756HizModeSwitch : public switch_::Switch, public Parented<BQ25756Component> {
 protected:
  void write_state(bool state) override;
};

class BQ25756ReverseModeSwitch : public switch_::Switch, public Parented<BQ25756Component> {
 protected:
  void write_state(bool state) override;
};

class BQ25756WatchdogSelect : public select::Select, public Parented<BQ25756Component> {
 protected:
  void control(size_t index) override;
};

class BQ25756WatchdogResetButton : public button::Button, public Parented<BQ25756Component> {
 protected:
  void press_action() override;
};

class BQ25756DumpRegistersButton : public button::Button, public Parented<BQ25756Component> {
 protected:
  void press_action() override;
};

}  // namespace bq25756
}  // namespace esphome
