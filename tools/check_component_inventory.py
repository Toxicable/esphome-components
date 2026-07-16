#!/usr/bin/env python3
"""Verify that every ESPHome component package is represented in COMPONENTS.md."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
COMPONENTS_DIR = ROOT / "components"
INVENTORY_PATH = ROOT / "COMPONENTS.md"

VALID_STATUSES = {
    "active",
    "internal",
    "experimental",
    "legacy",
    "candidate-removal",
    "deprecated",
}
ROW_RE = re.compile(r"^\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|")


def component_packages() -> set[str]:
    return {
        path.name
        for path in COMPONENTS_DIR.iterdir()
        if path.is_dir() and (path / "__init__.py").is_file()
    }


def inventory_rows() -> tuple[dict[str, str], list[str]]:
    rows: dict[str, str] = {}
    errors: list[str] = []
    in_inventory = False

    for line_number, line in enumerate(
        INVENTORY_PATH.read_text(encoding="utf-8").splitlines(), start=1
    ):
        if line.strip() == "## Inventory":
            in_inventory = True
            continue
        if in_inventory and line.startswith("## "):
            break
        if not in_inventory:
            continue

        match = ROW_RE.match(line)
        if match is None:
            continue

        name = match.group(1).strip()
        status = match.group(2).strip()
        if name in rows:
            errors.append(
                f"{INVENTORY_PATH.name}:{line_number}: duplicate component `{name}`"
            )
            continue
        rows[name] = status
        if status not in VALID_STATUSES:
            errors.append(
                f"{INVENTORY_PATH.name}:{line_number}: `{name}` has invalid status "
                f"`{status}`"
            )

    if not in_inventory:
        errors.append(f"{INVENTORY_PATH.name}: missing '## Inventory' section")

    return rows, errors


def main() -> int:
    packages = component_packages()
    rows, errors = inventory_rows()
    documented = set(rows)

    for name in sorted(packages - documented):
        errors.append(f"component package `{name}` is missing from COMPONENTS.md")
    for name in sorted(documented - packages):
        errors.append(
            f"COMPONENTS.md lists `{name}`, but components/{name}/__init__.py is missing"
        )

    if errors:
        for error in errors:
            print(f"component inventory: {error}", file=sys.stderr)
        return 1

    print(f"component inventory: {len(packages)} packages documented")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
