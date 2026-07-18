#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
COMPONENT = ROOT / "components" / "bq76952"
REGISTERS = COMPONENT / "bq76952_registers.h"

NAMESPACE_RE = r"namespace {name} \{{(?P<body>.*?)\}}  // namespace {name}"
CONST_RE = re.compile(r"inline constexpr uint(?:8|16)_t ([A-Z0-9_]+) = (0x[0-9A-Fa-f]+);")

DIRECT_RESPONSE = {
    "CONTROL_STATUS": "U16",
    "SAFETY_STATUS_A": "U16",
    "SAFETY_STATUS_B": "U16",
    "SAFETY_STATUS_C": "U16",
    "BATTERY_STATUS": "U16",
    "CELL1_VOLTAGE": "U16",
    "STACK_VOLTAGE": "U16",
    "PACK_VOLTAGE": "U16",
    "LD_VOLTAGE": "U16",
    "CC2_CURRENT": "U16",
    "SUBCOMMAND": "U16",
    "TRANSFER_BUFFER": "VARIABLE",
    "CHECKSUM": "U8",
    "LENGTH": "U8",
    "ALARM_STATUS": "U16",
    "INTERNAL_TEMPERATURE": "U16",
    "TS1_TEMPERATURE": "U16",
    "TS2_TEMPERATURE": "U16",
    "TS3_TEMPERATURE": "U16",
    "FET_STATUS": "U8",
}

DIRECT_REQUEST = {
    "SUBCOMMAND": "U16",
    "TRANSFER_BUFFER": "VARIABLE",
    "CHECKSUM": "U8",
    "LENGTH": "U8",
}

SUBCOMMAND_REQUEST = {
    "REG12_CONTROL": "U8",
}

SUBCOMMAND_RESPONSE = {
    "MANUFACTURING_STATUS": "U16",
    "DASTATUS6": "VARIABLE",
    "DA_CONFIGURATION": "U16",
}


def extract_namespace(text: str, name: str) -> tuple[list[tuple[str, int]], str]:
    pattern = re.compile(NAMESPACE_RE.format(name=name), re.S)
    match = pattern.search(text)
    if match is None:
        raise SystemExit(f"missing namespace {name}")
    entries = [(symbol, int(value, 16)) for symbol, value in CONST_RE.findall(match.group("body"))]
    if not entries:
        raise SystemExit(f"namespace {name} had no constants")
    return entries, text[: match.start()] + text[match.end() :]


def enum_block(name: str, entries: list[tuple[str, int]]) -> str:
    values = "\n".join(f"  {symbol}," for symbol, _ in entries)
    return f"enum class {name} : uint8_t {{\n{values}\n  COUNT,\n}};\n"


def info_block(
    enum_name: str,
    type_name: str,
    constant_prefix: str,
    api_name: str,
    entries: list[tuple[str, int]],
    request: dict[str, str],
    response: dict[str, str],
) -> str:
    rows = []
    for symbol, code in entries:
        req = request.get(symbol, "NONE")
        resp = response.get(symbol, "NONE")
        rows.append(
            "    {.id = "
            f"{enum_name}::{symbol}, .name = \"{symbol.lower()}\", .code = 0x{code:04X}, "
            f".request_width = OperationWidth::{req}, .response_width = OperationWidth::{resp}}},"
        )
    joined = "\n".join(rows)
    return f"""
using {type_name}Info = component_common::OperationInfo<{enum_name}>;
inline constexpr size_t {constant_prefix}_COUNT = static_cast<size_t>({enum_name}::COUNT);
inline constexpr std::array<{type_name}Info, {constant_prefix}_COUNT> {constant_prefix}_DEFINITIONS{{{{
{joined}
}}}};
static_assert(component_common::operation_definitions_have_all_ids_once({constant_prefix}_DEFINITIONS));
static_assert(component_common::operation_definitions_have_unique_codes({constant_prefix}_DEFINITIONS));
inline constexpr auto {constant_prefix}_INFO = component_common::index_operation_info_by_id({constant_prefix}_DEFINITIONS);
constexpr const {type_name}Info &{api_name}_info({enum_name} id) {{
  return component_common::operation_info({constant_prefix}_INFO, id);
}}
constexpr uint16_t {api_name}_address({enum_name} id) {{
  return {api_name}_info(id).code;
}}
"""


