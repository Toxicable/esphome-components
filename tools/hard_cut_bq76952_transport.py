#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
COMMON = ROOT / "components" / "component_common"
BQ = ROOT / "components" / "bq76952"
REGISTERS = BQ / "bq76952_registers.h"
WORKFLOW = ROOT / ".github" / "workflows" / "compile.yml"

COMMAND_INFO = r'''#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace component_common {

enum class PayloadWidth : uint8_t {
  NONE = 0,
  U8 = 1,
  U16 = 2,
  U32 = 4,
  VARIABLE = 0xFF,
};

template<typename CommandId> struct CommandInfo {
  CommandId id{};
  const char *name{nullptr};
  uint16_t code{0};
  PayloadWidth request_width{PayloadWidth::NONE};
  PayloadWidth response_width{PayloadWidth::NONE};
};

template<typename CommandId>
constexpr size_t command_id_index(CommandId id) {
  static_assert(std::is_enum_v<CommandId>, "CommandId must be an enum");
  return static_cast<size_t>(id);
}

template<typename CommandId, size_t N>
constexpr bool command_definitions_have_all_ids_once(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = command_id_index(definition.id);
    if (index >= N || seen[index] || definition.name == nullptr || definition.name[0] == '\0') {
      return false;
    }
    seen[index] = true;
  }
  for (const bool present : seen) {
    if (!present) return false;
  }
  return true;
}

template<typename CommandId, size_t N>
constexpr bool command_definitions_have_unique_codes(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].code == definitions[other].code) return false;
    }
  }
  return true;
}

template<typename CommandId, size_t N>
constexpr std::array<CommandInfo<CommandId>, N> index_command_info_by_id(
    const std::array<CommandInfo<CommandId>, N> &definitions) {
  std::array<CommandInfo<CommandId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = command_id_index(definition.id);
    if (index < N) indexed[index] = definition;
  }
  return indexed;
}

template<typename CommandId, size_t N>
constexpr const CommandInfo<CommandId> &command_info(
    const std::array<CommandInfo<CommandId>, N> &indexed, CommandId id) {
  return indexed[command_id_index(id)];
}

}  // namespace component_common
'''

DATA_MEMORY_INFO = r'''#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "register_info.h"

namespace component_common {

template<typename DataMemoryId> struct DataMemoryInfo {
  DataMemoryId id{};
  const char *name{nullptr};
  uint16_t address{0};
  RegisterWidth width{RegisterWidth::U8};
};

template<typename DataMemoryId>
constexpr size_t data_memory_id_index(DataMemoryId id) {
  static_assert(std::is_enum_v<DataMemoryId>, "DataMemoryId must be an enum");
  return static_cast<size_t>(id);
}

template<typename DataMemoryId, size_t N>
constexpr bool data_memory_definitions_have_all_ids_once(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  std::array<bool, N> seen{};
  for (const auto &definition : definitions) {
    const size_t index = data_memory_id_index(definition.id);
    if (index >= N || seen[index] || definition.name == nullptr || definition.name[0] == '\0') {
      return false;
    }
    seen[index] = true;
  }
  for (const bool present : seen) {
    if (!present) return false;
  }
  return true;
}

template<typename DataMemoryId, size_t N>
constexpr bool data_memory_definitions_have_unique_addresses(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  for (size_t index = 0; index < N; index++) {
    for (size_t other = index + 1; other < N; other++) {
      if (definitions[index].address == definitions[other].address) return false;
    }
  }
  return true;
}

template<typename DataMemoryId, size_t N>
constexpr std::array<DataMemoryInfo<DataMemoryId>, N> index_data_memory_info_by_id(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &definitions) {
  std::array<DataMemoryInfo<DataMemoryId>, N> indexed{};
  for (const auto &definition : definitions) {
    const size_t index = data_memory_id_index(definition.id);
    if (index < N) indexed[index] = definition;
  }
  return indexed;
}

template<typename DataMemoryId, size_t N>
constexpr const DataMemoryInfo<DataMemoryId> &data_memory_info(
    const std::array<DataMemoryInfo<DataMemoryId>, N> &indexed, DataMemoryId id) {
  return indexed[data_memory_id_index(id)];
}

}  // namespace component_common
'''

