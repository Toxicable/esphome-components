#include "makita_xgt.h"

#include "esphome/components/uart/uart_component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace makita_xgt {

static const char* const TAG = "makita_xgt";

void MakitaXGTComponent::set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor) {
  if (index >= this->cell_voltage_sensors_.size()) {
    return;
  }
  this->cell_voltage_sensors_[index] = sensor;
}

void MakitaXGTComponent::setup() {
  this->check_uart_settings(9600);
}

void MakitaXGTComponent::update() {
  if (this->read_all_()) {
    this->publish_();
    this->status_clear_warning();
  } else {
    this->status_set_warning();
  }
}

void MakitaXGTComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Makita XGT Battery:");
  LOG_UPDATE_INTERVAL(this);
  LOG_TEXT_SENSOR("  ", "Model", this->model_text_sensor_);
  LOG_SENSOR("  ", "Charge Count", this->charge_count_sensor_);
  LOG_SENSOR("  ", "Health", this->health_sensor_);
  LOG_SENSOR("  ", "Charge", this->charge_sensor_);
  LOG_SENSOR("  ", "Temperature 1", this->temperature1_sensor_);
  LOG_SENSOR("  ", "Temperature 2", this->temperature2_sensor_);
  LOG_SENSOR("  ", "Lock Status", this->lock_status_sensor_);
  LOG_SENSOR("  ", "Pack Voltage", this->pack_voltage_sensor_);
  LOG_SENSOR("  ", "Cell Size", this->cell_size_sensor_);
  LOG_SENSOR("  ", "Parallel Count", this->parallel_count_sensor_);
  for (uint8_t i = 0; i < this->cell_voltage_sensors_.size(); i++) {
    if (this->cell_voltage_sensors_[i] == nullptr) {
      continue;
    }
    ESP_LOGCONFIG(TAG, "  Cell %u voltage configured", i + 1);
  }
}

bool MakitaXGTComponent::factory_reset() {
  ESP_LOGW(TAG, "Sending battery factory reset sequence");
  if (!this->wake_battery_()) {
    return false;
  }

  uint8_t buffer[FRAME_MAX]{};
  uint8_t rx_length = 0;
  if (this->send_command_("reset_0", RESET_CMD0, sizeof(RESET_CMD0), buffer, rx_length) != ReadStatus::OK) {
    return false;
  }
  delay(10);
  return this->send_command_("reset_1", RESET_CMD1, sizeof(RESET_CMD1), buffer, rx_length) == ReadStatus::OK;
}

bool MakitaXGTComponent::wake_battery_() {
  this->flush_rx_();
  ESP_LOGD(TAG, "Wake: sending 0x00");
  this->write_byte(0x00);
  this->flush();
  delay(WAKE_DELAY_MS);
  return true;
}

void MakitaXGTComponent::log_bytes_(const char* prefix, const uint8_t* data, uint8_t length) const {
  char line[3 * FRAME_MAX + 1];
  size_t pos = 0;
  for (uint8_t i = 0; i < length && pos + 3 < sizeof(line); i++) {
    int written = snprintf(&line[pos], sizeof(line) - pos, "%02X%s", data[i], (i + 1 < length) ? " " : "");
    if (written <= 0) {
      break;
    }
    pos += static_cast<size_t>(written);
  }
  line[sizeof(line) - 1] = '\0';
  ESP_LOGD(TAG, "%s: %s", prefix, line);
}

MakitaXGTComponent::ReadStatus MakitaXGTComponent::send_command_(
  const char* label,
  const uint8_t* command,
  uint8_t command_length,
  uint8_t* buffer,
  uint8_t& rx_length
) {
  rx_length = 0;
  this->flush_rx_();
  ESP_LOGD(TAG, "TX %s", label);
  this->log_bytes_("TX raw", command, command_length);
  this->write_array(command, command_length);
  this->flush();

  if (!this->read_frame_(command, command_length, buffer, rx_length)) {
    if (rx_length > 0) {
      this->log_bytes_("RX partial", buffer, rx_length);
    }
    ESP_LOGW(TAG, "RX timeout for %s (%u bytes after echo)", label, rx_length);
    return ReadStatus::RX_ERROR;
  }

  this->log_bytes_("RX raw", buffer, rx_length);

  for (uint8_t i = 0; i < rx_length; i++) {
    buffer[i] = this->reverse_bits_(buffer[i]);
  }

  this->log_bytes_("RX decoded", buffer, rx_length);

  if (!this->check_crc_(buffer, rx_length)) {
    ESP_LOGW(TAG, "CRC mismatch for %s response", label);
    return ReadStatus::CRC_ERROR;
  }
  ESP_LOGD(TAG, "RX %s CRC OK (%u bytes)", label, rx_length);
  return ReadStatus::OK;
}

