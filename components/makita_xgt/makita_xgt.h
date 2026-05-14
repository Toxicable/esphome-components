#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "esphome/components/button/button.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace makita_xgt {

class MakitaXGTComponent : public PollingComponent, public uart::UARTDevice {
 public:
  void set_model_text_sensor(text_sensor::TextSensor* sensor) { model_text_sensor_ = sensor; }
  void set_charge_count_sensor(sensor::Sensor* sensor) { charge_count_sensor_ = sensor; }
  void set_health_sensor(sensor::Sensor* sensor) { health_sensor_ = sensor; }
  void set_charge_sensor(sensor::Sensor* sensor) { charge_sensor_ = sensor; }
  void set_temperature1_sensor(sensor::Sensor* sensor) { temperature1_sensor_ = sensor; }
  void set_temperature2_sensor(sensor::Sensor* sensor) { temperature2_sensor_ = sensor; }
  void set_lock_status_sensor(sensor::Sensor* sensor) { lock_status_sensor_ = sensor; }
  void set_pack_voltage_sensor(sensor::Sensor* sensor) { pack_voltage_sensor_ = sensor; }
  void set_cell_size_sensor(sensor::Sensor* sensor) { cell_size_sensor_ = sensor; }
  void set_parallel_count_sensor(sensor::Sensor* sensor) { parallel_count_sensor_ = sensor; }
  void set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor);

  void setup() override;
  void update() override;
  void dump_config() override;

  bool factory_reset();

 protected:
  enum class ReadStatus : int8_t {
    OK = 0,
    RX_ERROR = 1,
    CRC_ERROR = -1,
    FORMAT_ERROR = -2,
  };

  bool wake_battery_();
  ReadStatus send_command_(
    const char* label,
    const uint8_t* command,
    uint8_t command_length,
    uint8_t* buffer,
    uint8_t& rx_length
  );
  void log_bytes_(const char* prefix, const uint8_t* data, uint8_t length) const;
  bool read_frame_(const uint8_t* command, uint8_t command_length, uint8_t* buffer, uint8_t& rx_length);
  uint8_t reverse_bits_(uint8_t value) const;
  bool check_crc_(const uint8_t* buffer, uint8_t length) const;
  bool read_all_();
  uint16_t decode_u16_swapped_(const uint8_t* buffer) const;
  void flush_rx_();
  void publish_();

  text_sensor::TextSensor* model_text_sensor_{nullptr};
  sensor::Sensor* charge_count_sensor_{nullptr};
  sensor::Sensor* health_sensor_{nullptr};
  sensor::Sensor* charge_sensor_{nullptr};
  sensor::Sensor* temperature1_sensor_{nullptr};
  sensor::Sensor* temperature2_sensor_{nullptr};
  sensor::Sensor* lock_status_sensor_{nullptr};
  sensor::Sensor* pack_voltage_sensor_{nullptr};
  sensor::Sensor* cell_size_sensor_{nullptr};
  sensor::Sensor* parallel_count_sensor_{nullptr};
  std::array<sensor::Sensor*, 10> cell_voltage_sensors_{{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};

  std::string model_;
  uint16_t charge_count_{0};
  uint16_t health_percent_{0};
  uint16_t charge_percent_{0};
  float temperature1_c_{0.0f};
  float temperature2_c_{0.0f};
  uint8_t lock_status_{0};
  float pack_voltage_v_{0.0f};
  uint16_t cell_size_mah_{0};
  uint16_t parallel_count_{0};
  std::array<float, 10> cell_voltages_v_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

  static constexpr uint8_t MODEL_CMD[32] = {
    0xA5, 0xA5, 0x00, 0x58, 0x0A, 0xD4, 0xB2, 0x32,
    0x00, 0xD3, 0xC8, 0xE0, 0x00, 0x60, 0x00, 0xC0,
    0x00, 0x80, 0xC8, 0xD0, 0x40, 0xDC, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };
  static constexpr uint8_t NUM_CHARGES_CMD[8] = {0x33, 0xC8, 0x03, 0x00, 0x2A, 0x00, 0x00, 0xCC};
  static constexpr uint8_t CELL_SIZE_CMD[8] = {0x33, 0x27, 0xBB, 0x10, 0x00, 0x00, 0x00, 0xCC};
  static constexpr uint8_t PARALLEL_COUNT_CMD[8] = {0x33, 0x67, 0xBB, 0x50, 0x00, 0x00, 0x00, 0xCC};
  static constexpr uint8_t HEALTH_CMD[8] = {0x33, 0xC4, 0x03, 0x00, 0x26, 0x00, 0x00, 0xCC};
  static constexpr uint8_t CHARGE_CMD[8] = {0x33, 0x13, 0x03, 0x80, 0x10, 0x00, 0x00, 0xCC};
  static constexpr uint8_t TEMPERATURE1_CMD[8] = {0x33, 0x3B, 0x03, 0xC0, 0x58, 0x00, 0x00, 0xCC};
  static constexpr uint8_t TEMPERATURE2_CMD[8] = {0x33, 0x7B, 0x03, 0xC0, 0x38, 0x00, 0x00, 0xCC};
  static constexpr uint8_t PACK_VOLTAGE_CMD[8] = {0x33, 0x43, 0x03, 0xC0, 0x00, 0x00, 0x00, 0xCC};
  static constexpr uint8_t LOCK_STATUS_CMD[8] = {0x33, 0xF8, 0x03, 0x00, 0x06, 0x00, 0x00, 0xCC};
  static constexpr uint8_t RESET_CMD0[8] = {0x33, 0xC8, 0x9B, 0x69, 0xA5, 0x00, 0x00, 0xCC};
  static constexpr uint8_t RESET_CMD1[8] = {0x33, 0x00, 0x4B, 0xF4, 0x00, 0x00, 0x00, 0xCC};
  static constexpr uint8_t CELL_VOLTAGE_CMD_TEMPLATE[8] = {0x33, 0x23, 0x03, 0xC0, 0x00, 0x00, 0x00, 0xCC};
  static constexpr std::array<uint8_t, 16> BIT_REVERSE_LOOKUP_{{0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF}};
  static constexpr uint8_t FRAME_MAX = 32;
  static constexpr uint32_t WAKE_DELAY_MS = 70;
  static constexpr uint32_t RESPONSE_TIMEOUT_MS = 80;
  static constexpr uint32_t INTER_BYTE_TIMEOUT_MS = 5;
  static constexpr uint32_t RETRY_DELAY_MS = 50;
};

class MakitaXGTResetButton : public button::Button {
 public:
  void set_parent(MakitaXGTComponent* parent) { parent_ = parent; }

 protected:
  void press_action() override;
  MakitaXGTComponent* parent_{nullptr};
};

}  // namespace makita_xgt
}  // namespace esphome
