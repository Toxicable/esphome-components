# esphome-components

## LLM ingestion
Use `tools/pdf_to_text.py` to generate `<pdf>.txt` from PDFs, and provide the `.txt` file to LLMs instead of the PDF.

## Checks
- `./check.bash` runs `clangd --check` on C/C++ headers/sources (or pass a specific file path).
- `./check_py.bash` runs Python syntax checks via `py_compile` without creating `__pycache__` files in the repo.

## README conventions
- In component README YAML examples, entity `name` values should not include the component name prefix (for example, `Temperature` instead of `LPS25HB Temperature`).
- In component README YAML examples, use `##` for section/explanatory comment lines and single `#` for commented config keys so optional blocks can be uncommented quickly.
- Prefer monolith component examples (`<component_name>:` with nested optional entities) over split `platform:` examples for component-specific entities.
