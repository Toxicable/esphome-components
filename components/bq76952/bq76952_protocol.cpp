#include "bq76952_protocol.h"

#include <array>
#include <cstring>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char *const TAG = "bq76952.protocol";

constexpr uint8_t REG_SUBCOMMAND = 0x3E;
constexpr uint8_t REG_TRANSFER_BUFFER = 0x40;
constexpr uint8_t REG_CHECKSUM = 0x60;
constexpr uint8_t REG_LENGTH = 0x61;
constexpr uint8_t REG_BATTERY_STATUS = 0x12;

constexpr uint16_t SUBCMD_SET_CFGUPDATE = 0x0090;
constexpr uint16_t SUBCMD_EXIT_CFGUPDATE = 0x0092;
constexpr uint16_t BATTERY_STATUS_CFGUPDATE = 1U << 0;

constexpr size_t MAX_TRANSFER_PAYLOAD = 32;

uint8_t crc8(const uint8_t *data, size_t length) {
  uint8_t crc = 0;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80U) != 0 ? static_cast<uint8_t>((crc << 1) ^ 0x07U) : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

uint8_t transfer_checksum(uint16_t command, const uint8_t *data, size_t length) {
  uint16_t sum = static_cast<uint16_t>(command & 0xFFU) + static_cast<uint16_t>((command >> 8) & 0xFFU);
  for (size_t i = 0; i < length; i++) {
    sum += data[i];
  }
  return static_cast<uint8_t>(~(sum & 0xFFU));
}
}  // namespace

bool BQ76952Protocol::read_u8(uint8_t command, uint8_t &value) {
  return this->read_bytes(command, &value, 1);
}

bool BQ76952Protocol::read_i16(uint8_t command, int16_t &value) {
  uint16_t raw = 0;
  if (!this->read_u16(command, raw)) {
    return false;
  }
  value = static_cast<int16_t>(raw);
  return true;
}

