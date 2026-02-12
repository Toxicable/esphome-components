# esphome-components

## LLM ingestion
Use `tools/pdf_to_text.py` to generate `<pdf>.txt` from PDFs, and provide the `.txt` file to LLMs instead of the PDF.

## Checks
- `./check.bash` runs `clangd --check` on C/C++ headers/sources (or pass a specific file path).
- `./check_py.bash` runs Python syntax checks via `py_compile` without creating `__pycache__` files in the repo.
