# esc_higher Context Index

## Read Order
1. `components/esc_higher/AGENTS_KNOWLEDGE.md`
2. `components/esc_higher/i2c_interface.md`
3. `components/esc_higher/i2c_guideline.md`
4. `components/esc_higher/README.md`
5. `components/esc_higher/__init__.py`
6. `components/esc_higher/esc_higher.h`
7. `components/esc_higher/esc_higher_text.h` (text sensor mappings)
8. `components/esc_higher/esc_higher.cpp`

## Edit Map
- `__init__.py`: ESPHome schema and codegen wiring for the top-level `esc_higher:` block.
- `esc_higher.h`: C++ component class surface.
- `esc_higher_text.h`: enum-to-string and bitmask-to-names mappings for text sensors.
- `esc_higher.cpp`: I2C transport, register reads/writes, and state publishing.
- `README.md`: user-facing YAML configuration example.
- `i2c_interface.md`: protocol source-of-truth for command/response format.
- `i2c_guideline.md`: canonical field offsets/types for register payloads.
- `AGENTS_KNOWLEDGE.md`: active component invariants for future edits.