NORMAL_WORKFLOW = r'''name: Compile checks

on:
  push:
  pull_request:

concurrency:
  group: compile-checks-${{ github.head_ref || github.ref_name }}
  cancel-in-progress: true

jobs:
  host-checks:
    runs-on: ubuntu-latest
    timeout-minutes: 15
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Check Python syntax
        run: ./check_py.bash

      - name: Run host core tests
        run: bash ./check_host.bash 2>&1 | tee host-check-failure.log

      - name: Upload host failure diagnostics
        if: failure()
        uses: actions/upload-artifact@v4
        with:
          name: host-check-failure
          path: host-check-failure.log
          if-no-files-found: error

  esphome-checks:
    name: ESPHome ${{ matrix.name }}
    runs-on: ubuntu-latest
    timeout-minutes: 60
    strategy:
      fail-fast: false
      matrix:
        include:
          - name: bq25756
            config: components/bq25756/test_config.yaml
          - name: bq25756-invalid-cell-count
            config: components/bq25756/test_invalid_cell_count.yaml
          - name: bq76952
            config: components/bq76952/test_config.yaml
          - name: drv8243
            config: components/drv8243/test_config.yaml
          - name: esc-higher
            config: components/esc_higher/test_config.yaml
          - name: lps25hb
            config: components/lps25hb/test_config.yaml
          - name: mcf8316d
            config: components/mcf8316d/test_config.yaml
          - name: mcf8329a
            config: components/mcf8329a/test_config.yaml
          - name: mlx90614
            config: components/mlx90614/test_config.yaml
          - name: programmable-load
            config: components/programmable_load/test_config.yaml
          - name: programmable-load-invalid-hardware-voltage
            config: components/programmable_load/test_invalid_hardware_voltage.yaml
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Cache PlatformIO packages
        uses: actions/cache@v4
        with:
          path: ~/.platformio
          key: ${{ runner.os }}-platformio-esphome-2026.5.3

      - name: Install pinned ESPHome
        run: python -m pip install "esphome==2026.5.3"

      - name: Compile or validate fixture
        run: bash ./check_esphome.bash "${{ matrix.config }}"

      - name: Upload failure diagnostics
        if: failure()
        uses: actions/upload-artifact@v4
        with:
          name: esphome-failure-${{ matrix.name }}
          path: esphome-failure.log
          if-no-files-found: ignore
'''


