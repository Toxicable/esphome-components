#!/usr/bin/env python3
"""Require every retained component to have an explicit register architecture classification."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
COMPONENTS = ROOT / "components"

TYPED_REGISTER_COMPONENTS = {
    "bq25756": "bq25756_registers.h",
    "bq76952": "bq76952_registers.h",
    "esc_higher": "esc_higher_registers.h",
    "husb238": "husb238_registers.h",
    "lps25hb": "lps25hb_registers.h",
    "mcf8316d": "mcf8316d_registers.h",
    "mcf8329a": "mcf8329a_registers.h",
    "mcp4726": "mcp4726_protocol.h",
    "mlx90614": "mlx90614_registers.h",
}
NON_REGISTER_COMPONENTS = {
    "drv8243",
    "l04xmtw",
    "makita_xgt",
    "programmable_load",
}
INTERNAL_COMPONENTS = {
    "component_common",
    "mcf83xx_common",
}

MIGRATED_RAW_CONSTANT_SCAN = {
    "esc_higher",
    "husb238",
    "lps25hb",
    "mcp4726",
    "mlx90614",
}
RAW_DEFINITION = re.compile(r"\b(?:REG|RAM|OPCODE)_[A-Z0-9_]+\s*=\s*0x[0-9A-Fa-f]+")


def packages() -> set[str]:
    return {
        path.name
        for path in COMPONENTS.iterdir()
        if path.is_dir() and (path / "__init__.py").is_file()
    }


def main() -> int:
    errors: list[str] = []
    actual = packages()
    classified = set(TYPED_REGISTER_COMPONENTS) | NON_REGISTER_COMPONENTS | INTERNAL_COMPONENTS

    for name in sorted(actual - classified):
        errors.append(f"component `{name}` has no register architecture classification")
    for name in sorted(classified - actual):
        errors.append(f"classified component `{name}` has no package")

    for component, metadata_file in sorted(TYPED_REGISTER_COMPONENTS.items()):
        path = COMPONENTS / component / metadata_file
        if not path.is_file():
            errors.append(f"typed component `{component}` is missing {metadata_file}")

    for component in sorted(MIGRATED_RAW_CONSTANT_SCAN):
        for path in sorted((COMPONENTS / component).glob("*")):
            if not path.is_file() or path.name.endswith("_registers.h") or path.name == "mcp4726_protocol.h":
                continue
            if path.suffix not in {".h", ".cpp"}:
                continue
            text = path.read_text(encoding="utf-8")
            match = RAW_DEFINITION.search(text)
            if match:
                errors.append(f"{path.relative_to(ROOT)} retains raw wire constant `{match.group(0)}`")

    if errors:
        for error in errors:
            print(f"register models: {error}", file=sys.stderr)
        return 1

    print(
        "register models: "
        f"{len(TYPED_REGISTER_COMPONENTS)} typed, "
        f"{len(NON_REGISTER_COMPONENTS)} non-register, "
        f"{len(INTERNAL_COMPONENTS)} internal"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
