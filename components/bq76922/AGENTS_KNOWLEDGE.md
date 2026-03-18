# AGENTS_KNOWLEDGE: bq76922

Component-scoped notes for `components/bq76922`.

- Use `components/bq76922/sluucg7.pdf` (TRM) as the command source of truth; the datasheet alone does not include full command/subcommand behavior.
- Direct commands used by this component come from TRM Table 12-1 and use little-endian values (`I2`/`U2`):
  - `0x12` Battery Status, `0x14..0x1D` cell voltages, `0x34/0x36/0x38` stack/PACK/LD, `0x3A` CC2 current, `0x62` Alarm Status, `0x68` internal temperature, `0x70` TS1 temperature, `0x7F` FET Status.
  - `0x70 TS1 Temperature()` returns 0.1K when TS1 is thermistor-configured; if TS1 is configured as ADCIN it returns TS1 pin voltage in mV.
- Subcommands are written little-endian to `0x3E/0x3F`; readback data comes from `0x40..`.
- `FET_EN` semantics (Manufacturing Status bit 4):
  - `1` = normal firmware/autonomous FET control
  - `0` = FET test mode (normal firmware FET control disabled)
- `0x0022 FET_ENABLE()` toggles `FET_EN` and is allowed only in `UNSEALED`/`FULLACCESS`.
- `CFETOFF`/`DFETOFF` hardware FET override details (TRM data-memory settings):
  - `Settings:Configuration:CFETOFF Pin Config` (`0x92FA`) and `...:DFETOFF Pin Config` (`0x92FB`) default to `0x00` (`PIN_FXN=0`, pin unused).
  - To use pin-based FET disable, set `PIN_FXN=2` on the relevant pin (`CFETOFF` or `DFETOFF/BOTHOFF`).
  - `OPT5` sets polarity: `0` active-high assert, `1` active-low assert.
  - `DFETOFF` `OPT4=1` selects `BOTHOFF` (asserting DFETOFF disables both CHG/PCHG and DSG/PDSG).
  - Asserting these pins disables FETs without serial commands; deasserting only allows re-enable if no other blocks are latched (for example host `FET_CONTROL`/`ALL_FETS_OFF` latch or active faults).
  - `REG18` is the internal 1.8V LDO rail and should only be used for internal circuitry/capacitor; for external pull-ups (for example CFETOFF/DFETOFF logic levels), use `REG1` or another valid external logic rail.
- Datasheet pin guidance when regulator chain is unused:
  - `REG1` can be left floating or tied to `VSS`.
  - `REGIN` should be tied to `VSS` if unused.
  - If both `BREG` and `REGIN` are unused, tie both to `VSS`.
  - Therefore, a board that hard-ties `REG1/REGIN/BREG` to `GND` is valid for a design not using the REG0/REG1 rails.
- `REG1` usage dependency:
  - `REG1` output requires `REGIN` around 5.5V.
  - If `REGIN` is not externally supplied, `REG0_EN` must be enabled and the external preregulator transistor driven by `BREG` must be present.
  - If `REGIN` is externally supplied, `BREG` is not used and should be tied to `REGIN`.
- Runtime power-path host commands are:
  - `0x0093` DSG/PDSG off, `0x0094` CHG/PCHG off, `0x0095` all off, `0x0096` all on.
- `power_path` control now checks `Manufacturing Status[FET_EN]` first; if `FET_EN=0` (test mode), control is rejected with a log warning.
- `power_path` now verifies CHG/DSG readback after command and rejects if device conditions keep FETs off; warning includes SS/PF bits.
- CC2 current units and stack/PACK/LD voltage units are auto-detected from `Settings:Configuration:DA Configuration` (`0x9303`) during setup.
- Optional boot-time current-limit settings are supported:
  - `charge_current_limit_a` writes `Protections OCC Threshold` (`0x9280`), 2mV/step, code range `2..62`.
  - `discharge_current_limit_a` writes `Protections OCD1 Threshold` (`0x9282`), 2mV/step, code range `2..100`.
  - `charge_current_delay_ms` writes `Protections OCC Delay` (`0x9281`) with code conversion `round(delay_ms / 3.3 - 2)`, clamped to `1..127`.
  - `discharge_current_delay_ms` writes `Protections OCD1 Delay` (`0x9283`) with code conversion `round(delay_ms / 3.3 - 2)`, clamped to `1..127`.
  - `current_recovery_time_s` writes `Protections:Recovery:Time` (`0x92AF`) in seconds (`0..255`).
  - Conversion uses `sense_resistor_milliohm` with `threshold_mV = current_A * shunt_mOhm`.
  - When charge/discharge limits are configured, the component also enables matching protection and FET-action bits:
    - `Settings:Protection:Enabled Protections A` (`0x9261`) sets OCC (bit 4) and/or OCD1 (bit 5).
    - `Settings:Protection:CHG FET Protections A` (`0x9265`) sets OCC bit (bit 4) for CHG turnoff.
    - `Settings:Protection:DSG FET Protections A` (`0x9269`) sets OCD1 bit (bit 5) for DSG turnoff.
  - Writes require `FULLACCESS` and are applied in `CONFIG_UPDATE` mode.
  - Entering `CONFIG_UPDATE` turns CHG/DSG FETs off briefly; if the host MCU is powered through that switched path,
    this can reset the MCU and look like an OTA rollback (boot not marked successful).
  - Runtime behavior: if ESP32 OTA rollback is active and the running image is `PENDING_VERIFY`, boot-time current-limit
    writes are deferred until OTA verification is complete, then applied once.
  - Before entering `CONFIG_UPDATE`, the component now pre-checks current-limit-related data memory values and skips
    re-apply when requested values already match (avoids unnecessary FET-off windows/resets).
- Public config key for top-of-stack voltage is `bat_voltage`; keep `stack_voltage` as backward-compatible alias.
- User preference for this component README: keep config simple and avoid jargon-heavy terms where possible (for example, explain `LD` as load-detect pin).
- User preference: allow `cell_count` configuration range `1..5` in this component schema.
- Cell voltage read mapping is auto-selected once at startup based on non-zero commands (>500mV), so common fewer-cell jumper layouts (for example 4S using Cell 1/2/3/5 commands) publish correctly.
