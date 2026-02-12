#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq25798 {

class BQ25798Component : public PollingComponent, public i2c::I2CDevice {
public:
  void set_disable_watchdog(bool disable_watchdog) { disable_watchdog_ = disable_watchdog; }

  void set_ibus_current_sensor(sensor::Sensor *sensor) { ibus_current_sensor_ = sensor; }
  void set_ibat_current_sensor(sensor::Sensor *sensor) { ibat_current_sensor_ = sensor; }
  void set_vbus_voltage_sensor(sensor::Sensor *sensor) { vbus_voltage_sensor_ = sensor; }
  void set_vbat_voltage_sensor(sensor::Sensor *sensor) { vbat_voltage_sensor_ = sensor; }
  void set_vsys_voltage_sensor(sensor::Sensor *sensor) { vsys_voltage_sensor_ = sensor; }
  void set_ts_percent_sensor(sensor::Sensor *sensor) { ts_percent_sensor_ = sensor; }
  void set_die_temperature_sensor(sensor::Sensor *sensor) { die_temperature_sensor_ = sensor; }

  void set_charge_status_text_sensor(text_sensor::TextSensor *sensor) { charge_status_text_sensor_ = sensor; }
  void set_vbus_status_text_sensor(text_sensor::TextSensor *sensor) { vbus_status_text_sensor_ = sensor; }
  void set_status_flags_text_sensor(text_sensor::TextSensor *sensor) { status_flags_text_sensor_ = sensor; }

  void set_charge_enable_switch(switch_::Switch *sw) { charge_enable_switch_ = sw; }
  void set_hiz_mode_switch(switch_::Switch *sw) { hiz_mode_switch_ = sw; }
  void set_otg_mode_switch(switch_::Switch *sw) { otg_mode_switch_ = sw; }
  void set_watchdog_select(select::Select *sel) { watchdog_select_ = sel; }

  bool set_charge_enabled(bool enabled);
  bool set_hiz_mode(bool enabled);
  bool set_otg_mode(bool enabled);
  bool set_watchdog_code(uint8_t code);
  bool reset_watchdog();

  void setup() override;
  void update() override;
  void dump_config() override;

  // Datasheet notes a multi-read can cross register boundaries.
  // This helper dumps the contiguous byte space 0x00..0x48.
  bool dump_registers_0x00_0x48();

protected:
  struct Reg16Value {
    uint16_t raw_be{0};
    uint8_t msb{0};
    uint8_t lsb{0};
  };

  bool read_byte_(uint8_t reg, uint8_t &value);
  bool read_bytes_(uint8_t reg, uint8_t *data, size_t len);
  bool write_byte_(uint8_t reg, uint8_t value);
  bool write_bytes_(uint8_t reg, const uint8_t *data, size_t len);
  bool read_u16_be_(uint8_t reg, Reg16Value &value);
  bool update_register_bits_(uint8_t reg, uint8_t mask, uint8_t value_bits);
  bool read_control_states_(bool &charge_enabled, bool &hiz_mode, bool &otg_mode, uint8_t &watchdog_code);
  void publish_status_texts_(const std::array<uint8_t, 5> &status);
  void publish_control_states_();
  const char *charge_status_to_string_(uint8_t charge_status) const;
  const char *vbus_status_to_string_(uint8_t vbus_status) const;
  bool initialize_();
  bool ensure_adc_enabled_();

  sensor::Sensor *ibus_current_sensor_{nullptr};
  sensor::Sensor *ibat_current_sensor_{nullptr};
  sensor::Sensor *vbus_voltage_sensor_{nullptr};
  sensor::Sensor *vbat_voltage_sensor_{nullptr};
  sensor::Sensor *vsys_voltage_sensor_{nullptr};
  sensor::Sensor *ts_percent_sensor_{nullptr};
  sensor::Sensor *die_temperature_sensor_{nullptr};

  text_sensor::TextSensor *charge_status_text_sensor_{nullptr};
  text_sensor::TextSensor *vbus_status_text_sensor_{nullptr};
  text_sensor::TextSensor *status_flags_text_sensor_{nullptr};

  switch_::Switch *charge_enable_switch_{nullptr};
  switch_::Switch *hiz_mode_switch_{nullptr};
  switch_::Switch *otg_mode_switch_{nullptr};
  select::Select *watchdog_select_{nullptr};

  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};
  static constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;

  bool disable_watchdog_{true};
};

class BQ25798ChargeEnableSwitch : public switch_::Switch, public Parented<BQ25798Component> {
protected:
  void write_state(bool state) override;
};

class BQ25798HizModeSwitch : public switch_::Switch, public Parented<BQ25798Component> {
protected:
  void write_state(bool state) override;
};

class BQ25798OtgModeSwitch : public switch_::Switch, public Parented<BQ25798Component> {
protected:
  void write_state(bool state) override;
};

class BQ25798WatchdogSelect : public select::Select, public Parented<BQ25798Component> {
protected:
  void control(size_t index) override;
};

class BQ25798WatchdogResetButton : public button::Button, public Parented<BQ25798Component> {
protected:
  void press_action() override;
};

class BQ25798DumpRegistersButton : public button::Button, public Parented<BQ25798Component> {
protected:
  void press_action() override;
};

} // namespace bq25798
} // namespace esphome
