#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
COMPONENTS = ("mcf8316d", "mcf8329a")

CALLS = ("read_reg32", "read_reg16", "write_reg32", "update_bits32")


def transform_source(path: Path) -> bool:
    original = path.read_text(encoding="utf-8")
    text = original

    # Replace legacy address constants with typed IDs everywhere outside the
    # metadata table itself.
    text = re.sub(r"\bREG_([A-Z0-9_]+)\b", r"register_address(RegisterId::\1)", text)

    # Component and service APIs accept IDs, never addresses.
    for name in CALLS:
        text = re.sub(
            rf"\b{name}\(register_address\(RegisterId::([A-Z0-9_]+)\)",
            rf"{name}(RegisterId::\1",
            text,
        )
        text = re.sub(
            rf"\b{name}\(uint16_t offset",
            rf"{name}(RegisterId id",
            text,
        )

    # Low-level register access remains address based behind the service.
    text = text.replace("registers_.read32(offset,", "registers_.read32(register_address(id),")
    text = text.replace("registers_.read16(offset,", "registers_.read16(register_address(id),")
    text = text.replace("registers_.write32(offset,", "registers_.write32(register_address(id),")
    text = text.replace("registers_.update_bits32(offset,", "registers_.update_bits32(register_address(id),")
    text = text.replace("service_.read_reg32(offset,", "service_.read_reg32(id,")
    text = text.replace("service_.read_reg16(offset,", "service_.read_reg16(id,")
    text = text.replace("service_.write_reg32(offset,", "service_.write_reg32(id,")
    text = text.replace("service_.update_bits32(offset,", "service_.update_bits32(id,")

    if text != original:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def transform_register_header(path: Path) -> bool:
    original = path.read_text(encoding="utf-8")
    text = re.sub(
        r"\ninline constexpr uint16_t REG_[A-Z0-9_]+ = register_address\(RegisterId::[A-Z0-9_]+\);",
        "",
        original,
    )
    if text != original:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def assert_hard_cut() -> None:
    failures: list[str] = []
    for component in COMPONENTS:
        root = ROOT / "components" / component
        for path in root.glob("*.h"):
            text = path.read_text(encoding="utf-8")
            if re.search(r"\bREG_[A-Z0-9_]+\b", text):
                failures.append(f"{path}: legacy REG_* alias remains")
            if re.search(r"\b(?:read_reg32|read_reg16|write_reg32|update_bits32)\(uint16_t", text):
                failures.append(f"{path}: raw-address component API remains")
        for path in root.glob("*.cpp"):
            text = path.read_text(encoding="utf-8")
            if re.search(r"\bREG_[A-Z0-9_]+\b", text):
                failures.append(f"{path}: legacy REG_* use remains")
            if re.search(r"\b(?:read_reg32|read_reg16|write_reg32|update_bits32)\(uint16_t", text):
                failures.append(f"{path}: raw-address component API remains")
    if failures:
        raise SystemExit("\n".join(failures))


def main() -> int:
    changed = False
    for component in COMPONENTS:
        root = ROOT / "components" / component
        for path in sorted(root.glob("*.h")) + sorted(root.glob("*.cpp")):
            if path.name == f"{component}_registers.h":
                changed |= transform_register_header(path)
            else:
                changed |= transform_source(path)
    assert_hard_cut()
    print("typed register hard cut applied" if changed else "typed register hard cut already applied")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
