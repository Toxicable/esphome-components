## Continuous learning
- When you learn new **repo-wide** project knowledge, coding style, or preferences during a session, update `AGENTS.md` (and `README.md` if it affects users) before finishing so the next agent benefits.
- When you learn new **component-specific** knowledge, update that component's `components/<name>/AGENTS_KNOWLEDGE.md` before finishing.

## Component knowledge files (required)
- Before working in `components/<name>/`, read `components/<name>/AGENTS_KNOWLEDGE.md` if it exists and follow it as component-scoped guidance.
- Keep component-specific notes out of this root file when possible; store them in the component's own `AGENTS_KNOWLEDGE.md`.

## Repo tips
- Source components live under `components/`; prefer scoped changes there unless asked otherwise.
- Run `./check.bash` to perform `clangd --check` over C/C++ sources (or pass a file path for a single-file check).
- Run `./check_py.bash` to syntax-check Python files via `py_compile` without creating `__pycache__` artifacts in the repo.

## Cross-component learned project notes
- Component READMEs should present a single configuration example with optional items commented out instead of separate basic/full examples.
- README entity example names should not be prefixed with the component name (for example, use `Pack Voltage` instead of `BQ76922 Pack Voltage`).
- In README YAML examples, use `##` for section/explanatory comment lines and keep single `#` for commented keys so bulk-uncomment keeps headings as comments.
- Prefer `std::numeric_limits<float>::quiet_NaN()` (with `<limits>`) in headers instead of `NAN` to avoid macro ordering issues.
- Validate any chip assumptions against the datasheet before implementing or documenting behavior.
- `tools/pdf_to_text.py` extracts verbatim text from text-based PDFs using `pypdf`, writing `<pdf>.txt` (input-only CLI).
- Devcontainer installs `pypdf` alongside `esphome` for the PDF ingestion tool.
- Devcontainer shells default to the ESP-IDF Python venv; Python deps needed at runtime should be installed into that env (Dockerfile sources `export.sh` before installing).
- ESPHome `i2c::I2CDevice::write_read()` returns `i2c::ErrorCode` (not bool); always compare to `i2c::ERROR_OK` or reads will be inverted.
- ESPHome I2C `ErrorCode` value `2` maps to `ERROR_NOT_ACKNOWLEDGED` (target did not ACK address/data), useful when logs print numeric errors only.
- Component READMEs should include an `external_components` snippet; add an explicit `i2c` block when the component depends on I2C.
- README `external_components` examples should use `source: github://Toxicable/esphome-components@main` with `refresh: 0s`, and list the specific component in `components: [ ... ]`.
- ESPHome `number.number_schema()` accepts metadata only (no `min_value`/`max_value`/`step`); bounds belong in `number.new_number(...)`.
- `.clang-format` should keep a JS-like feel for C++ formatting (2-space indents, attached braces, and tighter wrapping).
- Prefer monolith ESPHome integrations in component `__init__.py` (single `<component_name>:` YAML block with optional nested entity configs) over split platform modules (`sensor.py`, `switch.py`, `number.py`, etc.).
