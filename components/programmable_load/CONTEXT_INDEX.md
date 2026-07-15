# programmable_load Context Index

## Read Order
1. `components/programmable_load/AGENTS_KNOWLEDGE.md`
2. `components/programmable_load/README.md`
3. `components/programmable_load/__init__.py`
4. `components/programmable_load/load_types.h`
5. `components/programmable_load/calibration.h`
6. `components/programmable_load/procedure.h`
7. `components/programmable_load/dcr_test.h`
8. `components/programmable_load/battery_cycle.h`
9. `components/programmable_load/programmable_load.h`
10. `components/programmable_load/dcr_test.cpp`
11. `components/programmable_load/battery_cycle.cpp`
12. `components/programmable_load/programmable_load.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, typed charger-component wiring, entity creation, and codegen bindings.
- `load_types.h`: Core state, fault, load measurements, shared charger aliases, procedure context, and procedure result types.
- `calibration.h`: Persisted calibration record and version.
- `procedure.h`: Pure procedure boundary between the core and optional tests.
- `dcr_test.h` / `dcr_test.cpp`: Explicit DCR procedure and start-button entity.
- `battery_cycle.h` / `battery_cycle.cpp`: Full discharge/rest/Charger_14 recharge procedure, integration, progress and results.
- `programmable_load.h`: Component class surface, calibration, ownership, typed charger capability, and generated entities.
- `programmable_load.cpp`: Core control, limits, cooling, state/fault publishing, typed charger mutual exclusion, and procedure coordination.
- `README.md`: User-facing configuration example and safety/ownership notes.
- `AGENTS_KNOWLEDGE.md`: Active component invariants and gotchas.
- `test_config.yaml`: Full ESPHome compile fixture including BQ25756-backed Charger_14 cycle wiring.

## Architecture
- Monolith component: one `programmable_load:` YAML block, no split platform modules.
- One configurable control loop owns measurement updates, limits, output control, cooling, typed charger enable, and status publishing.
- Procedures receive a `ProcedureContext` and return a `ProcedureResult`; they never call the core.
- Charger support uses `component_common::ChargerInterface`; BQ25756 entities are optional observers, not the internal API. The onboard STM32 firmware path remains separate.
