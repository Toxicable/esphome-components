# HUSB238 Context Index

## Read Order
1. `components/husb238/AGENTS_KNOWLEDGE.md`
2. `ARCHITECTURE.md`
3. `components/husb238/README.md`
4. `components/husb238/__init__.py`
5. `components/husb238/husb238_bus.h`
6. `components/husb238/husb238_protocol.h`
7. `components/husb238/husb238_protocol.cpp`
8. `components/husb238/husb238_service.h`
9. `components/husb238/husb238_service.cpp`
10. `components/husb238/husb238.h`
11. `components/husb238/husb238.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, entity wiring, codegen bindings.
- `husb238_bus.h`: host register bus boundary for reusable core code.
- `husb238_protocol.*`: register map, command constants, status/PDO decoding, response strings.
- `husb238_service.*`: reusable HUSB238 behavior built on `RegisterBus`.
- `husb238.h` / `husb238.cpp`: ESPHome component wrapper, logging, entities, and I2C adapter.
- `README.md`: user-facing configuration and supported entities.
- `AGENTS_KNOWLEDGE.md`: active component invariants and gotchas.
