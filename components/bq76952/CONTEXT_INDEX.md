# CONTEXT_INDEX: bq76952

Read order for `components/bq76952`:

1. `AGENTS_KNOWLEDGE.md`
2. `__init__.py`
3. `bq76952_soc.h` / `bq76952_soc.cpp`
4. `bq76952_service.h` / `bq76952_service.cpp`
5. `bq76952.h` / `bq76952.cpp`
6. `README.md`

Edit map:

- `__init__.py`: ESPHome schema, entity wiring, YAML surface.
- `bq76952_soc.*`: SoC learning, persisted endpoints, capacity-calibration status. SoC must retain voltage fallback until both measured endpoints establish a coulomb-count span.
- `bq76952_service.*`: device service, measurements, and SoC ownership.
- `bq76952.h` / `bq76952.cpp`: ESPHome entity facade and entity publication.
- `README.md`: user-facing config and behavioral notes.

External references:

- Canonical datasheet: `bq76952.pdf`
