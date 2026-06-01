# esc_higher Context Index

## Read Order
1. `components/esc_higher/AGENTS_KNOWLEDGE.md`
2. `components/esc_higher/i2c_interface.md`
3. `components/esc_higher/README.md`
4. `components/esc_higher/__init__.py`
5. `components/esc_higher/esc_higher.h`
6. `components/esc_higher/esc_higher.cpp`

## Edit Map
- `__init__.py`: ESPHome schema and codegen wiring for the top-level `esc_higher:` block.
- `esc_higher.h`: C++ component class surface.
- `esc_higher.cpp`: runtime setup and config logging behavior.
- `README.md`: user-facing YAML configuration example.
- `i2c_interface.md`: protocol source-of-truth for command/response format.
- `AGENTS_KNOWLEDGE.md`: active component invariants for future edits.