bool MakitaXGTComponent::read_frame_(const uint8_t* command, uint8_t command_length, uint8_t* buffer, uint8_t& rx_length) {
  rx_length = 0;
  uint8_t echo_length = 0;
  bool ignored_echo = false;
  uint32_t deadline = millis() + RESPONSE_TIMEOUT_MS;
  while (millis() < deadline && rx_length < FRAME_MAX) {
    while (this->available() > 0 && rx_length < FRAME_MAX) {
      uint8_t byte = 0;
      if (!this->read_byte(&byte)) {
        break;
      }
      deadline = millis() + INTER_BYTE_TIMEOUT_MS;

      if (echo_length < command_length) {
        if (byte == command[echo_length]) {
          echo_length++;
          if (echo_length == command_length) {
            ESP_LOGD(TAG, "Ignoring echoed TX frame (%u bytes)", command_length);
            rx_length = 0;
            echo_length = 0;
            ignored_echo = true;
          }
          continue;
        }

        for (uint8_t i = 0; i < echo_length && rx_length < FRAME_MAX; i++) {
          buffer[rx_length++] = command[i];
        }
        echo_length = command_length;
      }

      buffer[rx_length++] = byte;
    }
    if (rx_length >= 8 && this->available() == 0) {
      return true;
    }
    delay(1);
  }
  if (ignored_echo && rx_length == 0) {
    ESP_LOGD(TAG, "No response bytes arrived after echoed TX frame");
  }
  return rx_length >= 8;
}

uint8_t MakitaXGTComponent::reverse_bits_(uint8_t value) const {
  return static_cast<uint8_t>(
    (BIT_REVERSE_LOOKUP_[value & 0x0F] << 4) | BIT_REVERSE_LOOKUP_[value >> 4]
  );
}

bool MakitaXGTComponent::check_crc_(const uint8_t* buffer, uint8_t length) const {
  if (length < 8) {
    return false;
  }
  if (buffer[0] == 0xCC && buffer[7] == 0x33) {
    uint16_t crc = buffer[0];
    for (uint8_t i = 2; i < length; i++) {
      crc += buffer[i];
    }
    return static_cast<uint8_t>(crc % 256U) == buffer[1];
  }
  if (buffer[0] == 0xA5 && buffer[1] == 0xA5) {
    const uint8_t trimmed_length = static_cast<uint8_t>(length - (buffer[3] & 0x0F));
    if (trimmed_length < 4) {
      return false;
    }
    uint16_t crc = 0;
    for (uint8_t i = 2; i < trimmed_length - 2; i++) {
      crc += buffer[i];
    }
    const uint16_t reported_crc =
      static_cast<uint16_t>((static_cast<uint16_t>(buffer[trimmed_length - 2]) << 8) | buffer[trimmed_length - 1]);
    return crc == reported_crc;
  }
  return false;
}

uint16_t MakitaXGTComponent::decode_u16_swapped_(const uint8_t* buffer) const {
  return static_cast<uint16_t>((static_cast<uint16_t>(buffer[5]) << 8) | buffer[4]);
}

void MakitaXGTComponent::flush_rx_() {
  uint8_t value = 0;
  while (this->available() > 0) {
    this->read_byte(&value);
  }
}

