# AGENTS_KNOWLEDGE: bq769x0

Component-scoped notes for `components/bq769x0`.

- Uses an ultra-simple YAML config: required `cell_count` (fixed to 4) and `chemistry` (`liion_lipo`), with SOC defaults hardcoded in C++.
- Auto-loads sensor/binary_sensor/select/button to keep optional header includes available.
- Ships a local crc8 helper header so external builds don't need shared helpers (shared crc8 helper removed).
- Exposes `mode` as a select that writes CHG_ON/DSG_ON with options `standby`, `charge`, `discharge`, and `charge+discharge`.
- Replaces `fault`/`device_ready` binary sensors with an `alerts` text sensor (`none`, `protection`, `device`, `protection+device`) and adds `power_path_state` for live CHG/DSG state; `power_path` select uses `off`, `charge`, `discharge`, `bidirectional`.
- For 4S BQ76920 wiring, maps cells to VC1/VC2/VC3/VC5 and expects VC4 shorted to VC3 per TI Table 9-2.
- CRC-enabled BQ769x0 variants (e.g., BQ7692003) require CRC reads; auto-detection should try CRC first and fall back to non-CRC.
- Quick CRC sanity check: if VCx_LO equals `crc8([read_addr, VCx_HI])`, you're reading CRC as data (CRC mode needed).
