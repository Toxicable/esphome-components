# programmable_load Context Index

## Read Order
1. `components/programmable_load/AGENTS_KNOWLEDGE.md`
2. `components/programmable_load/README.md`
3. `components/programmable_load/__init__.py`
4. `components/programmable_load/programmable_load.h`
5. `components/programmable_load/programmable_load.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings.
- `programmable_load.h`: Component class surface, entity pointers, helper declarations, setpoint number class.
- `programmable_load.cpp`: Control loop, safety checks, DCR estimation, fan PWM, state publishing.
- `README.md`: User-facing configuration example and notes.
- `AGENTS_KNOWLEDGE.md`: Active component invariants and gotchas.

## Architecture
- Monolith component (single `programmable_load:` YAML block, no split platform modules).
- Two internal intervals: tight control loop (configurable, default 50ms) and slow update (500ms for fan/publishing).
- No I2C or UART dependency — the component receives external sensor/output references and drives DAC/fan outputs.