def rewrite_registers() -> None:
    text = REGISTERS.read_text(encoding="utf-8")
    text = text.replace('#include "../component_common/operation_info.h"', '#include "../component_common/command_info.h"\n#include "../component_common/data_memory_info.h"')
    text = text.replace('using component_common::OperationWidth;', 'using component_common::PayloadWidth;\nusing component_common::RegisterWidth;')
    text = text.replace('DirectCommandId', 'RegisterId').replace('SubcommandId', 'CommandId')
    text = text.replace('OperationWidth::', 'PayloadWidth::')

    start = text.index('using DirectCommandInfo =')
    middle = text.index('using SubcommandInfo =')
    register_block = text[start:middle]
    register_block = register_block.replace('using DirectCommandInfo = component_common::OperationInfo<RegisterId>;', '''struct RegisterInfo {
  RegisterId id{};
  const char *name{nullptr};
  uint8_t address{0};
  PayloadWidth write_width{PayloadWidth::NONE};
  PayloadWidth read_width{PayloadWidth::NONE};
};''')
    register_block = register_block.replace('DIRECT_COMMAND_COUNT', 'REGISTER_COUNT').replace('DIRECT_COMMAND_DEFINITIONS', 'REGISTER_DEFINITIONS').replace('DIRECT_COMMAND_INFO', 'REGISTER_INFO')
    register_block = register_block.replace('DirectCommandInfo', 'RegisterInfo').replace('.code =', '.address =').replace('.request_width =', '.write_width =').replace('.response_width =', '.read_width =')
    register_block = re.sub(r'static_assert\(component_common::operation_definitions_have_all_ids_once\(REGISTER_DEFINITIONS\)\);', '''constexpr bool register_definitions_have_all_ids_once() {
  std::array<bool, REGISTER_COUNT> seen{};
  for (const auto &definition : REGISTER_DEFINITIONS) {
    const size_t index = static_cast<size_t>(definition.id);
    if (index >= REGISTER_COUNT || seen[index] || definition.name == nullptr || definition.name[0] == '\\0') return false;
    seen[index] = true;
  }
  for (const bool present : seen) if (!present) return false;
  return true;
}
static_assert(register_definitions_have_all_ids_once());''', register_block)
    register_block = re.sub(r'static_assert\(component_common::operation_definitions_have_unique_codes\(REGISTER_DEFINITIONS\)\);', '''constexpr bool register_definitions_have_unique_addresses() {
  for (size_t index = 0; index < REGISTER_COUNT; index++)
    for (size_t other = index + 1; other < REGISTER_COUNT; other++)
      if (REGISTER_DEFINITIONS[index].address == REGISTER_DEFINITIONS[other].address) return false;
  return true;
}
static_assert(register_definitions_have_unique_addresses());''', register_block)
    register_block = register_block.replace('component_common::index_operation_info_by_id(REGISTER_DEFINITIONS)', '''[] {
  std::array<RegisterInfo, REGISTER_COUNT> indexed{};
  for (const auto &definition : REGISTER_DEFINITIONS) indexed[static_cast<size_t>(definition.id)] = definition;
  return indexed;
}()''')
    register_block = register_block.replace('component_common::operation_info(REGISTER_INFO, id)', 'REGISTER_INFO[static_cast<size_t>(id)]')
    register_block = register_block.replace('direct_command_info', 'register_info').replace('direct_command_address', 'register_address')
    register_block = register_block.replace('return register_info(id).code;', 'return register_info(id).address;')

    command_start = middle
    data_start = text.index('using DataMemoryInfo =')
    command_block = text[command_start:data_start]
    command_block = command_block.replace('using SubcommandInfo = component_common::OperationInfo<CommandId>;', 'using CommandInfo = component_common::CommandInfo<CommandId>;')
    command_block = command_block.replace('SubcommandInfo', 'CommandInfo').replace('SUBCOMMAND_COUNT', 'COMMAND_COUNT').replace('SUBCOMMAND_DEFINITIONS', 'COMMAND_DEFINITIONS').replace('SUBCOMMAND_INFO', 'COMMAND_INFO')
    command_block = command_block.replace('operation_definitions_have_all_ids_once', 'command_definitions_have_all_ids_once').replace('operation_definitions_have_unique_codes', 'command_definitions_have_unique_codes').replace('index_operation_info_by_id', 'index_command_info_by_id').replace('component_common::operation_info', 'component_common::command_info')
    command_block = command_block.replace('subcommand_info', 'command_info').replace('subcommand_address', 'command_code')

    data_end = text.index('namespace bits {')
    data_block = text[data_start:data_end]
    data_block = data_block.replace('using DataMemoryInfo = component_common::OperationInfo<DataMemoryId>;', 'using DataMemoryInfo = component_common::DataMemoryInfo<DataMemoryId>;')
    row_pattern = re.compile(r'\.code = (0x[0-9A-F]+), \.request_width = PayloadWidth::([A-Z0-9_]+), \.response_width = PayloadWidth::([A-Z0-9_]+)')
    def row_replace(match: re.Match[str]) -> str:
      if match.group(2) != match.group(3):
        raise SystemExit(f"data-memory width mismatch: {match.group(0)}")
      return f'.address = {match.group(1)}, .width = RegisterWidth::{match.group(2)}'
    data_block = row_pattern.sub(row_replace, data_block)
    data_block = data_block.replace('operation_definitions_have_all_ids_once', 'data_memory_definitions_have_all_ids_once').replace('operation_definitions_have_unique_codes', 'data_memory_definitions_have_unique_addresses').replace('index_operation_info_by_id', 'index_data_memory_info_by_id').replace('component_common::operation_info', 'component_common::data_memory_info')
    data_block = data_block.replace('return data_memory_info(id).code;', 'return data_memory_info(id).address;')

    command_transport = '''struct CommandTransport {
  RegisterId command_register;
  RegisterId transfer_buffer_register;
  RegisterId checksum_register;
  RegisterId length_register;
};

inline constexpr CommandTransport COMMAND_TRANSPORT{
    .command_register = RegisterId::SUBCOMMAND,
    .transfer_buffer_register = RegisterId::TRANSFER_BUFFER,
    .checksum_register = RegisterId::CHECKSUM,
    .length_register = RegisterId::LENGTH,
};

'''
    text = text[:start] + register_block + command_block + data_block + command_transport + text[data_end:]
    REGISTERS.write_text(text, encoding="utf-8")