bool BQ76952Protocol::read_u16(uint8_t command, uint16_t &value) {
  uint8_t raw[2]{};
  if (!this->read_bytes(command, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>(raw[0]) | (static_cast<uint16_t>(raw[1]) << 8);
  return true;
}

bool BQ76952Protocol::read_bytes(uint8_t command, uint8_t *data, size_t length) {
  if (length == 0) {
    return true;
  }
  if (data == nullptr) {
    return false;
  }

  if (!this->crc_enabled_) {
    if (this->write_read(&command, 1, data, length) == i2c::ERROR_OK) {
      return true;
    }
    if (this->write(&command, 1) != i2c::ERROR_OK) {
      return false;
    }
    return this->read(data, length) == i2c::ERROR_OK;
  }

  if (length > MAX_TRANSFER_PAYLOAD) {
    ESP_LOGE(TAG, "CRC read length %u exceeds supported maximum", static_cast<unsigned>(length));
    return false;
  }

  std::array<uint8_t, MAX_TRANSFER_PAYLOAD * 2> framed{};
  if (this->write_read(&command, 1, framed.data(), length * 2) != i2c::ERROR_OK) {
    return false;
  }

  const uint8_t write_address = static_cast<uint8_t>(this->address_ << 1);
  const uint8_t read_address = static_cast<uint8_t>(write_address | 1U);
  for (size_t i = 0; i < length; i++) {
    const uint8_t value = framed[i * 2];
    const uint8_t received_crc = framed[i * 2 + 1];
    uint8_t expected_crc = 0;
    if (i == 0) {
      const uint8_t crc_data[4] = {write_address, command, read_address, value};
      expected_crc = crc8(crc_data, sizeof(crc_data));
    } else {
      expected_crc = crc8(&value, 1);
    }
    if (received_crc != expected_crc) {
      ESP_LOGW(TAG, "CRC mismatch reading register 0x%02X byte %u: got=0x%02X expected=0x%02X", command,
               static_cast<unsigned>(i), received_crc, expected_crc);
      return false;
    }
    data[i] = value;
  }
  return true;
}

bool BQ76952Protocol::write_u8(uint8_t command, uint8_t value) {
  return this->write_bytes(command, &value, 1);
}

bool BQ76952Protocol::write_u16(uint8_t command, uint16_t value) {
  const uint8_t raw[2] = {static_cast<uint8_t>(value & 0xFFU), static_cast<uint8_t>((value >> 8) & 0xFFU)};
  return this->write_bytes(command, raw, sizeof(raw));
}

bool BQ76952Protocol::write_bytes(uint8_t command, const uint8_t *data, size_t length) {
  if (length > 0 && data == nullptr) {
    return false;
  }

  if (!this->crc_enabled_) {
    return i2c::I2CDevice::write_bytes(command, data, static_cast<uint8_t>(length));
  }

  if (length > MAX_TRANSFER_PAYLOAD) {
    ESP_LOGE(TAG, "CRC write length %u exceeds supported maximum", static_cast<unsigned>(length));
    return false;
  }

  std::array<uint8_t, 1 + MAX_TRANSFER_PAYLOAD * 2> framed{};
  framed[0] = command;
  const uint8_t write_address = static_cast<uint8_t>(this->address_ << 1);
  for (size_t i = 0; i < length; i++) {
    framed[1 + i * 2] = data[i];
    if (i == 0) {
      const uint8_t crc_data[3] = {write_address, command, data[i]};
      framed[2 + i * 2] = crc8(crc_data, sizeof(crc_data));
    } else {
      framed[2 + i * 2] = crc8(&data[i], 1);
    }
  }
  return this->write(framed.data(), 1 + length * 2) == i2c::ERROR_OK;
}

bool BQ76952Protocol::send_subcommand(uint16_t subcommand) {
  return this->write_u16(REG_SUBCOMMAND, subcommand);
}

bool BQ76952Protocol::wait_for_transfer_buffer(uint16_t expected_command, uint32_t timeout_ms) {
  delay_microseconds_safe(2500);
  const uint32_t started = millis();
  while ((millis() - started) <= timeout_ms) {
    uint16_t echo = 0;
    if (!this->read_u16(REG_SUBCOMMAND, echo)) {
      return false;
    }
    if (echo == expected_command) {
      return true;
    }
    delay_microseconds_safe(500);
  }
  ESP_LOGW(TAG, "Timed out waiting for transfer buffer command 0x%04X", expected_command);
  return false;
}

bool BQ76952Protocol::verify_transfer_buffer(uint16_t command, const uint8_t *data, size_t length,
                                             uint8_t checksum) const {
  return transfer_checksum(command, data, length) == checksum;
}

bool BQ76952Protocol::read_transfer_buffer(uint16_t expected_command, uint8_t *data, size_t length) {
  uint8_t response_length = 0;
  if (!this->read_u8(REG_LENGTH, response_length) || response_length < 4) {
    return false;
  }

  const size_t payload_length = static_cast<size_t>(response_length - 4);
  if (payload_length > MAX_TRANSFER_PAYLOAD || length > payload_length) {
    ESP_LOGW(TAG, "Unexpected transfer payload length %u for command 0x%04X", static_cast<unsigned>(payload_length),
             expected_command);
    return false;
  }

  std::array<uint8_t, MAX_TRANSFER_PAYLOAD> payload{};
  if (payload_length > 0 && !this->read_bytes(REG_TRANSFER_BUFFER, payload.data(), payload_length)) {
    return false;
  }

  uint8_t checksum = 0;
  if (!this->read_u8(REG_CHECKSUM, checksum) ||
      !this->verify_transfer_buffer(expected_command, payload.data(), payload_length, checksum)) {
    ESP_LOGW(TAG, "Transfer-buffer checksum failed for command 0x%04X", expected_command);
    return false;
  }

  if (length > 0) {
    std::memcpy(data, payload.data(), length);
  }
  return true;
}

bool BQ76952Protocol::read_subcommand(uint16_t subcommand, uint8_t *data, size_t length) {
  if (!this->send_subcommand(subcommand) || !this->wait_for_transfer_buffer(subcommand, 100)) {
    return false;
  }
  return this->read_transfer_buffer(subcommand, data, length);
}

bool BQ76952Protocol::write_subcommand(uint16_t subcommand, const uint8_t *data, size_t length) {
  if (length > MAX_TRANSFER_PAYLOAD || (length > 0 && data == nullptr)) {
    return false;
  }
  if (!this->send_subcommand(subcommand)) {
    return false;
  }
  if (length > 0 && !this->write_bytes(REG_TRANSFER_BUFFER, data, length)) {
    return false;
  }
  const uint8_t footer[2] = {transfer_checksum(subcommand, data, length), static_cast<uint8_t>(length + 4)};
  return this->write_bytes(REG_CHECKSUM, footer, sizeof(footer));
}

bool BQ76952Protocol::read_data_memory(uint16_t address, uint8_t *data, size_t length) {
  return this->read_subcommand(address, data, length);
}

bool BQ76952Protocol::write_data_memory(uint16_t address, const uint8_t *data, size_t length) {
  if (!this->write_subcommand(address, data, length)) {
    return false;
  }
  delay_microseconds_safe(2500);

  std::array<uint8_t, MAX_TRANSFER_PAYLOAD> verify{};
  if (length > verify.size() || !this->read_data_memory(address, verify.data(), length)) {
    return false;
  }
  return std::memcmp(verify.data(), data, length) == 0;
}

bool BQ76952Protocol::read_data_memory_u8(uint16_t address, uint8_t &value) {
  return this->read_data_memory(address, &value, 1);
}

bool BQ76952Protocol::read_data_memory_u16(uint16_t address, uint16_t &value) {
  uint8_t raw[2]{};
  if (!this->read_data_memory(address, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>(raw[0]) | (static_cast<uint16_t>(raw[1]) << 8);
  return true;
}

bool BQ76952Protocol::write_data_memory_u8(uint16_t address, uint8_t value) {
  return this->write_data_memory(address, &value, 1);
}

bool BQ76952Protocol::write_data_memory_u16(uint16_t address, uint16_t value) {
  const uint8_t raw[2] = {static_cast<uint8_t>(value & 0xFFU), static_cast<uint8_t>((value >> 8) & 0xFFU)};
  return this->write_data_memory(address, raw, sizeof(raw));
}

bool BQ76952Protocol::set_config_update(bool enabled) {
  const uint16_t command = enabled ? SUBCMD_SET_CFGUPDATE : SUBCMD_EXIT_CFGUPDATE;
  if (!this->send_subcommand(command)) {
    return false;
  }

  delay_microseconds_safe(enabled ? 2200 : 1200);
  const uint32_t started = millis();
  while ((millis() - started) < 500U) {
    uint16_t battery_status = 0;
    if (!this->read_u16(REG_BATTERY_STATUS, battery_status)) {
      return false;
    }
    if (((battery_status & BATTERY_STATUS_CFGUPDATE) != 0) == enabled) {
      return true;
    }
    delay_microseconds_safe(1000);
  }
  ESP_LOGW(TAG, "Timed out waiting for CONFIG_UPDATE=%s", enabled ? "on" : "off");
  return false;
}

void BQ76952Protocol::set_crc_enabled(bool enabled) {
  this->crc_enabled_ = enabled;
}

}  // namespace bq76952
}  // namespace esphome
