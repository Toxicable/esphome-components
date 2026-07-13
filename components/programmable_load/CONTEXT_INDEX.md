# programmable_load Context Index

## Read Order
1. `components/programmable_load/AGENTS_KNOWLEDGE.md`
2. `components/programmable_load/README.md`
3. `components/programmable_load/__init__.py`
4. `components/programmable_load/load_types.h`
5. `components/programmable_load/calibration.h`
6. `components/programmable_load/procedure.h`
7. `components/programmable_load/dcr_test.h`
8. `components/programmable_load/programmable_load.h`
9. `components/programmable_load/dcr_test.cpp`
10. `components/programmable_load/programmable_load.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings.
- `load_types.h`: Core state, fault, measurement, and configuration data types.
- `calibration.h`: Persisted calibration record and version.
- `procedure.h`: Procedure boundary between the core and optional tests.
- `dcr_test.h` / `dcr_test.cpp`: Explicit DCR procedure and start-button entity.
- `programmable_load.h`: Component class surface, calibration, ownership, and generated entities.
- `programmable_load.cpp`: Core control, limits, cooling, state publishing, and the DCR implementation inclusion required by ESPHome's source discovery.
- `README.md`: User-facing configuration example and notes.
- `AGENTS_KNOWLEDGE.md`: Active component invariants and gotchas.

## Architecture
- Monolith component (single `programmable_load:` YAML block, no split platform modules).
- One configurable control loop owns measurement updates, limits, output control, cooling, and status publishing.
- No I2C or UART dependency — the component receives external sensor/output references and drives DAC/fan outputs.
