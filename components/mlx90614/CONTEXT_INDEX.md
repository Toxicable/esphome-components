# MLX90614 Context Index

## Read order

1. `AGENTS_KNOWLEDGE.md`
2. `../../ARCHITECTURE.md`
3. `README.md`
4. `mlx90614_registers.h`
5. `mlx90614.h`
6. `mlx90614.cpp`
7. `__init__.py`

## Edit map

- `mlx90614_registers.h`: typed temperature-register IDs and metadata.
- `mlx90614.h` / `mlx90614.cpp`: SMBus PEC transport, conversion and entity publication.
- `__init__.py`: schema and shared-helper loading.
- `test_config.yaml`: compile coverage including dual-zone `object2`.
