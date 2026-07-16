# programmable_load Context Index

## Read Order
1. `components/programmable_load/AGENTS_KNOWLEDGE.md`
2. `components/programmable_load/README.md`
3. `components/programmable_load/__init__.py`
4. `components/programmable_load/_schema.py`
5. `components/programmable_load/_codegen.py`
6. `components/programmable_load/_actions.py`
7. `components/programmable_load/programmable_load_core.h`
8. `components/programmable_load/calibration.h`
9. `components/programmable_load/procedure.h`
10. `components/programmable_load/dcr_test.h`
11. `components/programmable_load/battery_cycle.h`
12. `components/programmable_load/programmable_load.h`
13. `components/programmable_load/programmable_load.cpp`

## Edit Map
- `__init__.py`: Small public ESPHome facade; imports the private schema, codegen and action modules.
- `_schema.py` / `_types.py`: YAML validation, entity schemas, and codegen type declarations.
- `_codegen.py`: Component/entity/procedure construction and typed charger wiring.
- `_actions.py` / `calibration_actions.h`: Atomic calibration apply/reset automation actions.
- `programmable_load_core.h` / `.cpp`: Host-independent state, ownership lock, fault aggregation, calibration validation and safety calculations.
- `load_types.h`: Compatibility aliases used by the ESPHome facade and existing procedures.
- `calibration.h`: Host-independent persisted calibration record, source and version.
- `procedure.h`: Pure procedure boundary between the core and optional tests.
- `dcr_test.h` / `dcr_test.cpp`: Explicit DCR procedure and start-button entity.
- `battery_cycle.h` / `battery_cycle.cpp`: Full discharge/rest/Charger_14 recharge procedure, integration, progress and results.
- `programmable_load.h`: Component class surface, calibration, ownership, typed charger capability, and generated entities.
- `programmable_load.cpp`: Core control, limits, cooling, state/fault publishing, typed charger mutual exclusion, and procedure coordination.
- `README.md`: User-facing configuration example and safety/ownership notes.
- `AGENTS_KNOWLEDGE.md`: Active component invariants and gotchas.
- `test_config.yaml`: Full ESPHome compile fixture including BQ25756-backed Charger_14 cycle wiring.

## Architecture
- One public `programmable_load:` YAML block with private Python implementation modules; there are no extra top-level component platforms.
- One configurable control loop owns measurement updates, limits, output control, cooling, typed charger enable, and status publishing.
- Procedures receive a `ProcedureContext` and return a `ProcedureResult`; they never call the core.
- Charger support uses `component_common::ChargerInterface`; BQ25756 entities are optional observers, not the internal API. The onboard STM32 firmware path remains separate.
