#!/usr/bin/env python3
from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
import re
from pathlib import Path

try:
    from pypdf import PdfReader
except ImportError:  # pragma: no cover - runtime dependency guard
    PdfReader = None


BOILERPLATE_PATTERNS = (
    ("product_links", re.compile(r"^Product Folder Links: .+$")),
    ("website", re.compile(r"^www\.[A-Za-z0-9.-]+$")),
    ("copyright", re.compile(r"^Copyright © .+$")),
    ("submit_feedback", re.compile(r"^.+ Submit Document Feedback .+$")),
    ("revision_tag", re.compile(r"^SLL[A-Za-z0-9-]+\s+–\s+[A-Z]+\s+\d{4}$")),
)

SECTION_HEADING_PATTERN = re.compile(r"^\d+(?:\.\d+){0,3}\s+[A-Z][A-Za-z0-9_-].+")
TABLE_ROW_PATTERN = re.compile(r"^\d+\s+[A-Z0-9_]+\s+[A-Z/]+\s+[0-9A-Fa-f]+h\b")
TABLE_ROW_REGISTER_PATTERN = re.compile(
    r"^[0-9A-Fa-f]+h\s+([A-Z][A-Z0-9_]+)\s+.+\bSection\s+\d"
)
REGISTER_OFFSET_PATTERN = re.compile(
    r"(?:^|\s)([A-Z][A-Z0-9_]+)\s+Register\s+\(Offset\s*=\s*([0-9A-Fa-f]+h)\)"
)
TOKEN_CANDIDATE_PATTERN = re.compile(r"\b[A-Z][A-Z0-9_]{2,}\b")
BITFIELD_TOKEN_PATTERN = re.compile(r"\b([A-Z][A-Z0-9_]{2,})\s*\[\d+(?::\d+)?\]")
TOKEN_STOPWORDS = {
    "TABLE",
    "FIGURE",
    "SECTION",
    "REGISTER",
    "REGISTERS",
    "OFFSET",
    "FIELD",
    "FIELDS",
    "BITS",
    "BIT",
    "DEFAULT",
    "VALUE",
    "VALUES",
    "DESCRIPTION",
    "DEVICE",
    "PRODUCT",
    "FOLDER",
    "LINKS",
    "SUBMIT",
    "DOCUMENT",
    "FEEDBACK",
    "COPYRIGHT",
    "REVISION",
    "PAGE",
    "READ",
    "WRITE",
    "RESERVED",
    "INPUT",
    "OUTPUT",
    "ADDRESS",
    "STATUS",
    "CONTROL",
    "CONFIGURATION",
}

TI_MCF83XX_PROFILE_TOKENS = (
    "ALGORITHM_STATE",
    "MOTOR_STARTUP1",
    "MOTOR_STARTUP2",
    "CLOSED_LOOP2",
    "CLOSED_LOOP3",
    "CLOSED_LOOP4",
    "FAULT_CONFIG1",
    "FAULT_CONFIG2",
    "ALGO_CTRL1",
    "ALGO_DEBUG2",
    "MTR_PARAMS",
    "VM_VOLTAGE",
    "SPEED_FDBK",
    "SPEED_REF_OPEN_LOOP",
    "FG_SPEED_FDBK",
)

TOKEN_PROFILES = {
    "none": (),
    "ti-mcf83xx": TI_MCF83XX_PROFILE_TOKENS,
}


@dataclass(frozen=True)
class LineEntry:
    canonical_line: int
    text: str


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Create <source>.compact.txt and <source>.index.md from a canonical datasheet source. "
            "Prefer a PDF input; extracted text is processed in memory and not kept on disk."
        )
    )
    parser.add_argument(
        "source",
        type=Path,
        help="Path to canonical datasheet source PDF (preferred) or text file.",
    )
    parser.add_argument(
        "--token",
        action="append",
        default=[],
        help="Additional quick-reference token to include in the index (repeatable).",
    )
    parser.add_argument(
        "--token-mode",
        choices=("auto", "manual", "hybrid"),
        default="auto",
        help=(
            "Quick-token selection mode: auto discovers from the datasheet, "
            "manual uses only profile/--token values, hybrid uses both."
        ),
    )
    parser.add_argument(
        "--profile",
        choices=tuple(TOKEN_PROFILES.keys()),
        default="none",
        help="Optional token profile for known datasheet families (default: none).",
    )
    parser.add_argument(
        "--max-token-hits",
        type=int,
        default=12,
        help="Maximum hits per quick-reference token in the index (default: 12).",
    )
    parser.add_argument(
        "--max-quick-tokens",
        type=int,
        default=20,
        help="Maximum number of quick tokens to emit (default: 20).",
    )
    parser.add_argument(
        "--min-quick-token-score",
        type=int,
        default=18,
        help="Minimum score required for auto-discovered quick tokens (default: 18).",
    )
    return parser.parse_args()


