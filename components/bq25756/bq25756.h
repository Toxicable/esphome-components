#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq25756 {

class BQ25756Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_disable_watchdog(bool disable_watchdog) {
    disable_watchdog_ = disable_watchdog;
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

  void set_pg_good_binary_sensor(binary_sensor::BinarySensor *sensor) {
    pg_good_binary_sensor_ = sensor;
  }
  void set_watchdog_expired_binary_sensor(binary_sensor::BinarySensor *sensor) {
    watchdog_expired_binary_sensor_ = sensor;
  }
  void set_iac_dpm_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    iac_dpm_active_binary_sensor_ = sensor;
  }
  void set_vac_dpm_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    vac_dpm_active_binary_sensor_ = sensor;
  }
  void set_reverse_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    reverse_active_binary_sensor_ = sensor;
  }
  void set_cv_timer_expired_binary_sensor(binary_sensor::BinarySensor *sensor) {
    cv_timer_expired_binary_sensor_ = sensor;
  }
  void set_charge_timer_expired_binary_sensor(binary_sensor::BinarySensor *sensor) {
    charge_timer_expired_binary_sensor_ = sensor;
  }
  void set_vac_uv_fault_binary_sensor(binary_sensor::BinarySensor *sensor) {
    vac_uv_fault_binary_sensor_ = sensor;
  }
  void set_vac_ov_fault_binary_sensor(binary_sensor::BinarySensor *sensor) {
    vac_ov_fault_binary_sensor_ = sensor;
  }
  void set_ibat_ocp_fault_binary_sensor(binary_sensor::BinarySensor *sensor) {
    ibat_ocp_fault_binary_sensor_ = sensor;
  }
  void set_vbat_ov_fault_binary_sensor(binary_sensor::BinarySensor *sensor) {
    vbat_ov_fault_binary_sensor_ = sensor;
  }
  void set_thermal_shutdown_binary_sensor(binary_sensor::BinarySensor *sensor) {
    thermal_shutdown_binary_sensor_ = sensor;
  }
  void set_drv_sup_fault_binary_sensor(binary_sensor::BinarySensor *sensor) {
    drv_sup_fault_binary_sensor_ = sensor;
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

  void setup() override;
  void update() override;
  void dump_config() override;

  bool dump_registers_0x00_0x3D();

 protected:
  struct Reg16Value {
    uint16_t raw_le{0};
    uint8_t lsb{0};
    uint8_t msb{0};
  };

  bool read_byte_(uint8_t reg, uint8_t &value);
  bool read_bytes_(uint8_t reg, uint8_t *data, size_t len);
  bool write_byte_(uint8_t reg, uint8_t value);
  bool read_u16_le_(uint8_t reg, Reg16Value &value);
  bool update_register_bits_(uint8_t reg, uint8_t mask, uint8_t value_bits);
  bool read_control_states_(bool &charge_enabled, bool &hiz_mode, bool &reverse_mode, uint8_t &watchdog_code);
  void publish_status_texts_(uint8_t status1, uint8_t status2, uint8_t status3, uint8_t fault);
  void publish_control_states_();
  const char *charge_status_to_string_(uint8_t charge_status) const;
  const char *ts_status_to_string_(uint8_t ts_status) const;
  const char *mppt_status_to_string_(uint8_t mppt_status) const;
  bool initialize_();
  bool ensure_adc_enabled_();

  sensor::Sensor *iac_current_sensor_{nullptr};
  sensor::Sensor *ibat_current_sensor_{nullptr};
  sensor::Sensor *vac_voltage_sensor_{nullptr};
  sensor::Sensor *vbat_voltage_sensor_{nullptr};
  sensor::Sensor *ts_percent_sensor_{nullptr};
  sensor::Sensor *vfb_voltage_sensor_{nullptr};

  text_sensor::TextSensor *charge_status_text_sensor_{nullptr};
  text_sensor::TextSensor *ts_status_text_sensor_{nullptr};
  text_sensor::TextSensor *mppt_status_text_sensor_{nullptr};
  text_sensor::TextSensor *status_flags_text_sensor_{nullptr};

  binary_sensor::BinarySensor *pg_good_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *watchdog_expired_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *iac_dpm_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *vac_dpm_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *reverse_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *cv_timer_expired_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *charge_timer_expired_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *vac_uv_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *vac_ov_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *ibat_ocp_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *vbat_ov_fault_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *thermal_shutdown_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *drv_sup_fault_binary_sensor_{nullptr};

  switch_::Switch *charge_enable_switch_{nullptr};
  switch_::Switch *hiz_mode_switch_{nullptr};
  switch_::Switch *reverse_mode_switch_{nullptr};
  select::Select *watchdog_select_{nullptr};

  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};
  static constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;

  bool disable_watchdog_{true};
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
