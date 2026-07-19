# HUSB238 Context Index

## Read order

1. `AGENTS_KNOWLEDGE.md`
2. `../../ARCHITECTURE.md`
3. `README.md`
4. `__init__.py`
5. `husb238_registers.h`
6. `husb238_bus.h`
7. `husb238_protocol.*`
8. `husb238_service.*`
9. `husb238.h` / `husb238.cpp`

## Edit map

- `husb238_registers.h`: register/command IDs, addresses, widths, names and validation.
- `husb238_protocol.*`: decoding and unit conversion with no transport dependency.
- `husb238_bus.h`: raw numeric-address boundary implemented by the platform wrapper.
- `husb238_service.*`: typed register/command operations and device behaviour.
- `husb238.h` / `husb238.cpp`: ESPHome entities, logging, scheduling and I2C adaptation.
- `__init__.py`: YAML schema and `component_common` loading.
- `test_config.yaml`: pinned ESPHome compile fixture.
