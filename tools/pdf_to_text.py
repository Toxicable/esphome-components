#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
import sys

try:
    from pypdf import PdfReader
except ImportError:  # pragma: no cover - runtime dependency guard
    PdfReader = None


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Extract text from a text-based PDF and write <pdf>.txt verbatim."
        )
    )
    parser.add_argument("pdf", type=Path, help="Path to the PDF file.")
    return parser.parse_args()


def _extract_text(pdf_path: Path, page_separator: str = "\f") -> str:
    reader = PdfReader(str(pdf_path))
    pages = []
    for page in reader.pages:
        pages.append(page.extract_text() or "")
    return page_separator.join(pages)


def main() -> int:
    args = _parse_args()

    if PdfReader is None:
        print("Missing dependency: pypdf", file=sys.stderr)
        print("Install with: pip install pypdf", file=sys.stderr)
        return 2

    if not args.pdf.exists():
        print(f"PDF not found: {args.pdf}", file=sys.stderr)
        return 1

    output_path = args.pdf.with_suffix(".txt")
    text = _extract_text(args.pdf)
    output_path.write_text(text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
