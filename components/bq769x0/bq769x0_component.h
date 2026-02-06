#pragma once

#include <array>
#include <vector>

#include "bq769x0.h"
#include "bq769x0_soc.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/gpio/gpio.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace bq769x0 {

class BQ769X0Component : public PollingComponent, public i2c::I2CDevice {
public:
  BQ769X0Component();

  void set_cell_count(uint8_t cell_count) { cell_count_ = cell_count; }
  void set_crc_enabled(bool enabled) { crc_enabled_ = enabled; }
  void set_rsense_milliohm(int rsense_milliohm) { rsense_milliohm_ = rsense_milliohm; }

  void set_pack_voltage_sensor(sensor::Sensor *sensor) { pack_voltage_sensor_ = sensor; }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor);
  void set_board_temp_sensor(sensor::Sensor *sensor) { board_temp_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
  void set_soc_percent_sensor(sensor::Sensor *sensor) { soc_percent_sensor_ = sensor; }
  void set_min_cell_sensor(sensor::Sensor *sensor) { min_cell_sensor_ = sensor; }
  void set_avg_cell_sensor(sensor::Sensor *sensor) { avg_cell_sensor_ = sensor; }
  void set_soc_confidence_sensor(sensor::Sensor *sensor) { soc_confidence_sensor_ = sensor; }
  void set_rest_state_sensor(text_sensor::TextSensor *sensor) { rest_state_sensor_ = sensor; }

  void set_fault_sensor(binary_sensor::BinarySensor *sensor) { fault_sensor_ = sensor; }
  void set_device_ready_sensor(binary_sensor::BinarySensor *sensor) { device_ready_sensor_ = sensor; }
  void set_cc_ready_sensor(binary_sensor::BinarySensor *sensor) { cc_ready_sensor_ = sensor; }
  void set_soc_valid_sensor(binary_sensor::BinarySensor *sensor) { soc_valid_sensor_ = sensor; }

  void set_mode_sensor(text_sensor::TextSensor *sensor) { mode_sensor_ = sensor; }

  void set_ocv_source(bool use_min_cell) { soc_cfg_.use_min_cell = use_min_cell; }
  void set_rest_current_threshold_ma(float value) { soc_cfg_.rest_current_threshold_ma = value; }
  void set_rest_min_seconds(float value) { soc_cfg_.rest_min_seconds = value; }
  void set_rest_full_weight_seconds(float value) { soc_cfg_.rest_full_weight_seconds = value; }
  void set_rest_dvdt_threshold_mv_per_s(float value) { soc_cfg_.rest_dvdt_threshold_mv_per_s = value; }
  void set_full_cell_mv(float value) { soc_cfg_.full_cell_mv = value; }
  void set_full_hold_seconds(float value) { soc_cfg_.full_hold_seconds = value; }
  void set_empty_cell_mv(float value) { soc_cfg_.empty_cell_mv = value; }
  void set_empty_hold_seconds(float value) { soc_cfg_.empty_hold_seconds = value; }
  void set_empty_discharge_current_ma(float value) { soc_cfg_.empty_discharge_current_ma = value; }
  void set_current_positive_is_discharge(bool value) { soc_cfg_.current_positive_is_discharge = value; }
  void set_coulombic_eff_discharge(float value) { soc_cfg_.coulombic_eff_discharge = value; }
  void set_coulombic_eff_charge(float value) { soc_cfg_.coulombic_eff_charge = value; }
  void set_learn_alpha(float value) { soc_cfg_.learn_alpha = value; }
  void set_use_hw_fault_anchors(bool value) { soc_cfg_.use_hw_fault_anchors = value; }

  void set_alert_pin(GPIOPin *pin) { alert_pin_ = pin; }

  void set_balance_enabled(bool enabled) { soc_cfg_.balance.enabled = enabled; }
  void set_balance_current_ma_per_cell(float value) { soc_cfg_.balance.balance_current_ma_per_cell = value; }
  void set_balance_duty(float value) { soc_cfg_.balance.balance_duty = value; }

  void add_ocv_point(int mv, float soc);

  void setup() override;
  void update() override;
  void dump_config() override;

  void clear_faults();
  void trigger_cc_oneshot();
  void force_full_anchor();
  void force_empty_anchor();
  void clear_learned_capacity();

protected:
  struct PersistedState {
    float soc_percent{NAN};
    float capacity_mah{NAN};
    float q_remaining_mah{NAN};
    bool soc_valid{false};
  };

  void update_soc_(const SocInputs &inputs);
  void publish_soc_outputs_(const SocOutputs &outputs);
  void load_preferences_();
  void save_preferences_();
  void handle_force_full_anchor_();
  void handle_force_empty_anchor_();
  void handle_clear_capacity_();

  bool ensure_cc_enabled_();
  bool read_sys_ctrl2_(uint8_t &value);
  bool write_sys_ctrl2_(uint8_t value);
  bool read_cell_block_(std::array<uint8_t, 10> &buffer, size_t count);
  void publish_cell_block_(const std::array<uint8_t, 10> &buffer, size_t count);

  bool check_i2c_(bool ok, const char *operation);

  uint8_t cell_count_{0};
  bool crc_enabled_{false};
  int rsense_milliohm_{0};

  GPIOPin *alert_pin_{nullptr};

  BQ769X0Driver driver_;
  Cal cal_{};
  bool cal_valid_{false};

  sensor::Sensor *pack_voltage_sensor_{nullptr};
  std::array<sensor::Sensor *, 5> cell_sensors_{};
  sensor::Sensor *board_temp_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *soc_percent_sensor_{nullptr};
  sensor::Sensor *min_cell_sensor_{nullptr};
  sensor::Sensor *avg_cell_sensor_{nullptr};
  sensor::Sensor *soc_confidence_sensor_{nullptr};
  text_sensor::TextSensor *rest_state_sensor_{nullptr};

  binary_sensor::BinarySensor *fault_sensor_{nullptr};
  binary_sensor::BinarySensor *device_ready_sensor_{nullptr};
  binary_sensor::BinarySensor *cc_ready_sensor_{nullptr};
  binary_sensor::BinarySensor *soc_valid_sensor_{nullptr};

  text_sensor::TextSensor *mode_sensor_{nullptr};

  SocConfig soc_cfg_{};
  BQ769X0SocEstimator soc_estimator_{};

  uint32_t last_cc_ms_{0};
  uint32_t last_persist_ms_{0};
  float last_vmin_mv_{NAN};

  preferences::PreferenceObject pref_{};
};

class BQ769X0ClearFaultsButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

class BQ769X0CCOneshotButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

class BQ769X0ForceFullAnchorButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

class BQ769X0ForceEmptyAnchorButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

class BQ769X0ClearCapacityButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

} // namespace bq769x0
} // namespace esphome