bool MakitaXGTComponent::read_all_() {
  if (!this->wake_battery_()) {
    ESP_LOGW(TAG, "Failed to wake battery");
    return false;
  }

  uint8_t buffer[FRAME_MAX]{};
  uint8_t rx_length = 0;
  ReadStatus status = this->send_command_("model", MODEL_CMD, sizeof(MODEL_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) {
    ESP_LOGW(TAG, "Failed to read model");
    return false;
  }

  uint8_t payload_end = static_cast<uint8_t>(rx_length - ((buffer[3] & 0x0F) + 3));
  this->model_.clear();
  for (uint8_t i = 0; i < 8 && payload_end >= i + 1; i++) {
    const char c = static_cast<char>(buffer[payload_end - i]);
    if (c == '\0' || c == static_cast<char>(0xFF)) {
      continue;
    }
    this->model_.push_back(c);
  }

  status = this->send_command_("charge_count", NUM_CHARGES_CMD, sizeof(NUM_CHARGES_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->charge_count_ = this->decode_u16_swapped_(buffer);

  status = this->send_command_("cell_size", CELL_SIZE_CMD, sizeof(CELL_SIZE_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->cell_size_mah_ = buffer[5];

  status = this->send_command_("parallel_count", PARALLEL_COUNT_CMD, sizeof(PARALLEL_COUNT_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->parallel_count_ = buffer[4];

  status = this->send_command_("health", HEALTH_CMD, sizeof(HEALTH_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  const uint16_t raw_health = this->decode_u16_swapped_(buffer);
  const uint32_t divisor = static_cast<uint32_t>(this->cell_size_mah_) * this->parallel_count_;
  this->health_percent_ = divisor == 0 ? 0 : static_cast<uint16_t>(raw_health / divisor);

  status = this->send_command_("charge", CHARGE_CMD, sizeof(CHARGE_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->charge_percent_ = static_cast<uint16_t>(this->decode_u16_swapped_(buffer) / 255U);

  status = this->send_command_("temperature1", TEMPERATURE1_CMD, sizeof(TEMPERATURE1_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->temperature1_c_ = -30.0f + ((static_cast<float>(this->decode_u16_swapped_(buffer)) - 2431.0f) / 10.0f);

  status = this->send_command_("temperature2", TEMPERATURE2_CMD, sizeof(TEMPERATURE2_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->temperature2_c_ = -30.0f + ((static_cast<float>(this->decode_u16_swapped_(buffer)) - 2431.0f) / 10.0f);

  status = this->send_command_("pack_voltage", PACK_VOLTAGE_CMD, sizeof(PACK_VOLTAGE_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->pack_voltage_v_ = static_cast<float>(this->decode_u16_swapped_(buffer)) / 1000.0f;

  for (uint8_t cell = 0; cell < this->cell_voltages_v_.size(); cell++) {
    uint8_t cell_cmd[8];
    for (uint8_t i = 0; i < sizeof(cell_cmd); i++) {
      cell_cmd[i] = CELL_VOLTAGE_CMD_TEMPLATE[i];
    }
    const uint8_t cell_index = static_cast<uint8_t>(cell + 1);
    const uint8_t value0 = static_cast<uint8_t>(cell_index * 2U);
    const uint8_t value1 = static_cast<uint8_t>(cell_index * 2U + 194U);
    cell_cmd[4] = this->reverse_bits_(value0);
    cell_cmd[1] = this->reverse_bits_(value1);
    char label[16];
    snprintf(label, sizeof(label), "cell_%u", cell_index);
    status = this->send_command_(label, cell_cmd, sizeof(cell_cmd), buffer, rx_length);
    if (status != ReadStatus::OK) return false;
    this->cell_voltages_v_[cell] = static_cast<float>(this->decode_u16_swapped_(buffer)) / 1000.0f;
  }

  status = this->send_command_("lock_status", LOCK_STATUS_CMD, sizeof(LOCK_STATUS_CMD), buffer, rx_length);
  if (status != ReadStatus::OK) return false;
  this->lock_status_ = buffer[4];

  return true;
}

void MakitaXGTComponent::publish_() {
  if (this->model_text_sensor_ != nullptr) {
    this->model_text_sensor_->publish_state(this->model_);
  }
  if (this->charge_count_sensor_ != nullptr) {
    this->charge_count_sensor_->publish_state(this->charge_count_);
  }
  if (this->health_sensor_ != nullptr) {
    this->health_sensor_->publish_state(this->health_percent_);
  }
  if (this->charge_sensor_ != nullptr) {
    this->charge_sensor_->publish_state(this->charge_percent_);
  }
  if (this->temperature1_sensor_ != nullptr) {
    this->temperature1_sensor_->publish_state(this->temperature1_c_);
  }
  if (this->temperature2_sensor_ != nullptr) {
    this->temperature2_sensor_->publish_state(this->temperature2_c_);
  }
  if (this->lock_status_sensor_ != nullptr) {
    this->lock_status_sensor_->publish_state(this->lock_status_);
  }
  if (this->pack_voltage_sensor_ != nullptr) {
    this->pack_voltage_sensor_->publish_state(this->pack_voltage_v_);
  }
  if (this->cell_size_sensor_ != nullptr) {
    this->cell_size_sensor_->publish_state(this->cell_size_mah_);
  }
  if (this->parallel_count_sensor_ != nullptr) {
    this->parallel_count_sensor_->publish_state(this->parallel_count_);
  }
  for (uint8_t i = 0; i < this->cell_voltage_sensors_.size(); i++) {
    if (this->cell_voltage_sensors_[i] != nullptr) {
      this->cell_voltage_sensors_[i]->publish_state(this->cell_voltages_v_[i]);
    }
  }
}

void MakitaXGTResetButton::press_action() {
  if (this->parent_ == nullptr) {
    return;
  }
  if (!this->parent_->factory_reset()) {
    ESP_LOGW(TAG, "Battery reset command failed");
    this->parent_->status_set_warning();
    return;
  }
  this->parent_->status_clear_warning();
  ESP_LOGI(TAG, "Battery accepted reset command");
}

}  // namespace makita_xgt
}  // namespace esphome
