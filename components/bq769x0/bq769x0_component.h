#pragma once

#include <array>
#include <limits>
#include <vector>

#include "bq769x0.h"
#include "bq769x0_soc.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/select/select.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace bq769x0 {

enum class Chemistry : uint8_t {
  LIION_LIPO = 0,
};

class BQ769X0Component : public PollingComponent, public i2c::I2CDevice {
public:
  BQ769X0Component();

  void set_cell_count(uint8_t cell_count) { cell_count_ = cell_count; }
  void set_chemistry(Chemistry chemistry) { chemistry_ = chemistry; }

  void set_pack_voltage_sensor(sensor::Sensor *sensor) { pack_voltage_sensor_ = sensor; }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor);
  void set_board_temp_sensor(sensor::Sensor *sensor) { board_temp_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
  void set_soc_percent_sensor(sensor::Sensor *sensor) { soc_percent_sensor_ = sensor; }
  void set_min_cell_sensor(sensor::Sensor *sensor) { min_cell_sensor_ = sensor; }
  void set_avg_cell_sensor(sensor::Sensor *sensor) { avg_cell_sensor_ = sensor; }

  void set_alerts_sensor(text_sensor::TextSensor *sensor) { alerts_sensor_ = sensor; }

  void set_power_path_select(select::Select *select) { power_path_select_ = select; }
  void set_power_path_state_sensor(text_sensor::TextSensor *sensor) { power_path_state_sensor_ = sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;

  void clear_faults();
  bool set_fet_state(bool chg_on, bool dsg_on);

protected:
  struct PersistedState {
    float soc_percent{std::numeric_limits<float>::quiet_NaN()};
    float capacity_mah{std::numeric_limits<float>::quiet_NaN()};
    float q_remaining_mah{std::numeric_limits<float>::quiet_NaN()};
    bool soc_valid{false};
  };

  void update_soc_(const SocInputs &inputs);
  void publish_soc_outputs_(const SocOutputs &outputs);
  void load_preferences_();
  void save_preferences_();

  bool ensure_cc_enabled_();
  bool read_sys_ctrl2_(uint8_t &value);
  bool write_sys_ctrl2_(uint8_t value);
  bool read_cell_block_(std::array<uint8_t, 10> &buffer, size_t count);

  bool check_i2c_(bool ok, const char *operation);

  uint8_t cell_count_{0};
  Chemistry chemistry_{Chemistry::LIION_LIPO};
  bool crc_enabled_{false};
  int rsense_milliohm_{2};

  BQ769X0Driver driver_;
  Cal cal_{};
  bool cal_valid_{false};

  sensor::Sensor *pack_voltage_sensor_{nullptr};
  std::array<sensor::Sensor *, 4> cell_sensors_{};
  sensor::Sensor *board_temp_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *soc_percent_sensor_{nullptr};
  sensor::Sensor *min_cell_sensor_{nullptr};
  sensor::Sensor *avg_cell_sensor_{nullptr};

  text_sensor::TextSensor *alerts_sensor_{nullptr};

  select::Select *power_path_select_{nullptr};
  text_sensor::TextSensor *power_path_state_sensor_{nullptr};

  SocConfig soc_cfg_{};
  BQ769X0SocEstimator soc_estimator_{};

  uint32_t last_cc_ms_{0};
  uint32_t last_persist_ms_{0};
  float last_vmin_mv_{std::numeric_limits<float>::quiet_NaN()};
  float last_vavg_mv_{std::numeric_limits<float>::quiet_NaN()};

  decltype(global_preferences->make_preference<PersistedState>(0)) pref_{};
};

class BQ769X0PowerPathSelect : public select::Select, public Parented<BQ769X0Component> {
protected:
  void control(size_t index) override;
};

class BQ769X0ClearFaultsButton : public button::Button {
public:
  void set_parent(BQ769X0Component *parent) { parent_ = parent; }

protected:
  void press_action() override;

  BQ769X0Component *parent_{nullptr};
};

} // namespace bq769x0
} // namespace esphome
