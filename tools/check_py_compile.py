#!/usr/bin/env python3
"""Syntax-check Python files with py_compile without creating repo __pycache__ artifacts."""

from __future__ import annotations

import argparse
import py_compile
import sys
import tempfile
from pathlib import Path


def _compiled_output_path(tmp_root: Path, source: Path) -> Path:
    # Stable, filesystem-safe path under temp root.
    encoded = source.as_posix().replace("/", "__")
    return tmp_root / f"{encoded}.pyc"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", help="Python files to syntax-check")
    args = parser.parse_args()

    failed = False
    with tempfile.TemporaryDirectory(prefix="pycompile-check-") as tmp_dir:
        tmp_root = Path(tmp_dir)

        for raw_path in args.files:
            source = Path(raw_path)
            if not source.is_file():
                print(f"Missing file: {source}", file=sys.stderr)
                failed = True
                continue

            cfile = _compiled_output_path(tmp_root, source)
            cfile.parent.mkdir(parents=True, exist_ok=True)

            try:
                py_compile.compile(str(source), cfile=str(cfile), doraise=True)
            except py_compile.PyCompileError as err:
                print(f"[FAIL] {source}", file=sys.stderr)
                print(err.msg, file=sys.stderr)
                failed = True
            except Exception as err:  # pragma: no cover - defensive
                print(f"[FAIL] {source}: {err}", file=sys.stderr)
                failed = True

    if failed:
        return 1

    for raw_path in args.files:
        print(f"[OK] {raw_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
