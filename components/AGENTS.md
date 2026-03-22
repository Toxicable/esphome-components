## Component-scoped workflow
- Before modifying files in `components/<name>/`, read `components/<name>/AGENTS_KNOWLEDGE.md` if it exists and apply that guidance for the task.
- Then read `components/<name>/CONTEXT_INDEX.md`; if it does not exist, create it.
- When you learn new component-specific behavior, update that component's `AGENTS_KNOWLEDGE.md` in the same change.
- Keep `AGENTS_KNOWLEDGE.md` concise (active invariants only); move historical bring-up/session detail to `components/<name>/notes/*.md` and reference it.
- If file ownership or architecture boundaries change, update (or add) `components/<name>/CONTEXT_INDEX.md` in the same change.
- For datasheet workflows, keep the PDF canonical and regenerate `<doc>.compact.txt`, `<doc>.index.md`, and `<doc>.compact.map.tsv` via `tools/datasheet_prepare.py` when the PDF changes.
- Quick token references are auto-discovered by default; add `--profile ti-mcf83xx` and/or `--token ...` when a component needs fixed family-specific entries.
