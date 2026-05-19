# AGENTS_KNOWLEDGE: bq25756

Component-scoped notes for `components/bq25756`.

- Top-level polling component defaults: address `0x6B`, `update_interval: 1s`, `disable_watchdog: true`.
- Setup validates `REG0x3D` `PART_NUM[6:3] == 0b0010` before treating the device as a BQ25756.
- ADC data registers are little-endian in the I2C address space (`REGx` = low byte, `REGx+1` = high byte).
- Implemented ADC channels: `iac_current`, `ibat_current`, `vac_voltage`, `vbat_voltage`, `ts_percent`, optional `vfb_voltage`.
- Implemented status text sensors: `charge_status`, `ts_status`, `mppt_status`, `status_flags`.
- Implemented controls: `charge_enable`, `hiz_mode`, `reverse_mode`, `watchdog`, `watchdog_reset`, `dump_registers`.
- `vfb_voltage` should stay disabled unless explicitly configured; the datasheet recommends disabling that ADC channel during charging when it is not needed.
