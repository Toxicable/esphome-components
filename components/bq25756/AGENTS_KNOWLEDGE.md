# AGENTS_KNOWLEDGE: bq25756

Component-scoped notes for `components/bq25756`.

- Top-level polling component defaults: address `0x6B`, `update_interval: 1s`, `disable_watchdog: true`.
- `event_logging` defaults to `true` and emits concise `INFO` event lines when charger status/fault bytes change (not every polling interval).
- Setup validates `REG0x3D` `PART_NUM[6:3] == 0b0010` before treating the device as a BQ25756.
- ADC data registers are little-endian in the I2C address space (`REGx` = low byte, `REGx+1` = high byte).
- ADC setup explicitly writes `REG0x2B = 0b1000_1100` to initialize continuous, 15-bit running-average conversion from a fresh sample, then verifies its steady self-cleared readback (`0b1000_1000`). It does not inherit the POR `ADC_SAMPLE=0b10` 13-bit setting. This makes behavior deterministic and maximizes low-current resolution; validate any ADC-scale change on hardware rather than assuming the documented LSB changes with resolution.
- Implemented ADC channels: `iac_current`, `ibat_current`, `vac_voltage`, `vbat_voltage`, `ts_percent`, optional `vfb_voltage`.
- Implemented status text sensors: `charge_status`, `ts_status`, `mppt_status`, `status_flags` (fault/state detail is intentionally aggregated here).
- Per-fault/per-flag binary status entities were removed to reduce duplicate entity noise; use `status_flags` for fault summary.
- Implemented controls: `charge_enable`, `hiz_mode`, `reverse_mode`, `watchdog`, `watchdog_reset`, `dump_registers`.
- Control entities now emit explicit `Action:` / `Action result:` INFO logs so HA clicks can be correlated with subsequent status/fault transitions.
- Required `battery.cell_count` and `battery.cell_chemistry` define the pack maximum/minimum voltage; supported chemistries are `lithium_ion` and `lifepo4`.
- `charging.control: i2c` makes I2C the sole source of charge-enable and current-limit control, disabling CE, ILIM/HIZ, and ICHG pin functions together.
- Initialization explicitly clears `EN_CHG` before publishing the charge-enable state, so software never advertises charging-off while the chip is still enabled from reset.
- `calibration` is an explicit persisted DMM workflow: while charging is disabled, enter the measured pack voltage in `measured_voltage` and press `calibrate`. The component derives the feedback ratio from the live VFB ADC reading and rewrites `REG0x00.VFB_REG` for the chemistry-derived maximum pack voltage.
- Diagnostics publish pack-domain `charge_voltage_target`, `battery_overvoltage_rising`, and `battery_overvoltage_falling`; raw feedback-domain thresholds are intentionally not user-facing.
- `vfb_voltage` should stay disabled unless explicitly configured; the datasheet recommends disabling that ADC channel during charging when it is not needed.
- `ILIM_HIZ` remains hardware-active unless the board or firmware explicitly disables that pin function; if the pin is left floating or pulled above its HIZ threshold, the charger enters HIZ even when `REG0x17.EN_HIZ` is 0.
- `CE` is still a hardware gate for charging unless `DIS_CE_PIN` is set; a floating/high CE pin can block charging while the I2C `EN_CHG` bit still reads enabled.
