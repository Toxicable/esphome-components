#pragma once

#include <cmath>
#include <cstdint>

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/custom_api_device.h"
#endif

// ESP32-C3 uses ESP-IDF; I2C driver is available
#include "driver/i2c.h"

namespace esphome {
namespace mcf8316a {

class MCF8316AComponent : public PollingComponent
#ifdef USE_API
    ,
                          public api::CustomAPIDevice
#endif
{
public:
  MCF8316AComponent(gpio_num_t sda_pin,
                    gpio_num_t scl_pin,
                    uint32_t i2c_freq_hz = 50000,
                    uint8_t i2c_addr_7bit = 0x01,
                    uint32_t poll_ms = 1000,
                    bool crc_enable = false,
                    uint16_t pole_pairs = 0);

  void setup() override;
  void dump_config() override;
  void update() override;

  bool set_speed_percent(float percent);
  void stop();

  float get_last_electrical_hz() const {
    return last_electrical_hz_;
  }
  float get_last_mech_rpm() const {
    return last_mech_rpm_;
  }
  uint32_t get_last_speed_fdbk_raw() const {
    return last_speed_fdbk_raw_;
  }

private:
#ifdef USE_API
  void api_set_speed_percent_(float percent);
  void api_stop_();
#endif

  uint8_t crc8_ccitt_(const uint8_t *data, size_t len);
  void build_cw32_(bool is_read, uint32_t addr22, uint8_t cw[3]);
  bool write_u32_(uint32_t addr22, uint32_t value);
  bool read_u32_(uint32_t addr22, uint32_t *out);

  // ESP32-C3: assume I2C_NUM_0 only
  static constexpr i2c_port_t port_ = I2C_NUM_0;

  gpio_num_t sda_;
  gpio_num_t scl_;
  uint32_t freq_hz_;
  uint8_t addr_;
  bool crc_en_;
  uint16_t pole_pairs_;

  uint16_t max_speed_hz_{0};

  float last_cmd_percent_{0.0f};
  uint32_t last_speed_fdbk_raw_{0};
  float last_electrical_hz_{NAN};
  float last_mech_rpm_{NAN};
};

} // namespace mcf8316a
} // namespace esphome
