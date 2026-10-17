# AstralPool Halo Context Index

## Read order

1. `AGENTS_KNOWLEDGE.md`
2. `../../ARCHITECTURE.md`
3. `README.md`
4. `astral_ble_ph_orp.h` / `astral_ble_ph_orp.cpp`
5. `__init__.py`

## Edit map

- `astral_ble_ph_orp.cpp`: manufacturer-data filtering and provisional field decoding.
- `astral_ble_ph_orp.h`: sensor setters and BLE listener adapter.
- `__init__.py`: YAML schema, sensor metadata, and BLE tracker registration.
- `README.md`: installation, example YAML, payload layout, and validation caveat.
