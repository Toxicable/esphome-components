## Continuous learning
- When you learn new project knowledge, coding style, or preferences during a session, update `AGENTS.md` (and `README.md` if it affects users) before finishing so the next agent benefits.

## Repo tips
- Source components live under `components/`; prefer scoped changes there unless asked otherwise.
- Run `./check.bash` to perform `clangd --check` over C/C++ sources (or pass a file path for a single-file check).

## Learned project notes
- `components/bq769x0` now uses an ultra-simple YAML config: required `cell_count` (fixed to 4) and `chemistry` (`liion_lipo`), with SOC defaults hardcoded in C++. 
- Component READMEs should present a single configuration example with optional items commented out instead of separate basic/full examples.
- `components/bq769x0` auto-loads sensor/binary_sensor/select/button to keep optional header includes available, and ships a local crc8 helper header so external builds don't need shared helpers (shared crc8 helper removed).
- `components/bq769x0` exposes `mode` as a select that writes CHG_ON/DSG_ON with options `safe`, `standby`, `charge`, `discharge`, and `charge+discharge`.
- For 4S BQ76920 wiring, the component maps cells to VC1/VC2/VC3/VC5 and expects VC4 shorted to VC3 per TI Table 9-2.
- Prefer `std::numeric_limits<float>::quiet_NaN()` (with `<limits>`) in headers instead of `NAN` to avoid macro ordering issues.
- Validate any chip assumptions against the datasheet before implementing or documenting behavior.
- `tools/pdf_to_text.py` extracts verbatim text from text-based PDFs using `pypdf`, writing `<pdf>.txt` (input-only CLI).
- Devcontainer installs `pypdf` alongside `esphome` for the PDF ingestion tool.
- Devcontainer shells default to the ESP-IDF Python venv; Python deps needed at runtime should be installed into that env (Dockerfile sources `export.sh` before installing).
