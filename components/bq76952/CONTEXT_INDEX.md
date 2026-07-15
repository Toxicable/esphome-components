# CONTEXT_INDEX: bq76952

Read order for `components/bq76952`:

1. `AGENTS_KNOWLEDGE.md`
2. `__init__.py`, then `_schema.py`, `_types.py`, `_codegen.py`
3. `bq76952_registers.h` and `bq76952_status.h` / `.cpp`
4. `bq76952_i2c_transport.h` / `.cpp`
5. `bq76952_soc.h` / `.cpp`
6. `bq76952_service.h` / `.cpp`
7. `bq76952.h` / `.cpp`
8. `README.md`

Edit map:

- `__init__.py`: public ESPHome loader metadata and re-exported schema/codegen entry points.
- `_schema.py`: YAML keys, schemas, entity exposure, and cross-field validation.
- `_types.py`: ESPHome code-generation declarations for C++ structs/classes.
- `_codegen.py`: typed config construction and entity/component wiring.
- `bq76952_registers.h`: host-independent chip register map and encodings.
- `bq76952_status.*`: host-independent lifecycle/operating/fault snapshot decoding and formatting.
- `bq76952_i2c_transport.*`: ESPHome I2C transactions and BQ transfer framing.
- `bq76952_soc.*`: SoC learning, persisted endpoints, and capacity-calibration status.
- `bq76952_service.*`: desired-state synchronization, measurements, controls, and SoC ownership.
- `bq76952.h` / `.cpp`: ESPHome lifecycle and entity publication.
- `README.md`: user-facing config and behavioral notes.

External references:

- Canonical datasheet: `bq76952.pdf`
