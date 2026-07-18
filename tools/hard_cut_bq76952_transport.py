#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
COMPONENT = ROOT / "components" / "bq76952"
HEADER = COMPONENT / "bq76952_i2c_transport.h"
SOURCE = COMPONENT / "bq76952_i2c_transport.cpp"

HEADER_CONTENT = r'''#pragma once

#include <cstddef>
#include <cstdint>

#include "bq76952_registers.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace bq76952 {

class BQ76952Service;

class BQ76952I2CTransport : public i2c::I2CDevice {
  friend class BQ76952Service;

 protected:
  using DirectCommandId = ::bq76952_core::registers::DirectCommandId;
  using SubcommandId = ::bq76952_core::registers::SubcommandId;
  using DataMemoryId = ::bq76952_core::registers::DataMemoryId;

  bool read_u8(DirectCommandId command, uint8_t &value);
  bool read_i16(DirectCommandId command, int16_t &value);
  bool read_u16(DirectCommandId command, uint16_t &value);
  bool read_bytes(DirectCommandId command, uint8_t *data, size_t length);

  bool write_u8(DirectCommandId command, uint8_t value);
  bool write_u16(DirectCommandId command, uint16_t value);
  bool write_bytes(DirectCommandId command, const uint8_t *data, size_t length);

  bool send_subcommand(SubcommandId subcommand);
  bool read_subcommand(SubcommandId subcommand, uint8_t *data, size_t length);
  bool write_subcommand(SubcommandId subcommand, const uint8_t *data, size_t length);

  bool read_data_memory(DataMemoryId address, uint8_t *data, size_t length);
  bool write_data_memory(DataMemoryId address, const uint8_t *data, size_t length);
  bool read_data_memory_u8(DataMemoryId address, uint8_t &value);
  bool read_data_memory_u16(DataMemoryId address, uint16_t &value);
  bool write_data_memory_u8(DataMemoryId address, uint8_t value);
  bool write_data_memory_u16(DataMemoryId address, uint16_t value);

  bool set_config_update(bool enabled);
  void set_crc_enabled(bool enabled);

 private:
  bool read_u8_address(uint8_t command, uint8_t &value);
  bool read_i16_address(uint8_t command, int16_t &value);
  bool read_u16_address(uint8_t command, uint16_t &value);
  bool read_bytes_address(uint8_t command, uint8_t *data, size_t length);
  bool write_u8_address(uint8_t command, uint8_t value);
  bool write_u16_address(uint8_t command, uint16_t value);
  bool write_bytes_address(uint8_t command, const uint8_t *data, size_t length);

  bool read_bytes_with_mode(uint8_t command, uint8_t *data, size_t length, bool crc_enabled);
  bool write_bytes_with_mode(uint8_t command, const uint8_t *data, size_t length, bool crc_enabled);
  bool send_subcommand_code(uint16_t subcommand);
  bool read_subcommand_code(uint16_t subcommand, uint8_t *data, size_t length);
  bool write_subcommand_code(uint16_t subcommand, const uint8_t *data, size_t length);
  bool wait_for_transfer_buffer(uint16_t expected_command, uint32_t timeout_ms);
  bool read_transfer_buffer(uint16_t expected_command, uint8_t *data, size_t length);
  bool verify_transfer_buffer(uint16_t command, const uint8_t *data, size_t length, uint8_t checksum) const;

  bool crc_enabled_{true};
  bool desired_crc_enabled_{false};
};

}  // namespace bq76952
}  // namespace esphome
'''

