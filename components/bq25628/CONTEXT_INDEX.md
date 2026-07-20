# bq25628 Context Index

## Read Order

1. `AGENTS_KNOWLEDGE.md`
2. `README.md`
3. `__init__.py`
4. `bq25628_registers.h`
5. `bq25628_protocol.*`
6. `bq25628_bus.h`
7. `bq25628_service.*`
8. `bq25628.h` / `bq25628.cpp`
9. `test_config.yaml`

## Edit Map

- `__init__.py`: ESPHome YAML schema and entity binding.
- `bq25628_registers.h`: typed register manifest, ownership masks, and fields.
- `bq25628_protocol.*`: register decoding and ADC-value conversion.
- `bq25628_bus.h` / `bq25628_service.*`: host-independent register transport and behavior.
- `bq25628.h` / `bq25628.cpp`: ESPHome I2C adapter and entity publication.
- `README.md`: supported configuration and telemetry behavior.
- `test_config.yaml`: compile fixture.
