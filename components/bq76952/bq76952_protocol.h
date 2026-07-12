#pragma once

#include <cstddef>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace bq76952 {

class BQ76952Service;

// Low-level BQ76952 transport only. This class owns register framing,
// subcommand transfer-buffer handling, checksums, optional I2C CRC, and
// CONFIG_UPDATE entry/exit. It contains no ESPHome entities or product policy.
class BQ76952Protocol : public i2c::I2CDevice {
  friend class BQ76952Service;

 protected:
  bool read_u8(uint8_t command, uint8_t &value);
  bool read_i16(uint8_t command, int16_t &value);
  bool read_u16(uint8_t command, uint16_t &value);
  bool read_bytes(uint8_t command, uint8_t *data, size_t length);

  bool write_u8(uint8_t command, uint8_t value);
  bool write_u16(uint8_t command, uint16_t value);
  bool write_bytes(uint8_t command, const uint8_t *data, size_t length);

  bool send_subcommand(uint16_t subcommand);
  bool read_subcommand(uint16_t subcommand, uint8_t *data, size_t length);
  bool write_subcommand(uint16_t subcommand, const uint8_t *data, size_t length);

  bool read_data_memory(uint16_t address, uint8_t *data, size_t length);
  bool write_data_memory(uint16_t address, const uint8_t *data, size_t length);
  bool read_data_memory_u8(uint16_t address, uint8_t &value);
  bool read_data_memory_u16(uint16_t address, uint16_t &value);
  bool write_data_memory_u8(uint16_t address, uint8_t value);
  bool write_data_memory_u16(uint16_t address, uint16_t value);

  bool set_config_update(bool enabled, bool crc_after_exit = false);
  void set_crc_enabled(bool enabled);

 private:
  bool wait_for_transfer_buffer(uint16_t expected_command, uint32_t timeout_ms);
  bool read_transfer_buffer(uint16_t expected_command, uint8_t *data, size_t length);
  bool verify_transfer_buffer(uint16_t command, const uint8_t *data, size_t length, uint8_t checksum) const;

  bool crc_enabled_{false};
};

}  // namespace bq76952
}  // namespace esphome
