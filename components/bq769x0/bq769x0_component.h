#pragma once

#include <array>

#include "bq769x0.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

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
  void set_soc_sensor(sensor::Sensor *sensor) { soc_sensor_ = sensor; }
  void set_capacity_mah(float capacity_mah) { capacity_mah_ = capacity_mah; }
  void set_initial_soc(float initial_soc) { initial_soc_ = initial_soc; }

  void set_fault_sensor(binary_sensor::BinarySensor *sensor) { fault_sensor_ = sensor; }
  void set_device_ready_sensor(binary_sensor::BinarySensor *sensor) { device_ready_sensor_ = sensor; }
  void set_cc_ready_sensor(binary_sensor::BinarySensor *sensor) { cc_ready_sensor_ = sensor; }

  void set_mode_sensor(text_sensor::TextSensor *sensor) { mode_sensor_ = sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;

  void clear_faults();
  void trigger_cc_oneshot();

protected:
  void update_soc_(float current_ma);
  bool ensure_cc_enabled_();
  bool read_sys_ctrl2_(uint8_t &value);
  bool write_sys_ctrl2_(uint8_t value);
  bool read_cell_block_(std::array<uint8_t, 10> &buffer, size_t count);
  void publish_cell_block_(const std::array<uint8_t, 10> &buffer, size_t count);

  bool check_i2c_(bool ok, const char *operation);

  uint8_t cell_count_{0};
  bool crc_enabled_{false};
  int rsense_milliohm_{0};

  BQ769X0Driver driver_;
  Cal cal_{};
  bool cal_valid_{false};

  sensor::Sensor *pack_voltage_sensor_{nullptr};
  std::array<sensor::Sensor *, 5> cell_sensors_{};
  sensor::Sensor *board_temp_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *soc_sensor_{nullptr};

  binary_sensor::BinarySensor *fault_sensor_{nullptr};
  binary_sensor::BinarySensor *device_ready_sensor_{nullptr};
  binary_sensor::BinarySensor *cc_ready_sensor_{nullptr};

  text_sensor::TextSensor *mode_sensor_{nullptr};

  float capacity_mah_{0.0f};
  float initial_soc_{100.0f};
  float soc_percent_{NAN};
  uint32_t last_soc_ms_{0};
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

} // namespace bq769x0
} // namespace esphome