def _split_canonical_lines(raw_text: str) -> list[str]:
    normalized = raw_text.replace("\x00", "").replace("\f", "\n")
    normalized = normalized.replace("\r\n", "\n").replace("\r", "\n")
    return normalized.split("\n")


def _extract_pdf_text(pdf_path: Path, page_separator: str = "\f") -> str:
    if PdfReader is None:
        raise SystemExit("Missing dependency: pypdf")
    reader = PdfReader(str(pdf_path))
    pages = []
    for page in reader.pages:
        pages.append(page.extract_text() or "")
    return page_separator.join(pages)


def _load_canonical_lines(source_path: Path) -> list[str]:
    if source_path.suffix.lower() == ".pdf":
        raw_text = _extract_pdf_text(source_path)
    else:
        raw_text = source_path.read_text(encoding="utf-8", errors="replace")
    return _split_canonical_lines(raw_text)


def _boilerplate_reason(line: str) -> str | None:
    stripped = line.strip()
    if not stripped:
        return None
    for reason, pattern in BOILERPLATE_PATTERNS:
        if pattern.match(stripped):
            return reason
    return None


def _compact_lines(canonical_lines: list[str]) -> tuple[list[LineEntry], Counter[str]]:
    kept: list[LineEntry] = []
    removed: Counter[str] = Counter()
    previous_blank = False

    for idx, raw_line in enumerate(canonical_lines, start=1):
        line = raw_line.rstrip()
        reason = _boilerplate_reason(line)
        if reason is not None:
            removed[reason] += 1
            continue

        if not line.strip():
            if previous_blank:
                continue
            previous_blank = True
            kept.append(LineEntry(canonical_line=idx, text=""))
            continue

        previous_blank = False
        kept.append(LineEntry(canonical_line=idx, text=line))

    while kept and kept[0].text == "":
        kept.pop(0)
    while kept and kept[-1].text == "":
        kept.pop()
    return kept, removed


def _extract_section_headings(canonical_lines: list[str]) -> list[tuple[int, str]]:
    headings: list[tuple[int, str]] = []
    seen: set[str] = set()
    for idx, line in enumerate(canonical_lines, start=1):
        candidate = line.strip()
        if not candidate:
            continue
        if _boilerplate_reason(candidate) is not None:
            continue
        if "Submit Document Feedback" in candidate:
            continue
        if "..." in candidate:
            continue
        if "Register (Offset =" in candidate:
            continue
        if TABLE_ROW_PATTERN.match(candidate):
            continue
        if SECTION_HEADING_PATTERN.match(candidate):
            number_part, _, title_part = candidate.partition(" ")
            if "." not in number_part:
                try:
                    if int(number_part) > 20:
                        continue
                except ValueError:
                    continue
            if not re.search(r"[A-Za-z]{3,}", title_part):
                continue
            normalized = re.sub(r"\s+", " ", candidate)
            if normalized in seen:
                continue
            seen.add(normalized)
            headings.append((idx, candidate))
    return headings


def _extract_register_offsets(canonical_lines: list[str]) -> list[tuple[int, str, str]]:
    offsets: list[tuple[int, str, str]] = []
    seen: set[tuple[str, str]] = set()
    for idx, line in enumerate(canonical_lines, start=1):
        candidate = line.strip()
        match = REGISTER_OFFSET_PATTERN.search(candidate)
        if match is None:
            continue
        register_name = match.group(1)
        offset = match.group(2).lower()
        dedupe_key = (register_name, offset)
        if dedupe_key in seen:
            continue
        seen.add(dedupe_key)
        offsets.append((idx, register_name, offset))
    return offsets


def _dedupe_tokens(tokens: tuple[str, ...] | list[str]) -> tuple[str, ...]:
    out: list[str] = []
    seen: set[str] = set()
    for token in tokens:
        normalized = token.strip().upper()
        if not normalized:
            continue
        if normalized in seen:
            continue
        seen.add(normalized)
        out.append(normalized)
    return tuple(out)


def _is_token_candidate(token: str) -> bool:
    if len(token) < 3:
        return False
    if token in TOKEN_STOPWORDS:
        return False
    if not token[0].isalpha():
        return False
    if re.fullmatch(r"[A-Z]{1,2}\d*", token) is not None:
        return False
    return True


