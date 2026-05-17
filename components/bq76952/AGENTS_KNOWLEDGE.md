# AGENTS_KNOWLEDGE: bq76952

Component-scoped notes for `components/bq76952`.

- Use `components/bq76952/bq76952.pdf` as the local source of truth for this component unless a TI TRM PDF is later checked into this component directory.
- This integration intentionally mirrors the existing `bq76922` feature set and command style, but extends direct cell-voltage reads through Cell 16 for `cell_count: 3..16`.
- The direct cell-voltage commands are treated as contiguous from `0x14` through `0x32` in 2-byte steps; top-of-stack, PACK, LD, current, die temperature, TS1/TS2/TS3, alarm status, and FET status follow the same command usage pattern as the existing BQ769x2 component.
- Cell sensor publishing follows the first `cell_count` populated differential cell-voltage commands seen at startup in ascending order, which supports sparse layouts like `VC1-VC0`, `VC2-VC1`, `VC3-VC2`, `VC16-VC15`.
- Configured `ts1_temperature`/`ts2_temperature`/`ts3_temperature` entities auto-program the corresponding BQ pin into thermistor mode at boot (report-only measurement type) with an `18k` or `180k` pull-up selected from YAML; these writes require `FULLACCESS` and use `CONFIG_UPDATE`.
- TS1/TS2/TS3 config register `OPT[5:0]` fields live in bits `7:2`; build thermistor config bytes by encoding pull-up/polynomial/measurement in `OPT`, then shifting left by 2 before OR-ing `PIN_FXN=3`.
- `reg0_enabled`, `reg1_enabled`, and `reg1_voltage` program `REG0 Config (0x9237)` and `REG12 Config (0x9236)` at boot; if a live REG1 voltage change is requested while REG1 is already on, the component stages the write by disabling REG1 first, then applying the new voltage and enable state.
- After any live `REG12 Config (0x9236)` change, send `REG12_CONTROL()/0x0098` with the full updated byte so REG1 runtime state matches the data-memory setting, even when only the voltage bits changed.
- `REG12 Config (0x9236)` encodes `REG1_EN` in bit `0` and `REG1V[2:0]` in bits `3:1` (not bits `3` and `2:0` respectively). YAML voltage option values and C++ masks/shifts must match that layout.
- Coulomb-counter accumulation is exposed from `DASTATUS6 (0x0076)` as signed `userAh` plus a 32-bit fractional term and converted through the auto-detected `userA` scale; `RESET_PASSQ (0x0082)` is exposed as a manual button, not a boot-time automatic reset.
- Keep the YAML monolithic in `__init__.py`; do not split this component into platform modules unless the repo-wide preference changes.
- Document `i2c_id` in examples; ESPHome requires it for this component when a node defines more than one I2C bus.
- If you need an extracted text view for ad hoc searching, generate it from `components/bq76952/bq76952.pdf`; the PDF remains canonical.
