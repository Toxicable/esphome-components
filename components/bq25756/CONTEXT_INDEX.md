# bq25756 Context Index

## Read Order
1. `components/bq25756/AGENTS_KNOWLEDGE.md`
2. `ARCHITECTURE.md`
3. `components/component_common/AGENTS_KNOWLEDGE.md`
4. `components/bq25756/README.md`
5. `components/bq25756/__init__.py`
6. `components/bq25756/bq25756_bus.h`
7. `components/bq25756/bq25756_protocol.h`
8. `components/bq25756/bq25756_protocol.cpp`
9. `components/bq25756/bq25756_service.h`
10. `components/bq25756/bq25756_service.cpp`
11. `components/bq25756/bq25756.h`
12. `components/bq25756/bq25756.cpp`
13. `tests/bq25756_service_test.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings, and `component_common` auto-load.
- `bq25756_bus.h`: host register bus boundary for reusable core code.
- `bq25756_protocol.*`: register map, status decoding, ADC decoding, limit encoders, and typed common fields.
- `bq25756_service.*`: reusable BQ25756 behavior built on `RegisterBus` and common endian/masked-register helpers.
- `bq25756.h` / `bq25756.cpp`: ESPHome component wrapper, logging, entities, and I2C adapter.
- `README.md`: user-facing configuration and supported entities.
- `AGENTS_KNOWLEDGE.md`: active component invariants and datasheet-backed gotchas.
- `tests/bq25756_service_test.cpp`: fake-bus tests for host-independent service behavior.

## Datasheet Notes
- Source PDF used for this component work was the repo-root `bq25756 (2).pdf`.
- The implementation relies on the BQ25756-specific register map, not the sibling `bq25798` layout.
