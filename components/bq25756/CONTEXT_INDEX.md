# bq25756 Context Index

## Read Order
1. `components/bq25756/AGENTS_KNOWLEDGE.md`
2. `ARCHITECTURE.md`
3. `components/bq25756/README.md`
4. `components/bq25756/__init__.py`
5. `components/bq25756/bq25756_bus.h`
6. `components/bq25756/bq25756_protocol.h`
7. `components/bq25756/bq25756_protocol.cpp`
8. `components/bq25756/bq25756_service.h`
9. `components/bq25756/bq25756_service.cpp`
10. `components/bq25756/bq25756.h`
11. `components/bq25756/bq25756.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings.
- `bq25756_bus.h`: host register bus boundary for reusable core code.
- `bq25756_protocol.*`: register map, status decoding, ADC decoding, limit encoders.
- `bq25756_service.*`: reusable BQ25756 behavior built on `RegisterBus`.
- `bq25756.h` / `bq25756.cpp`: ESPHome component wrapper, logging, entities, and I2C adapter.
- `README.md`: user-facing configuration and supported entities.
- `AGENTS_KNOWLEDGE.md`: active component invariants and datasheet-backed gotchas.

## Datasheet Notes
- Source PDF used for this component work was the repo-root `bq25756 (2).pdf`.
- The implementation relies on the BQ25756-specific register map, not the sibling `bq25798` layout.