def rewrite_callers() -> None:
    replacements = {
        'DirectCommandId': 'RegisterId',
        'SubcommandId': 'CommandId',
        'direct_command_info': 'register_info',
        'direct_command_address': 'register_address',
        'subcommand_info': 'command_info',
        'subcommand_address': 'command_code',
        'OperationWidth': 'PayloadWidth',
    }
    for path in sorted(BQ.glob('*.h')) + sorted(BQ.glob('*.cpp')) + [ROOT / 'tests' / 'bq76952_status_test.cpp']:
      if path == REGISTERS:
        continue
      text = path.read_text(encoding='utf-8')
      for old, new in replacements.items():
        text = text.replace(old, new)
      text = text.replace('hw::RegisterId::SUBCOMMAND', 'hw::COMMAND_TRANSPORT.command_register')
      text = text.replace('hw::RegisterId::TRANSFER_BUFFER', 'hw::COMMAND_TRANSPORT.transfer_buffer_register')
      text = text.replace('hw::RegisterId::CHECKSUM', 'hw::COMMAND_TRANSPORT.checksum_register')
      text = text.replace('hw::RegisterId::LENGTH', 'hw::COMMAND_TRANSPORT.length_register')
      path.write_text(text, encoding='utf-8')


def cleanup() -> None:
    (COMMON / 'command_info.h').write_text(COMMAND_INFO, encoding='utf-8')
    (COMMON / 'data_memory_info.h').write_text(DATA_MEMORY_INFO, encoding='utf-8')
    old = COMMON / 'operation_info.h'
    if old.exists(): old.unlink()
    WORKFLOW.write_text(NORMAL_WORKFLOW, encoding='utf-8')
    Path(__file__).unlink()


def validate() -> None:
    forbidden = ('OperationInfo', 'OperationWidth', 'DirectCommandId', 'SubcommandId', 'direct_command_', 'subcommand_', 'operation_info.h', 'HEADER_CONTENT')
    paths = list(BQ.glob('*.h')) + list(BQ.glob('*.cpp')) + list(COMMON.glob('*.h')) + [ROOT / 'tests' / 'bq76952_status_test.cpp']
    failures = []
    for path in paths:
      text = path.read_text(encoding='utf-8')
      for token in forbidden:
        if token in text:
          failures.append(f'{path}: forbidden legacy token {token}')
    if failures: raise SystemExit('\n'.join(failures))


def main() -> int:
    rewrite_registers()
    rewrite_callers()
    cleanup()
    validate()
    print('Replaced generic operation model with explicit register, command and data-memory types')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
