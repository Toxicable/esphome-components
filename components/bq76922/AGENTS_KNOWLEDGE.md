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
- CC2 current units and stack/PACK/LD voltage units are auto-detected from `Settings:Configuration:DA Configuration` (`0x9303`) during setup.
- User preference for this component README: keep config simple and avoid jargon-heavy terms where possible (for example, explain `LD` as load-detect pin).
