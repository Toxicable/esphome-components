#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace esc_higher {

struct STM32TempReadResult {
  bool ok{false};
  uint8_t status{0xFF};
  int16_t temp_c{0};
  uint8_t fault{0};
  const char* error_message{"uninitialized"};
};

struct STM32FrameResult {
  bool ok{false};
  const char* error_message{"uninitialized"};
};

class ESCHigherComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  STM32TempReadResult read_stm32_temp_raw();
  STM32FrameResult read_frame_(uint8_t cmd, uint8_t* resp, size_t resp_len);
  void set_temperature_c_sensor(sensor::Sensor* s) {
    temperature_c_sensor_ = s;
  }
  void set_status_sensor(sensor::Sensor* s) {
    status_sensor_ = s;
  }
  void set_fault_sensor(sensor::Sensor* s) {
    fault_sensor_ = s;
  }
  void set_motor_state_sensor(sensor::Sensor* s) {
    motor_state_sensor_ = s;
  }
  void set_current_fault_sensor(sensor::Sensor* s) {
    current_fault_sensor_ = s;
  }
  void set_occurred_fault_sensor(sensor::Sensor* s) {
    occurred_fault_sensor_ = s;
  }
  void set_measured_speed_rpm_sensor(sensor::Sensor* s) {
    measured_speed_rpm_sensor_ = s;
  }
  void set_speed_reference_rpm_sensor(sensor::Sensor* s) {
    speed_reference_rpm_sensor_ = s;
  }
  void set_control_mode_sensor(sensor::Sensor* s) {
    control_mode_sensor_ = s;
  }
  void set_command_state_sensor(sensor::Sensor* s) {
    command_state_sensor_ = s;
  }
  void set_ia_sensor(sensor::Sensor* s) {
    ia_sensor_ = s;
  }
  void set_ib_sensor(sensor::Sensor* s) {
    ib_sensor_ = s;
  }
  void set_phase_current_amplitude_sensor(sensor::Sensor* s) {
    phase_current_amplitude_sensor_ = s;
  }
  void set_iq_sensor(sensor::Sensor* s) {
    iq_sensor_ = s;
  }
  void set_id_sensor(sensor::Sensor* s) {
    id_sensor_ = s;
  }
  void set_iq_ref_sensor(sensor::Sensor* s) {
    iq_ref_sensor_ = s;
  }
  void set_vq_sensor(sensor::Sensor* s) {
    vq_sensor_ = s;
  }
  void set_vd_sensor(sensor::Sensor* s) {
    vd_sensor_ = s;
  }
  void set_phase_voltage_amplitude_sensor(sensor::Sensor* s) {
    phase_voltage_amplitude_sensor_ = s;
  }
  void set_bus_voltage_sensor(sensor::Sensor* s) {
    bus_voltage_sensor_ = s;
  }
  void set_electrical_angle_sensor(sensor::Sensor* s) {
    electrical_angle_sensor_ = s;
  }
  void set_valpha_sensor(sensor::Sensor* s) {
    valpha_sensor_ = s;
  }

 protected:
  static int16_t decode_i16_(uint8_t lsb, uint8_t msb) {
    return static_cast<int16_t>(static_cast<uint16_t>(lsb) | (static_cast<uint16_t>(msb) << 8));
  }
  static uint16_t decode_u16_(uint8_t lsb, uint8_t msb) {
    return static_cast<uint16_t>(lsb) | (static_cast<uint16_t>(msb) << 8);
  }

  sensor::Sensor* temperature_c_sensor_{nullptr};
  sensor::Sensor* status_sensor_{nullptr};
  sensor::Sensor* fault_sensor_{nullptr};
  sensor::Sensor* motor_state_sensor_{nullptr};
  sensor::Sensor* current_fault_sensor_{nullptr};
  sensor::Sensor* occurred_fault_sensor_{nullptr};
  sensor::Sensor* measured_speed_rpm_sensor_{nullptr};
  sensor::Sensor* speed_reference_rpm_sensor_{nullptr};
  sensor::Sensor* control_mode_sensor_{nullptr};
  sensor::Sensor* command_state_sensor_{nullptr};
  sensor::Sensor* ia_sensor_{nullptr};
  sensor::Sensor* ib_sensor_{nullptr};
  sensor::Sensor* phase_current_amplitude_sensor_{nullptr};
  sensor::Sensor* iq_sensor_{nullptr};
  sensor::Sensor* id_sensor_{nullptr};
  sensor::Sensor* iq_ref_sensor_{nullptr};
  sensor::Sensor* vq_sensor_{nullptr};
  sensor::Sensor* vd_sensor_{nullptr};
  sensor::Sensor* phase_voltage_amplitude_sensor_{nullptr};
  sensor::Sensor* bus_voltage_sensor_{nullptr};
  sensor::Sensor* electrical_angle_sensor_{nullptr};
  sensor::Sensor* valpha_sensor_{nullptr};
};

}  // namespace esc_higher
}  // namespace esphome
