# AGENTS_KNOWLEDGE: bq76952

Component-scoped notes for `components/bq76952`.

- Use `components/bq76952/bq76952.pdf` as the local source of truth for this component unless a TI TRM PDF is later checked into this component directory.
- This integration intentionally mirrors the existing `bq76922` feature set and command style, but extends direct cell-voltage reads through Cell 16 for `cell_count: 3..16`.
- The direct cell-voltage commands are treated as contiguous from `0x14` through `0x32` in 2-byte steps; top-of-stack, PACK, LD, current, die temperature, TS1/TS2/TS3, alarm status, and FET status follow the same command usage pattern as the existing BQ769x2 component.
- Cell sensor publishing follows the first `cell_count` populated differential cell-voltage commands seen at startup in ascending order, which supports sparse layouts like `VC1-VC0`, `VC2-VC1`, `VC3-VC2`, `VC16-VC15`.
- Configured `ts1_temperature`/`ts2_temperature`/`ts3_temperature` entities auto-program the corresponding BQ pin into thermistor mode at boot (report-only measurement type) with an `18k` or `180k` pull-up selected from YAML; these writes require `FULLACCESS` and use `CONFIG_UPDATE`.
- Keep the YAML monolithic in `__init__.py`; do not split this component into platform modules unless the repo-wide preference changes.
- Document `i2c_id` in examples; ESPHome requires it for this component when a node defines more than one I2C bus.
- If you need an extracted text view for ad hoc searching, generate it from `components/bq76952/bq76952.pdf`; the PDF remains canonical.
