#!/usr/bin/env python3
"""Reject ESPHome dependencies from selected host-independent core files."""

from __future__ import annotations

from pathlib import Path
import re
import sys

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp"}
CORE_FILE_RE = re.compile(r"(?:.*_(?:bus|protocol|service|status|core)|calibration)\.(?:c|cc|cpp|cxx|h|hh|hpp)$")
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
FORBIDDEN_TOKENS = ("ESP_LOG", "esphome::")


def iter_checked_files(root: Path):
    if root.is_file():
        candidates = [root]
    else:
        candidates = sorted(path for path in root.rglob("*") if path.is_file())

    check_all_sources = root.name == "component_common"
    for path in candidates:
        if path.suffix not in SOURCE_SUFFIXES:
            continue
        if check_all_sources or CORE_FILE_RE.fullmatch(path.name):
            yield path


def check_file(path: Path) -> list[str]:
    errors: list[str] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        include = INCLUDE_RE.match(line)
        if include and (include.group(1).startswith("esphome/") or "/esphome/" in include.group(1)):
            errors.append(f"{path}:{line_number}: host-independent code includes {include.group(1)!r}")
        for token in FORBIDDEN_TOKENS:
            if token in line:
                errors.append(f"{path}:{line_number}: host-independent code contains {token!r}")
    return errors


def main(argv: list[str]) -> int:
    roots = [Path(value) for value in argv] or [Path("components/component_common"), Path("components/bq25756")]
    errors: list[str] = []
    checked = 0

    for root in roots:
        if not root.exists():
            errors.append(f"{root}: path does not exist")
            continue
        for path in iter_checked_files(root):
            checked += 1
            errors.extend(check_file(path))

    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    if checked == 0:
        print("no host-independent source files matched", file=sys.stderr)
        return 1

    print(f"core purity: checked {checked} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
