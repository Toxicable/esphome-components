# BQ25756

Datasheet: https://www.ti.com/lit/ds/symlink/bq25756.pdf

## What it does
I2C polling component for TI BQ25756 that:
- Confirms communication during setup by checking the `PART_NUM` field in `REG0x3D`.
- Optionally disables the I2C watchdog via `REG0x15`.
- Enables continuous ADC conversions for the documented BQ25756 channels.
- Optionally publishes ADC/status/fault entities and exposes common host controls.

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq25756 ]

i2c:
  id: i2c2_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq25756:
  id: charger
  i2c_id: i2c_ext
  address: 0x6B
  update_interval: 1s
  disable_watchdog: true
  event_logging: true
  # disable_ce_pin: true
  # disable_ilim_hiz_pin: true
  # disable_ichg_pin: true
  # charge_voltage_limit_mv: 1536
  # charge_current_limit_ma: 5000
  # input_current_dpm_limit_ma: 3000
  # input_voltage_dpm_limit_mv: 12000
  # fb_to_pack_voltage_scale: 10.94
  iac_current:
    name: "Charger Input Current"
  vac_voltage:
    name: "Charger Input Voltage"
  ibat_current:
    name: "Charger Output Current"
  vbat_voltage:
    name: "Charger Output Voltage"
  ts_percent:
    name: "Charger TS Percent"
  # vfb_voltage:
  #   name: "Charger VFB Voltage"
  # vfb_reg_target:
  #   name: "Charger VFB REG Target"
  # vbat_ov_rising_fb:
  #   name: "Charger VBAT OV Rising FB"
  # vbat_ov_falling_fb:
  #   name: "Charger VBAT OV Falling FB"
  # vbat_ov_rising_pack:
  #   name: "Charger VBAT OV Rising Pack"
  # vbat_ov_falling_pack:
  #   name: "Charger VBAT OV Falling Pack"
  charge_status:
    name: "Charge Status"
  ts_status:
    name: "Charger TS Status"
  mppt_status:
    name: "Charger MPPT Status"
  status_flags:
    name: "Charger Status Flags"
  pg_good:
    name: "Charger PG Good"
  watchdog_expired:
    name: "Charger Watchdog Expired"
  iac_dpm_active:
    name: "Charger IAC DPM Active"
  vac_dpm_active:
    name: "Charger VAC DPM Active"
  reverse_active:
    name: "Charger Reverse Active"
  cv_timer_expired:
    name: "Charger CV Timer Expired"
  charge_timer_expired:
    name: "Charger Charge Timer Expired"
  vac_uv_fault:
    name: "Charger VAC UV Fault"
  vac_ov_fault:
    name: "Charger VAC OV Fault"
  ibat_ocp_fault:
    name: "Charger IBAT OCP Fault"
  vbat_ov_fault:
    name: "Charger VBAT OV Fault"
  thermal_shutdown:
    name: "Charger Thermal Shutdown"
  drv_sup_fault:
    name: "Charger DRV_SUP Fault"
  charge_enable:
    name: "Charger Charge Enable"
  hiz_mode:
    name: "Charger HIZ Mode"
  reverse_mode:
    name: "Charger Reverse Mode"
  watchdog:
    name: "Charger Watchdog"
  watchdog_reset:
    name: "Charger Watchdog Reset"
  # dump_registers:
  #   name: "Charger Dump Registers"
```

## Entities

- Sensors: `iac_current`, `ibat_current`, `vac_voltage`, `vbat_voltage`, `ts_percent`, `vfb_voltage`
- Text sensors: `charge_status`, `ts_status`, `mppt_status`, `status_flags`
- Binary sensors: `pg_good`, `watchdog_expired`, `iac_dpm_active`, `vac_dpm_active`, `reverse_active`, `cv_timer_expired`, `charge_timer_expired`, `vac_uv_fault`, `vac_ov_fault`, `ibat_ocp_fault`, `vbat_ov_fault`, `thermal_shutdown`, `drv_sup_fault`
- Switches: `charge_enable`, `hiz_mode`, `reverse_mode`
- Select: `watchdog` (`disable`, `40s`, `80s`, `160s`)
- Buttons: `watchdog_reset`, `dump_registers`

## Notes

- The BQ25756 ADC result registers are little-endian in the I2C map: the low byte is at the base register and the high byte is at the next address.
- `vfb_voltage` is optional and stays disabled unless configured, matching the datasheet recommendation to avoid unnecessary VFB ADC use during charging.
- `ILIM_HIZ` and `CE` are still hardware-active by default. If `ILIM_HIZ` is left floating or driven high, the charger can enter HIZ and stop switching even though this component never sets `EN_HIZ` during startup. If `CE` is left floating or high, charging can stay blocked even when `charge_enable` reports on.
- Set `disable_ce_pin: true` to force software-only control of charging (`REG0x17.DIS_CE_PIN = 1`).
- Set `disable_ilim_hiz_pin: true` and `disable_ichg_pin: true` to ignore external ILIM/ICHG pin limits and use I2C charge/input limits.
- Set `event_logging: true` to emit `INFO` event lines only when charger status/fault bits change.
- Periodic charger telemetry is logged at `DEBUG` level.
- `vfb_reg_target`, `vbat_ov_rising_fb`, and `vbat_ov_falling_fb` are computed from live `REG0x00.VFB_REG`.
- If `fb_to_pack_voltage_scale` is set, `vbat_ov_rising_pack` and `vbat_ov_falling_pack` publish the same thresholds mapped to pack voltage.
