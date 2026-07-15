# esphome-components

## Repository documentation

- [`ARCHITECTURE.md`](ARCHITECTURE.md) defines component classes, layering, configuration/state/fault contracts, entity exposure, and the ESPHome external-component constraints that govern shared code.
- [`COMPONENTS.md`](COMPONENTS.md) records the lifecycle status, known consumers, and intended direction of every component.

Reusable chip components may split host-independent protocol/service code from the ESPHome wrapper. Small adapters should remain simple; the architecture is a decision guide rather than a mandatory file count.

## LLM ingestion

Use `tools/pdf_to_text.py` to generate `<pdf>.txt` from PDFs, and provide the `.txt` file to LLMs instead of the PDF. The tool strips embedded `\x00` bytes automatically so the output stays searchable as plain text.

## Checks

- `./check.bash` runs `clangd --check` on C/C++ headers/sources (or pass a specific file path).
- `./check_py.bash` runs Python syntax checks via `py_compile` without creating `__pycache__` files in the repo.
- The devcontainer pins ESPHome and persists VS Code/code-server plus PlatformIO state in named volumes so Coder rebuilds do not lose extensions or re-download ESP-IDF tooling.

Syntax checks do not verify ESPHome source discovery. Changes to shared helpers, `AUTO_LOAD`, public YAML, or external-component allowlists should also compile representative real consumer configurations.

## README conventions

- In component README YAML examples, entity `name` values should not include the component name prefix (for example, `Temperature` instead of `LPS25HB Temperature`).
- In component README YAML examples, use `##` for section/explanatory comment lines and single `#` for commented config keys so optional blocks can be uncommented quickly.
- Prefer monolith component examples (`<component_name>:` with nested optional entities) over split `platform:` examples for component-specific entities.