WRAPPERS = r'''
bool BQ76952I2CTransport::read_u8(hw::DirectCommandId command, uint8_t &value) {
  if (hw::direct_command_info(command).response_width != component_common::OperationWidth::U8) {
    return false;
  }
  return this->read_u8_address(static_cast<uint8_t>(hw::direct_command_address(command)), value);
}

bool BQ76952I2CTransport::read_i16(hw::DirectCommandId command, int16_t &value) {
  if (hw::direct_command_info(command).response_width != component_common::OperationWidth::U16) {
    return false;
  }
  return this->read_i16_address(static_cast<uint8_t>(hw::direct_command_address(command)), value);
}

bool BQ76952I2CTransport::read_u16(hw::DirectCommandId command, uint16_t &value) {
  if (hw::direct_command_info(command).response_width != component_common::OperationWidth::U16) {
    return false;
  }
  return this->read_u16_address(static_cast<uint8_t>(hw::direct_command_address(command)), value);
}

bool BQ76952I2CTransport::read_bytes(hw::DirectCommandId command, uint8_t *data, size_t length) {
  const auto width = hw::direct_command_info(command).response_width;
  if (width != component_common::OperationWidth::VARIABLE &&
      width != static_cast<component_common::OperationWidth>(length)) {
    return false;
  }
  return this->read_bytes_address(static_cast<uint8_t>(hw::direct_command_address(command)), data, length);
}

bool BQ76952I2CTransport::write_u8(hw::DirectCommandId command, uint8_t value) {
  if (hw::direct_command_info(command).request_width != component_common::OperationWidth::U8) {
    return false;
  }
  return this->write_u8_address(static_cast<uint8_t>(hw::direct_command_address(command)), value);
}

bool BQ76952I2CTransport::write_u16(hw::DirectCommandId command, uint16_t value) {
  if (hw::direct_command_info(command).request_width != component_common::OperationWidth::U16) {
    return false;
  }
  return this->write_u16_address(static_cast<uint8_t>(hw::direct_command_address(command)), value);
}

bool BQ76952I2CTransport::write_bytes(hw::DirectCommandId command, const uint8_t *data, size_t length) {
  const auto width = hw::direct_command_info(command).request_width;
  if (width != component_common::OperationWidth::VARIABLE &&
      width != static_cast<component_common::OperationWidth>(length)) {
    return false;
  }
  return this->write_bytes_address(static_cast<uint8_t>(hw::direct_command_address(command)), data, length);
}

bool BQ76952I2CTransport::send_subcommand(hw::SubcommandId subcommand) {
  return this->send_subcommand_code(hw::subcommand_address(subcommand));
}

bool BQ76952I2CTransport::read_subcommand(hw::SubcommandId subcommand, uint8_t *data, size_t length) {
  const auto width = hw::subcommand_info(subcommand).response_width;
  if (width != component_common::OperationWidth::VARIABLE &&
      width != static_cast<component_common::OperationWidth>(length)) {
    return false;
  }
  return this->read_subcommand_code(hw::subcommand_address(subcommand), data, length);
}

bool BQ76952I2CTransport::write_subcommand(hw::SubcommandId subcommand, const uint8_t *data, size_t length) {
  const auto width = hw::subcommand_info(subcommand).request_width;
  if (width != component_common::OperationWidth::VARIABLE &&
      width != static_cast<component_common::OperationWidth>(length)) {
    return false;
  }
  return this->write_subcommand_code(hw::subcommand_address(subcommand), data, length);
}

'''


def replace_header() -> None:
    HEADER.write_text(HEADER_CONTENT, encoding="utf-8")


def transform_source() -> None:
    text = SOURCE.read_text(encoding="utf-8")

    signature_replacements = {
        "read_u8(uint8_t command": "read_u8_address(uint8_t command",
        "read_i16(uint8_t command": "read_i16_address(uint8_t command",
        "read_u16(uint8_t command": "read_u16_address(uint8_t command",
        "read_bytes(uint8_t command": "read_bytes_address(uint8_t command",
        "write_u8(uint8_t command": "write_u8_address(uint8_t command",
        "write_u16(uint8_t command": "write_u16_address(uint8_t command",
        "write_bytes(uint8_t command": "write_bytes_address(uint8_t command",
        "send_subcommand(uint16_t subcommand": "send_subcommand_code(uint16_t subcommand",
        "read_subcommand(uint16_t subcommand": "read_subcommand_code(uint16_t subcommand",
        "write_subcommand(uint16_t subcommand": "write_subcommand_code(uint16_t subcommand",
    }
    for old, new in signature_replacements.items():
        text = text.replace(old, new)

    text = text.replace("this->read_bytes(command,", "this->read_bytes_address(command,")
    text = text.replace("this->read_u16(command,", "this->read_u16_address(command,")
    text = text.replace("this->write_bytes(command,", "this->write_bytes_address(command,")
    text = text.replace("this->send_subcommand(subcommand)", "this->send_subcommand_code(subcommand)")
    text = text.replace("this->read_subcommand(address,", "this->read_subcommand_code(address,")
    text = text.replace("this->write_subcommand(address,", "this->write_subcommand_code(address,")

    text = re.sub(
        r"bool BQ76952I2CTransport::read_data_memory\(uint16_t address,",
        "bool BQ76952I2CTransport::read_data_memory(hw::DataMemoryId address,",
        text,
    )
    text = re.sub(
        r"bool BQ76952I2CTransport::write_data_memory\(uint16_t address,",
        "bool BQ76952I2CTransport::write_data_memory(hw::DataMemoryId address,",
        text,
    )
    text = re.sub(
        r"bool BQ76952I2CTransport::read_data_memory_u8\(uint16_t address,",
        "bool BQ76952I2CTransport::read_data_memory_u8(hw::DataMemoryId address,",
        text,
    )
    text = re.sub(
        r"bool BQ76952I2CTransport::read_data_memory_u16\(uint16_t address,",
        "bool BQ76952I2CTransport::read_data_memory_u16(hw::DataMemoryId address,",
        text,
    )
    text = re.sub(
        r"bool BQ76952I2CTransport::write_data_memory_u8\(uint16_t address,",
        "bool BQ76952I2CTransport::write_data_memory_u8(hw::DataMemoryId address,",
        text,
    )
    text = re.sub(
        r"bool BQ76952I2CTransport::write_data_memory_u16\(uint16_t address,",
        "bool BQ76952I2CTransport::write_data_memory_u16(hw::DataMemoryId address,",
        text,
    )

    text = text.replace(
        "return this->read_subcommand_code(address, data, length);",
        "return this->read_subcommand_code(hw::data_memory_address(address), data, length);",
    )
    text = text.replace(
        "if (!this->write_subcommand_code(address, data, length))",
        "if (!this->write_subcommand_code(hw::data_memory_address(address), data, length))",
    )
    text = text.replace(
        "!this->read_data_memory(address, verify.data(), length)",
        "!this->read_data_memory(address, verify.data(), length)",
    )

    direct_call_patterns = (
        "read_u8", "read_i16", "read_u16", "read_bytes", "write_u8", "write_u16", "write_bytes"
    )
    for name in direct_call_patterns:
        text = re.sub(
            rf"this->{name}\(hw::direct_command_address\(hw::DirectCommandId::([A-Z0-9_]+)\)",
            rf"this->{name}(hw::DirectCommandId::\1",
            text,
        )

    text = text.replace(
        "const uint16_t command = enabled ? hw::subcommand_address(hw::SubcommandId::SET_CONFIG_UPDATE) : hw::subcommand_address(hw::SubcommandId::EXIT_CONFIG_UPDATE);\n  if (!this->send_subcommand_code(command))",
        "const auto command = enabled ? hw::SubcommandId::SET_CONFIG_UPDATE : hw::SubcommandId::EXIT_CONFIG_UPDATE;\n  if (!this->send_subcommand(command))",
    )

    insert_at = text.index("bool BQ76952I2CTransport::read_u8_address")
    text = text[:insert_at] + WRAPPERS + text[insert_at:]
    SOURCE.write_text(text, encoding="utf-8")


