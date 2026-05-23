# AGENTS_KNOWLEDGE: bq25756

Component-scoped notes for `components/bq25756`.

- Top-level polling component defaults: address `0x6B`, `update_interval: 1s`, `disable_watchdog: true`.
- `event_logging` defaults to `true` and emits concise `INFO` event lines when charger status/fault bytes change (not every polling interval).
- Setup validates `REG0x3D` `PART_NUM[6:3] == 0b0010` before treating the device as a BQ25756.
- ADC data registers are little-endian in the I2C address space (`REGx` = low byte, `REGx+1` = high byte).
- Implemented ADC channels: `iac_current`, `ibat_current`, `vac_voltage`, `vbat_voltage`, `ts_percent`, optional `vfb_voltage`.
- Implemented status text sensors: `charge_status`, `ts_status`, `mppt_status`, `status_flags`.
- Implemented controls: `charge_enable`, `hiz_mode`, `reverse_mode`, `watchdog`, `watchdog_reset`, `dump_registers`.
- Control entities now emit explicit `Action:` / `Action result:` INFO logs so HA clicks can be correlated with subsequent status/fault transitions.
- Optional init-time configuration now supports:
  - `charge_voltage_limit_mv` -> `REG0x00.VFB_REG`
  - `charge_current_limit_ma` -> `REG0x02.ICHG_REG`
  - `input_current_dpm_limit_ma` -> `REG0x06.IAC_DPM`
  - `input_voltage_dpm_limit_mv` -> `REG0x08.VAC_DPM`
  - `disable_ce_pin` -> `REG0x17.DIS_CE_PIN`
  - `disable_ilim_hiz_pin` -> clear `REG0x18.EN_ILIM_HIZ_PIN`
  - `disable_ichg_pin` -> clear `REG0x18.EN_ICHG_PIN`
- `vfb_voltage` should stay disabled unless explicitly configured; the datasheet recommends disabling that ADC channel during charging when it is not needed.
- `ILIM_HIZ` remains hardware-active unless the board or firmware explicitly disables that pin function; if the pin is left floating or pulled above its HIZ threshold, the charger enters HIZ even when `REG0x17.EN_HIZ` is 0.
- `CE` is still a hardware gate for charging unless `DIS_CE_PIN` is set; a floating/high CE pin can block charging while the I2C `EN_CHG` bit still reads enabled.
