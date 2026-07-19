# esc_higher Context Index

## Read order

1. `AGENTS_KNOWLEDGE.md`
2. `../../ARCHITECTURE.md`
3. `i2c_interface.md`
4. `i2c_guideline.md`
5. `README.md`
6. `esc_higher_registers.h`
7. `__init__.py`
8. `esc_higher.h`
9. `esc_higher_text.h`
10. `esc_higher.cpp`

## Edit map

- `esc_higher_registers.h`: typed block-register and command IDs, transfer sizes and compile-time validation.
- `esc_higher.cpp`: ESPHome I2C transport, command sequencing, decoding and publication.
- `esc_higher.h`: component/entity surface and non-wire policy constants.
- `esc_higher_text.h`: enum and fault text mappings.
- `i2c_interface.md` / `i2c_guideline.md`: STM32 wire-protocol source of truth.
- `__init__.py`: schema and shared-helper loading.
- `test_config.yaml`: pinned compile fixture.