def _discover_quick_tokens(
    canonical_lines: list[str], max_tokens: int, min_score: int
) -> tuple[str, ...]:
    token_scores: Counter[str] = Counter()
    token_hits: Counter[str] = Counter()
    structural_hits: Counter[str] = Counter()
    register_source_tokens: set[str] = set()
    bitfield_source_tokens: set[str] = set()

    def _add_structural(token: str, weight: int, source_kind: str) -> None:
        normalized = token.upper().strip()
        if not _is_token_candidate(normalized):
            return
        token_scores[normalized] += weight
        token_hits[normalized] += 1
        structural_hits[normalized] += 1
        if source_kind == "register":
            register_source_tokens.add(normalized)
        if source_kind == "bitfield":
            bitfield_source_tokens.add(normalized)

    register_offsets = _extract_register_offsets(canonical_lines)
    for _line_no, register_name, _offset in register_offsets:
        _add_structural(register_name, 90, "register")

    for _line_no, heading in _extract_section_headings(canonical_lines):
        for token in TOKEN_CANDIDATE_PATTERN.findall(heading):
            _add_structural(token, 12, "heading")

    for line in canonical_lines:
        candidate = line.strip()
        if not candidate:
            continue
        if _boilerplate_reason(candidate) is not None:
            continue

        row_match = TABLE_ROW_REGISTER_PATTERN.match(candidate)
        if row_match is not None:
            _add_structural(row_match.group(1), 50, "register")

        for bitfield_token in BITFIELD_TOKEN_PATTERN.findall(candidate):
            _add_structural(bitfield_token, 16, "bitfield")

        for token in TOKEN_CANDIDATE_PATTERN.findall(candidate):
            normalized = token.upper()
            if not _is_token_candidate(normalized):
                continue
            token_hits[normalized] += 1

    ubiq_threshold = max(20, len(canonical_lines) // 120)
    ranked: list[tuple[int, int, str]] = []
    for token, hits in token_hits.items():
        score = token_scores[token]
        score += min(hits, 12) * 3
        if structural_hits[token] == 0 and hits >= 4:
            score += 6
        if hits > ubiq_threshold:
            score -= (hits - ubiq_threshold) * 3
        if structural_hits[token] == 0 and hits < 3:
            continue
        is_register_like = token in register_source_tokens
        is_identifier_like = "_" in token
        if not is_register_like:
            if is_identifier_like and hits >= 3:
                pass
            elif token in bitfield_source_tokens and "_" in token and hits >= 4:
                pass
            else:
                continue
        if score < min_score:
            continue
        ranked.append((score, hits, token))

    ranked.sort(key=lambda item: (-item[0], -item[1], item[2]))
    return tuple(token for _score, _hits, token in ranked[: max(1, max_tokens)])


def _extract_token_hits(
    canonical_lines: list[str], token: str, max_hits: int
) -> list[tuple[int, str]]:
    token_upper = token.upper()
    hits: list[tuple[int, str]] = []
    for idx, line in enumerate(canonical_lines, start=1):
        candidate = line.strip()
        if not candidate:
            continue
        if token_upper in candidate.upper():
            hits.append((idx, candidate))
            if len(hits) >= max_hits:
                break
    return hits


def _write_compact(
    output_path: Path,
    canonical_path: Path,
    compact_entries: list[LineEntry],
    original_line_count: int,
) -> list[int]:
    lines = []
    canonical_map: list[int] = []
    lines.append("# Canonical source")
    canonical_map.append(0)
    lines.append(f"# {canonical_path.name}")
    canonical_map.append(0)
    lines.append(
        "# This compact text is generated. Do not edit manually; regenerate with tools/datasheet_prepare.py."
    )
    canonical_map.append(0)
    lines.append(f"# Original line count: {original_line_count}")
    canonical_map.append(0)
    lines.append("")
    canonical_map.append(0)
    for entry in compact_entries:
        lines.append(entry.text)
        canonical_map.append(entry.canonical_line)
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return canonical_map


def _write_line_map(output_path: Path, canonical_map: list[int]) -> None:
    lines = []
    lines.append("compact_line\tcanonical_line")
    for compact_line, canonical_line in enumerate(canonical_map, start=1):
        lines.append(f"{compact_line}\t{canonical_line}")
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _write_index(
    index_path: Path,
    canonical_path: Path,
    compact_path: Path,
    line_map_path: Path,
    canonical_lines: list[str],
    compact_entries: list[LineEntry],
    removed: Counter[str],
    quick_tokens: tuple[str, ...],
    token_mode: str,
    token_profile: str,
    max_token_hits: int,
) -> None:
    section_headings = _extract_section_headings(canonical_lines)
    register_offsets = _extract_register_offsets(canonical_lines)

    lines: list[str] = []
    lines.append(f"# Datasheet Index: `{canonical_path.name}`")
    lines.append("")
    lines.append("## Generated Artifacts")
    lines.append(f"- Canonical source: `{canonical_path.name}`")
    lines.append(f"- Compact source: `{compact_path.name}`")
    lines.append(f"- Compact line map: `{line_map_path.name}`")
    lines.append("")
    lines.append("## Normalization")
    lines.append("- Source text is extracted from the PDF in memory during processing.")
    lines.append(
        "- Compact output removes only known page boilerplate/footer lines and collapses repeated blank lines."
    )
    lines.append(
        "- Canonical lines are line numbers in the extracted text stream used for indexing, not a stored transcript."
    )
    lines.append(
        f"- Canonical lines: {len(canonical_lines)}; compact lines: {len(compact_entries)}"
    )
    lines.append("")
    if removed:
        lines.append("## Removed Boilerplate Summary")
        total_removed = sum(removed.values())
        lines.append(f"- Removed lines: {total_removed}")
        for reason, count in removed.most_common():
            lines.append(f"- {reason}: {count}")
        lines.append("")

    lines.append("## Section Headings (Canonical Line Numbers)")
    if section_headings:
        for line_no, heading in section_headings:
            lines.append(f"- L{line_no}: {heading}")
    else:
        lines.append("- No section headings detected with default pattern.")
    lines.append("")

    lines.append("## Register Offsets (Canonical Line Numbers)")
    if register_offsets:
        for line_no, register_name, offset in register_offsets:
            lines.append(f"- L{line_no}: `{register_name}` @ `{offset}`")
    else:
        lines.append("- No register offsets detected with default pattern.")
    lines.append("")

    lines.append("## Quick Token References (Canonical Line Numbers)")
    lines.append(
        f"- Selection mode: `{token_mode}`; profile: `{token_profile}`; tokens: {len(quick_tokens)}"
    )
    if not quick_tokens:
        lines.append("- No quick tokens selected.")
        lines.append("")
        lines.append(
            "Regenerate with: "
            f"`./tools/datasheet_prepare.py {canonical_path.as_posix()}`"
        )
        lines.append("")
        index_path.write_text("\n".join(lines), encoding="utf-8")
        return
    for token in quick_tokens:
        hits = _extract_token_hits(canonical_lines, token, max_token_hits)
        lines.append(f"- `{token}`:")
        if not hits:
            lines.append("  - no hits")
            continue
        for line_no, text in hits:
            lines.append(f"  - L{line_no}: {text}")
    lines.append("")
    lines.append(
        "Regenerate with: "
        f"`./tools/datasheet_prepare.py {canonical_path.as_posix()}`"
    )
    lines.append("")
    index_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = _parse_args()
    source_path = args.source

    if not source_path.exists():
        raise SystemExit(f"Input file not found: {source_path}")
    if source_path.suffix.lower() not in {".pdf", ".txt"}:
        raise SystemExit(f"Input must be a .pdf or .txt file: {source_path}")

    canonical_lines = _load_canonical_lines(source_path)
    compact_entries, removed = _compact_lines(canonical_lines)

    compact_path = source_path.with_name(f"{source_path.stem}.compact.txt")
    index_path = source_path.with_name(f"{source_path.stem}.index.md")

    compact_line_map = _write_compact(
        compact_path, source_path, compact_entries, len(canonical_lines)
    )
    line_map_path = source_path.with_name(f"{source_path.stem}.compact.map.tsv")
    _write_line_map(line_map_path, compact_line_map)

    manual_tokens = _dedupe_tokens(TOKEN_PROFILES[args.profile] + tuple(args.token))
    auto_tokens: tuple[str, ...] = ()
    if args.token_mode in {"auto", "hybrid"}:
        auto_tokens = _discover_quick_tokens(
            canonical_lines=canonical_lines,
            max_tokens=max(1, args.max_quick_tokens),
            min_score=max(0, args.min_quick_token_score),
        )

    if args.token_mode == "auto":
        quick_tokens = auto_tokens
    elif args.token_mode == "manual":
        quick_tokens = manual_tokens
    else:
        quick_tokens = _dedupe_tokens(manual_tokens + auto_tokens)

    _write_index(
        index_path=index_path,
        canonical_path=source_path,
        compact_path=compact_path,
        line_map_path=line_map_path,
        canonical_lines=canonical_lines,
        compact_entries=compact_entries,
        removed=removed,
        quick_tokens=quick_tokens,
        token_mode=args.token_mode,
        token_profile=args.profile,
        max_token_hits=max(1, args.max_token_hits),
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
