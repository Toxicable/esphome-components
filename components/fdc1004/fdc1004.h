#pragma once

#include <array>
#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace fdc1004 {

class FDC1004Component : public PollingComponent, public i2c::I2CDevice {
public:
  void set_channel_sensor(uint8_t index, sensor::Sensor *sensor);
  void set_channel_capdac(uint8_t index, uint8_t capdac_steps);
  void set_sample_rate(uint16_t sample_rate_sps);
  void tare_to_current();

  void setup() override;
  void update() override;
  void dump_config() override;

protected:
  bool initialize_();
  bool read_register16_(uint8_t reg, uint16_t &value);
  bool write_register16_(uint8_t reg, uint16_t value);
  bool configure_measurement_(uint8_t measurement_index);
  bool write_fdc_conf_();
  bool read_measurement_pf_(uint8_t measurement_index, float &capacitance_pf);
  float capdac_pf_(uint8_t measurement_index) const;

  std::array<sensor::Sensor *, 4> channel_sensors_{{nullptr, nullptr, nullptr, nullptr}};
  std::array<uint8_t, 4> capdac_steps_{{0, 0, 0, 0}};
  std::array<float, 4> tare_offsets_pf_{{0.0f, 0.0f, 0.0f, 0.0f}};
  uint8_t enabled_mask_{0};
  uint16_t sample_rate_sps_{100};
  uint8_t rate_bits_{0b01};
  bool initialized_{false};
  uint32_t next_init_retry_ms_{0};
  static constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;
};

class FDC1004ZeroButton : public button::Button {
public:
  void set_parent(FDC1004Component *parent) { parent_ = parent; }

protected:
  void press_action() override;
  FDC1004Component *parent_{nullptr};
};

} // namespace fdc1004
} // namespace esphome
