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
  i2c_id: i2c2_bus
  address: 0x6B
  update_interval: 1s
  disable_watchdog: true
  # iac_current:
  #   name: "IAC Current"
  # ibat_current:
  #   name: "IBAT Current"
  # vac_voltage:
  #   name: "VAC Voltage"
  # vbat_voltage:
  #   name: "VBAT Voltage"
  # ts_percent:
  #   name: "TS Percent"
  # vfb_voltage:
  #   name: "VFB Voltage"
  # charge_status:
  #   name: "Charge Status"
  # ts_status:
  #   name: "TS Status"
  # mppt_status:
  #   name: "MPPT Status"
  # status_flags:
  #   name: "Status Flags"
  # pg_good:
  #   name: "PG Good"
  # watchdog_expired:
  #   name: "Watchdog Expired"
  # iac_dpm_active:
  #   name: "IAC DPM Active"
  # vac_dpm_active:
  #   name: "VAC DPM Active"
  # reverse_active:
  #   name: "Reverse Active"
  # cv_timer_expired:
  #   name: "CV Timer Expired"
  # charge_timer_expired:
  #   name: "Charge Timer Expired"
  # vac_uv_fault:
  #   name: "VAC UV Fault"
  # vac_ov_fault:
  #   name: "VAC OV Fault"
  # ibat_ocp_fault:
  #   name: "IBAT OCP Fault"
  # vbat_ov_fault:
  #   name: "VBAT OV Fault"
  # thermal_shutdown:
  #   name: "Thermal Shutdown"
  # drv_sup_fault:
  #   name: "DRV_SUP Fault"
  # charge_enable:
  #   name: "Charge Enable"
  # hiz_mode:
  #   name: "HIZ Mode"
  # reverse_mode:
  #   name: "Reverse Mode"
  # watchdog:
  #   name: "Watchdog"
  # watchdog_reset:
  #   name: "Watchdog Reset"
  # dump_registers:
  #   name: "Dump Registers"
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
