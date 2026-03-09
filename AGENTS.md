## Continuous learning
- When you learn new project knowledge, coding style, or preferences during a session, update `AGENTS.md` (and `README.md` if it affects users) before finishing so the next agent benefits.

## Repo tips
- Source components live under `components/`; prefer scoped changes there unless asked otherwise.
- Run `./check.bash` to perform `clangd --check` over C/C++ sources (or pass a file path for a single-file check).
- Run `./check_py.bash` to syntax-check Python files via `py_compile` without creating `__pycache__` artifacts in the repo.

## Learned project notes
- `components/bq769x0` now uses an ultra-simple YAML config: required `cell_count` (fixed to 4) and `chemistry` (`liion_lipo`), with SOC defaults hardcoded in C++. 
- Component READMEs should present a single configuration example with optional items commented out instead of separate basic/full examples.
- `components/bq769x0` auto-loads sensor/binary_sensor/select/button to keep optional header includes available, and ships a local crc8 helper header so external builds don't need shared helpers (shared crc8 helper removed).
- `components/bq769x0` exposes `mode` as a select that writes CHG_ON/DSG_ON with options `standby`, `charge`, `discharge`, and `charge+discharge`.
- `components/bq769x0` replaces `fault`/`device_ready` binary sensors with an `alerts` text sensor (`none`, `protection`, `device`, `protection+device`) and adds `power_path_state` for live CHG/DSG state; `power_path` select now uses `off`, `charge`, `discharge`, `bidirectional`.
- For 4S BQ76920 wiring, the component maps cells to VC1/VC2/VC3/VC5 and expects VC4 shorted to VC3 per TI Table 9-2.
- CRC-enabled BQ769x0 variants (e.g., BQ7692003) require CRC reads; auto-detection should try CRC first and fall back to non-CRC.
- Quick CRC sanity check: if VCx_LO equals crc8([read_addr, VCx_HI]), you're reading CRC as data (CRC mode needed).
- Prefer `std::numeric_limits<float>::quiet_NaN()` (with `<limits>`) in headers instead of `NAN` to avoid macro ordering issues.
- Validate any chip assumptions against the datasheet before implementing or documenting behavior.
- `tools/pdf_to_text.py` extracts verbatim text from text-based PDFs using `pypdf`, writing `<pdf>.txt` (input-only CLI).
- Devcontainer installs `pypdf` alongside `esphome` for the PDF ingestion tool.
- Devcontainer shells default to the ESP-IDF Python venv; Python deps needed at runtime should be installed into that env (Dockerfile sources `export.sh` before installing).
- `components/fdc1004` is a `sensor` platform with optional `cin1`..`cin4` sensors; each channel supports optional `capdac` (0..31 steps at 3.125pF/step), `sample_rate` accepts 100/200/400 S/s, init retries if the device is unavailable at boot, optional `zero_now` button taring to current readings, and optional `cin*_offset` sensors for live tare offsets.
- `components/bq25798` is a top-level polling component defaulting to address `0x6B`, `update_interval: 1s`, `disable_watchdog: true`; it can publish ADC sensors (`ibus_current`, `ibat_current`, `vbus_voltage`, `vbat_voltage`, `vsys_voltage`, `ts_percent`, `die_temperature`), text status sensors (`charge_status`, `vbus_status`, `status_flags`), split status binary sensors (`pg_good`, `vbus_present`, `vbat_present`, `watchdog_expired`, `iindpm_active`, `vindpm_active`, `thermal_regulation`, `vsys_regulation`, `ts_cold`, `ts_cool`, `ts_warm`, `ts_hot`), controls (`charge_enable`, `hiz_mode`, `otg_mode`, `watchdog` select), and buttons (`watchdog_reset`, `dump_registers`) while still logging status bytes `0x1B..0x1F` and ADC raws.
- `components/bq25798` flag entities (`status_flags` text + split status binary sensors) should use `entity_category: diagnostic`.
- ESPHome `i2c::I2CDevice::write_read()` returns `i2c::ErrorCode` (not bool); always compare to `i2c::ERROR_OK` or reads will be inverted.
- Component READMEs should include an `external_components` snippet; add an explicit `i2c` block when the component depends on I2C.
- README `external_components` examples should use `source: github://Toxicable/esphome-components@main` with `refresh: 0s`, and list the specific component in `components: [ ... ]`.
- `components/mcf8316d_manual` adds an ESP-IDF I2C manual validation flow for MCF8316D with ALGO_DEBUG1 override speed control, default-safe boot state (speed 0 + brake on + hardware direction), fault-triggered speed shutdown, and no EEPROM writes.
- ESPHome `number.number_schema()` accepts metadata only (no `min_value`/`max_value`/`step`); bounds belong in `number.new_number(...)`.
- `components/mcf8316d_manual` should follow the repo pattern and use ESPHome `i2c::I2CDevice` APIs (`write`, `read`, `write_read`) instead of direct `driver/i2c.h` calls; `inter_byte_delay_us` is currently informational-only.
- `components/mcf8316d_manual` VM voltage decode currently uses an 11-bit field from `REG_VM_VOLTAGE` (`bits 26:16`) with scaling `60/2048`, and the debug log prints both `adc8` and `adc_q11` to help validate against measured VM.
- `components/mcf8316d_manual` `fault_active` should mirror asserted `nFAULT` behavior using gate + controller summary bits (`GATE_DRIVER_FAULT_STATUS[31]` OR `CONTROLLER_FAULT_STATUS[31]`), and controller detail bits are at their datasheet positions (for example `WATCHDOG_FAULT` is bit 3, not bit 1).
- `components/mcf8316d_manual` `fault_summary` now combines decoded gate-driver + controller fault bits, and the component logs transitions as `Active faults: ...` / `Faults cleared` even without a configured text sensor.
- `components/mcf8316d_manual` clear-fault flow now logs before/after gate+controller raw status and republishes `fault_summary` immediately; if only aggregate bits are set it uses fallback labels `DRV_FAULT_ACTIVE` / `CTRL_FAULT_ACTIVE` instead of `none`.
- `components/mcf8316d_manual` MPET startup diagnostics now log `ALGORITHM_STATE`, `ALGO_DEBUG2` MPET control bits, `ALGO_STATUS_MPET` completion bits, and `MTR_PARAMS`; these logs emit once at setup and periodically while MPET faults are active.
- `components/mcf8316d_manual` setup now force-clears `ALGO_DEBUG2` MPET bits (`MPET_CMD/R/L/KE/MECH/WRITE_SHADOW`) and triggers a startup fault clear if MPET fault bits are latched, to keep manual validation from auto-entering MPET.
- `components/mcf8316d_manual/mcf8316d.pdf` is the local MCF8316D datasheet source; extracted Markdown is tracked at `components/mcf8316d_manual/mcf8316d.txt` (page breaks rendered as `---`).