def infer_data_memory_widths(entries: list[tuple[str, int]]) -> dict[str, str]:
    source = "\n".join(path.read_text(encoding="utf-8") for path in COMPONENT.glob("*.cpp"))
    result: dict[str, str] = {}
    for symbol, _ in entries:
        token = rf"hw::data_memory::{symbol}"
        if re.search(rf"sync_u16\({token}|read_data_memory_u16\({token}|write_data_memory_u16\({token}", source):
            result[symbol] = "U16"
        elif re.search(rf"sync_data\({token}", source):
            result[symbol] = "U32"
        elif re.search(rf"sync_u8\({token}|read_data_memory_u8\({token}|write_data_memory_u8\({token}", source):
            result[symbol] = "U8"
        else:
            result[symbol] = "VARIABLE"
    return result


def rebuild_register_header() -> None:
    original = REGISTERS.read_text(encoding="utf-8")
    direct, text = extract_namespace(original, "direct")
    subcommand, text = extract_namespace(text, "subcommand")
    data_memory, text = extract_namespace(text, "data_memory")

    bits_index = text.index("namespace bits {")
    tail = text[bits_index:]
    widths = infer_data_memory_widths(data_memory)

    prefix = """#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/operation_info.h"

namespace bq76952_core {
namespace registers {

using component_common::OperationWidth;

"""
    generated = (
        prefix
        + enum_block("DirectCommandId", direct)
        + enum_block("SubcommandId", subcommand)
        + enum_block("DataMemoryId", data_memory)
        + info_block(
            "DirectCommandId", "DirectCommand", "DIRECT_COMMAND", "direct_command",
            direct, DIRECT_REQUEST, DIRECT_RESPONSE
        )
        + info_block(
            "SubcommandId", "Subcommand", "SUBCOMMAND", "subcommand",
            subcommand, SUBCOMMAND_REQUEST, SUBCOMMAND_RESPONSE
        )
        + info_block(
            "DataMemoryId", "DataMemory", "DATA_MEMORY", "data_memory",
            data_memory, widths, widths
        )
        + "\n"
        + tail
    )
    REGISTERS.write_text(generated, encoding="utf-8")


def migrate_callers() -> None:
    replacements = (
        (r"hw::direct::([A-Z0-9_]+)", r"hw::direct_command_address(hw::DirectCommandId::\1)"),
        (r"hw::subcommand::([A-Z0-9_]+)", r"hw::subcommand_address(hw::SubcommandId::\1)"),
        (r"hw::data_memory::([A-Z0-9_]+)", r"hw::data_memory_address(hw::DataMemoryId::\1)"),
    )
    for path in sorted(COMPONENT.glob("*.h")) + sorted(COMPONENT.glob("*.cpp")):
        if path == REGISTERS:
            continue
        original = path.read_text(encoding="utf-8")
        text = original
        for pattern, replacement in replacements:
            text = re.sub(pattern, replacement, text)
        if text != original:
            path.write_text(text, encoding="utf-8")


def validate() -> None:
    failures: list[str] = []
    for path in sorted(COMPONENT.glob("*.h")) + sorted(COMPONENT.glob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        for legacy in ("hw::direct::", "hw::subcommand::", "hw::data_memory::"):
            if legacy in text:
                failures.append(f"{path}: legacy operation namespace remains: {legacy}")
    if failures:
        raise SystemExit("\n".join(failures))


def main() -> int:
    rebuild_register_header()
    migrate_callers()
    validate()
    print("BQ76952 operation spaces migrated")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
