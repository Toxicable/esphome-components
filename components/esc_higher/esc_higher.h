#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace esc_higher {

class ESCHigherComponent;

class ESCHigherStartButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherStopButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherClearFaultsButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherEstopButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherSetSpeedRampButton : public button::Button, public Parented<ESCHigherComponent> {
 public:
  void press_action() override;
};

class ESCHigherComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  bool start_motor();
  bool stop_motor();
  bool clear_faults();
  bool estop();
  bool set_speed_ramp();

  void set_speed_ramp_target_dhz(int32_t v) {
    speed_ramp_target_dhz_ = v;
  }
  void set_speed_ramp_time_ms(int32_t v) {
    speed_ramp_time_ms_ = v;
  }

  void set_proto_major_sensor(sensor::Sensor* s) {
    proto_major_sensor_ = s;
  }
  void set_proto_minor_sensor(sensor::Sensor* s) {
    proto_minor_sensor_ = s;
  }
  void set_fw_major_sensor(sensor::Sensor* s) {
    fw_major_sensor_ = s;
  }
  void set_fw_minor_sensor(sensor::Sensor* s) {
    fw_minor_sensor_ = s;
  }
  void set_hw_id_sensor(sensor::Sensor* s) {
    hw_id_sensor_ = s;
  }
  void set_max_block_len_sensor(sensor::Sensor* s) {
    max_block_len_sensor_ = s;
  }
  void set_capabilities_sensor(sensor::Sensor* s) {
    capabilities_sensor_ = s;
  }

  void set_seq_sensor(sensor::Sensor* s) {
    seq_sensor_ = s;
  }
  void set_esc_state_sensor(sensor::Sensor* s) {
    esc_state_sensor_ = s;
  }
  void set_mc_state_sensor(sensor::Sensor* s) {
    mc_state_sensor_ = s;
  }
  void set_last_cmd_seq_sensor(sensor::Sensor* s) {
    last_cmd_seq_sensor_ = s;
  }
  void set_last_cmd_error_sensor(sensor::Sensor* s) {
    last_cmd_error_sensor_ = s;
  }
  void set_current_faults_sensor(sensor::Sensor* s) {
    current_faults_sensor_ = s;
  }
  void set_occurred_faults_sensor(sensor::Sensor* s) {
    occurred_faults_sensor_ = s;
  }
  void set_status_flags_sensor(sensor::Sensor* s) {
    status_flags_sensor_ = s;
  }
  void set_watchdog_ms_left_sensor(sensor::Sensor* s) {
    watchdog_ms_left_sensor_ = s;
  }

  void set_vbus_mv_sensor(sensor::Sensor* s) {
    vbus_mv_sensor_ = s;
  }
  void set_ibus_ma_sensor(sensor::Sensor* s) {
    ibus_ma_sensor_ = s;
  }
  void set_speed_dhz_sensor(sensor::Sensor* s) {
    speed_dhz_sensor_ = s;
  }
  void set_duty_centi_pct_sensor(sensor::Sensor* s) {
    duty_centi_pct_sensor_ = s;
  }
  void set_temp_mc_sensor(sensor::Sensor* s) {
    temp_mc_sensor_ = s;
  }
  void set_uptime_s_sensor(sensor::Sensor* s) {
    uptime_s_sensor_ = s;
  }
  void set_esc_state_text_sensor(text_sensor::TextSensor* s) {
    esc_state_text_sensor_ = s;
  }
  void set_last_cmd_error_text_sensor(text_sensor::TextSensor* s) {
    last_cmd_error_text_sensor_ = s;
  }
  void set_status_flags_text_sensor(text_sensor::TextSensor* s) {
    status_flags_text_sensor_ = s;
  }
  void set_current_faults_text_sensor(text_sensor::TextSensor* s) {
    current_faults_text_sensor_ = s;
  }
  void set_occurred_faults_text_sensor(text_sensor::TextSensor* s) {
    occurred_faults_text_sensor_ = s;
  }
  void set_capabilities_text_sensor(text_sensor::TextSensor* s) {
    capabilities_text_sensor_ = s;
  }

 protected:
  bool read_register_(uint8_t reg, uint8_t* out, size_t len);
  bool write_command_(uint8_t opcode, int32_t param0, int32_t param1, int32_t param2);

  static uint16_t u16_(const uint8_t* b, size_t off) {
    return static_cast<uint16_t>(b[off]) | (static_cast<uint16_t>(b[off + 1]) << 8);
  }
  static int32_t i32_(const uint8_t* b, size_t off) {
    return static_cast<int32_t>(
      static_cast<uint32_t>(b[off]) | (static_cast<uint32_t>(b[off + 1]) << 8) |
      (static_cast<uint32_t>(b[off + 2]) << 16) | (static_cast<uint32_t>(b[off + 3]) << 24)
    );
  }
  static uint32_t u32_(const uint8_t* b, size_t off) {
    return static_cast<uint32_t>(b[off]) | (static_cast<uint32_t>(b[off + 1]) << 8) |
           (static_cast<uint32_t>(b[off + 2]) << 16) |
           (static_cast<uint32_t>(b[off + 3]) << 24);
  }
  static int16_t i16_(const uint8_t* b, size_t off) {
    return static_cast<int16_t>(u16_(b, off));
  }
  static const char* esc_state_to_cstr_(uint8_t v);
  static const char* last_cmd_error_to_cstr_(uint8_t v);
  static std::string bitmask_to_names_(uint16_t v, const char* const* names, size_t count);

  static constexpr uint8_t REG_ID = 0x00;
  static constexpr uint8_t REG_STATUS = 0x10;
  static constexpr uint8_t REG_COMMAND = 0x20;
  static constexpr uint8_t REG_TELEMETRY = 0x30;

  static constexpr uint8_t OPCODE_START = 0x01;
  static constexpr uint8_t OPCODE_STOP = 0x02;
  static constexpr uint8_t OPCODE_CLEAR_FAULTS = 0x03;
  static constexpr uint8_t OPCODE_SET_SPEED_RAMP = 0x04;
  static constexpr uint8_t OPCODE_ESTOP = 0x05;

  uint8_t command_seq_{0};
  int32_t speed_ramp_target_dhz_{1000};
  int32_t speed_ramp_time_ms_{1000};

  sensor::Sensor* proto_major_sensor_{nullptr};
  sensor::Sensor* proto_minor_sensor_{nullptr};
  sensor::Sensor* fw_major_sensor_{nullptr};
  sensor::Sensor* fw_minor_sensor_{nullptr};
  sensor::Sensor* hw_id_sensor_{nullptr};
  sensor::Sensor* max_block_len_sensor_{nullptr};
  sensor::Sensor* capabilities_sensor_{nullptr};

  sensor::Sensor* seq_sensor_{nullptr};
  sensor::Sensor* esc_state_sensor_{nullptr};
  sensor::Sensor* mc_state_sensor_{nullptr};
  sensor::Sensor* last_cmd_seq_sensor_{nullptr};
  sensor::Sensor* last_cmd_error_sensor_{nullptr};
  sensor::Sensor* current_faults_sensor_{nullptr};
  sensor::Sensor* occurred_faults_sensor_{nullptr};
  sensor::Sensor* status_flags_sensor_{nullptr};
  sensor::Sensor* watchdog_ms_left_sensor_{nullptr};

  sensor::Sensor* vbus_mv_sensor_{nullptr};
  sensor::Sensor* ibus_ma_sensor_{nullptr};
  sensor::Sensor* speed_dhz_sensor_{nullptr};
  sensor::Sensor* duty_centi_pct_sensor_{nullptr};
  sensor::Sensor* temp_mc_sensor_{nullptr};
  sensor::Sensor* uptime_s_sensor_{nullptr};
  text_sensor::TextSensor* esc_state_text_sensor_{nullptr};
  text_sensor::TextSensor* last_cmd_error_text_sensor_{nullptr};
  text_sensor::TextSensor* status_flags_text_sensor_{nullptr};
  text_sensor::TextSensor* current_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* occurred_faults_text_sensor_{nullptr};
  text_sensor::TextSensor* capabilities_text_sensor_{nullptr};
};

}  // namespace esc_higher
}  // namespace esphome
