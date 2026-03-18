# AGENTS_KNOWLEDGE: bq76922

Component-scoped notes for `components/bq76922`.

- Use `components/bq76922/sluucg7.pdf` (TRM) as the command source of truth; the datasheet alone does not include full command/subcommand behavior.
- Direct commands used by this component come from TRM Table 12-1 and use little-endian values (`I2`/`U2`):
  - `0x12` Battery Status, `0x14..0x1D` cell voltages, `0x34/0x36/0x38` stack/PACK/LD, `0x3A` CC2 current, `0x62` Alarm Status, `0x68` internal temperature, `0x7F` FET Status.
- Subcommands are written little-endian to `0x3E/0x3F`; readback data comes from `0x40..`.
- `FET_EN` semantics (Manufacturing Status bit 4):
  - `1` = normal firmware/autonomous FET control
  - `0` = FET test mode (normal firmware FET control disabled)
- `0x0022 FET_ENABLE()` toggles `FET_EN` and is allowed only in `UNSEALED`/`FULLACCESS`.
- Runtime power-path host commands are:
  - `0x0093` DSG/PDSG off, `0x0094` CHG/PCHG off, `0x0095` all off, `0x0096` all on.
- `power_path` control now checks `Manufacturing Status[FET_EN]` first; if `FET_EN=0` (test mode), control is rejected with a log warning.
- `power_path` now verifies CHG/DSG readback after command and rejects if device conditions keep FETs off; warning includes SS/PF bits.
- CC2 current units and stack/PACK/LD voltage units are auto-detected from `Settings:Configuration:DA Configuration` (`0x9303`) during setup.
- Optional boot-time current-limit settings are supported:
  - `charge_current_limit_a` writes `Protections OCC Threshold` (`0x9280`), 2mV/step, code range `2..62`.
  - `discharge_current_limit_a` writes `Protections OCD1 Threshold` (`0x9282`), 2mV/step, code range `2..100`.
  - Conversion uses `sense_resistor_milliohm` with `threshold_mV = current_A * shunt_mOhm`.
  - Writes require `FULLACCESS` and are applied in `CONFIG_UPDATE` mode.
- Public config key for top-of-stack voltage is `bat_voltage`; keep `stack_voltage` as backward-compatible alias.
- User preference for this component README: keep config simple and avoid jargon-heavy terms where possible (for example, explain `LD` as load-detect pin).
- User preference: allow `cell_count` configuration range `1..5` in this component schema.
- Cell voltage read mapping is auto-selected once at startup based on non-zero commands (>500mV), so common fewer-cell jumper layouts (for example 4S using Cell 1/2/3/5 commands) publish correctly.
