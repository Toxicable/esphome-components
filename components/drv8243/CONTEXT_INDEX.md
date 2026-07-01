# DRV8243 Context Index

## Read Order
1. `components/drv8243/AGENTS_KNOWLEDGE.md`
2. `ARCHITECTURE.md`
3. `components/drv8243/README.md`
4. `components/drv8243/__init__.py`
5. `components/drv8243/drv8243_bus.h`
6. `components/drv8243/drv8243_protocol.h`
7. `components/drv8243/drv8243_protocol.cpp`
8. `components/drv8243/drv8243_service.h`
9. `components/drv8243/drv8243_service.cpp`
10. `components/drv8243/drv8243.h`
11. `components/drv8243/drv8243.cpp`

## Edit Map
- `__init__.py`: ESPHome schema, LEDC output wiring, codegen bindings.
- `drv8243_bus.h`: host pin/timing boundary for reusable handshake behavior.
- `drv8243_protocol.*`: handshake result strings and host-independent output shaping.
- `drv8243_service.*`: reusable nSLEEP/nFAULT handshake and static polarity operations.
- `drv8243.h` / `drv8243.cpp`: ESPHome wrapper, GPIO/output adapters, logging, and entity behavior.
- `README.md`: user-facing configuration and supported modes.
- `AGENTS_KNOWLEDGE.md`: active component invariants and gotchas.
