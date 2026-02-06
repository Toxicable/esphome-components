## Continuous learning
- When you learn new project knowledge, coding style, or preferences during a session, update `AGENTS.md` (and `README.md` if it affects users) before finishing so the next agent benefits.

## Repo tips
- Source components live under `components/`; prefer scoped changes there unless asked otherwise.
- Run `./check.bash` to perform `clangd --check` over C/C++ sources (or pass a file path for a single-file check).

## Learned project notes
- `components/bq769x0` now uses an ultra-simple YAML config: required `cell_count` (fixed to 4) and `chemistry` (`liion_lipo`), with SOC defaults hardcoded in C++. 
- Component READMEs should present a single configuration example with optional items commented out instead of separate basic/full examples.
- `components/bq769x0` auto-loads sensor/binary_sensor/text_sensor/button to keep optional header includes available, and ships a local crc8 helper header so external builds don't need shared helpers (shared crc8 helper removed).