def migrate_callers() -> None:
    for path in sorted(COMPONENT.glob("*.h")) + sorted(COMPONENT.glob("*.cpp")):
        if path in (HEADER, SOURCE, COMPONENT / "bq76952_registers.h"):
            continue
        original = path.read_text(encoding="utf-8")
        text = original
        for name in ("read_u8", "read_i16", "read_u16", "read_bytes", "write_u8", "write_u16", "write_bytes"):
            text = re.sub(
                rf"transport_\.{name}\(hw::direct_command_address\(hw::DirectCommandId::([A-Z0-9_]+)\)",
                rf"transport_.{name}(hw::DirectCommandId::\1",
                text,
            )
        for name in ("send_subcommand", "read_subcommand", "write_subcommand"):
            text = re.sub(
                rf"transport_\.{name}\(hw::subcommand_address\(hw::SubcommandId::([A-Z0-9_]+)\)",
                rf"transport_.{name}(hw::SubcommandId::\1",
                text,
            )
        for name in (
            "read_data_memory", "write_data_memory", "read_data_memory_u8",
            "read_data_memory_u16", "write_data_memory_u8", "write_data_memory_u16"
        ):
            text = re.sub(
                rf"transport_\.{name}\(hw::data_memory_address\(hw::DataMemoryId::([A-Z0-9_]+)\)",
                rf"transport_.{name}(hw::DataMemoryId::\1",
                text,
            )
        if text != original:
            path.write_text(text, encoding="utf-8")


def validate() -> None:
    failures: list[str] = []
    header = HEADER.read_text(encoding="utf-8")
    if re.search(r"(?:read_u8|read_i16|read_u16|read_bytes|write_u8|write_u16|write_bytes)\(uint8_t", header):
        failures.append("transport header still exposes raw direct-command addresses")
    if re.search(r"(?:send_subcommand|read_subcommand|write_subcommand)\(uint16_t", header):
        failures.append("transport header still exposes raw subcommand codes")
    if re.search(r"(?:read_data_memory|write_data_memory)[^(]*\(uint16_t", header):
        failures.append("transport header still exposes raw data-memory addresses")

    for path in sorted(COMPONENT.glob("*.h")) + sorted(COMPONENT.glob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        if path != SOURCE and "transport_." in text and re.search(
            r"transport_\.[a-z0-9_]+\(hw::(?:direct_command|subcommand|data_memory)_address", text
        ):
            failures.append(f"{path}: caller unwraps typed operation to an address")
    if failures:
        raise SystemExit("\n".join(failures))


def main() -> int:
    replace_header()
    transform_source()
    migrate_callers()
    validate()
    print("BQ76952 transport migrated to typed IDs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
