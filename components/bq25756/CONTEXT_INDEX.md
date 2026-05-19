# bq25756 Context Index

## Read Order
1. `components/bq25756/AGENTS_KNOWLEDGE.md`
2. `components/bq25756/README.md`
3. `components/bq25756/__init__.py`
4. `components/bq25756/bq25756.h`
5. `components/bq25756/bq25756.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings.
- `bq25756.h`: component class surface, entity pointers, helper declarations.
- `bq25756.cpp`: register map, setup/update flow, control writes, ADC decode, status publishing.
- `README.md`: user-facing configuration and supported entities.
- `AGENTS_KNOWLEDGE.md`: active component invariants and datasheet-backed gotchas.

## Datasheet Notes
- Source PDF used for this component work was the repo-root `bq25756 (2).pdf`.
- The implementation relies on the BQ25756-specific register map, not the sibling `bq25798` layout.
