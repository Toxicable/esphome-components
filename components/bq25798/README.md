# BQ25798

Datasheet: https://www.ti.com/lit/ds/symlink/bq25798.pdf

## What it does
I2C polling component for TI BQ25798 that:
- Confirms communication during setup by reading `REG10`.
- Optionally disables watchdog (`WATCHDOG[2:0] = 000`) to keep host-controlled behavior.
- Logs charger status bytes (`0x1B..0x1F`) and key ADC values on each update.
- Optionally publishes ADC/status sensors and exposes common controls.

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq25798 ]

i2c:
  id: i2c2_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq25798:
  id: charger
  i2c_id: i2c2_bus
  address: 0x6B
  update_interval: 1s
  disable_watchdog: true
  # ibus_current:
  #   name: "BQ25798 IBUS Current"
  # ibat_current:
  #   name: "BQ25798 IBAT Current"
  # vbus_voltage:
  #   name: "BQ25798 VBUS Voltage"
  # vbat_voltage:
  #   name: "BQ25798 VBAT Voltage"
  # vsys_voltage:
  #   name: "BQ25798 VSYS Voltage"
  # ts_percent:
  #   name: "BQ25798 TS Percent"
  # die_temperature:
  #   name: "BQ25798 Die Temperature"
  # charge_status:
  #   name: "BQ25798 Charge Status"
  # vbus_status:
  #   name: "BQ25798 VBUS Status"
  # pg_good:
  #   name: "BQ25798 PG Good"
  # vbus_present:
  #   name: "BQ25798 VBUS Present"
  # vbat_present:
  #   name: "BQ25798 VBAT Present"
  # watchdog_expired:
  #   name: "BQ25798 Watchdog Expired"
  # iindpm_active:
  #   name: "BQ25798 IINDPM Active"
  # vindpm_active:
  #   name: "BQ25798 VINDPM Active"
  # thermal_regulation:
  #   name: "BQ25798 Thermal Regulation"
  # vsys_regulation:
  #   name: "BQ25798 VSYS Regulation"
  # ts_cold:
  #   name: "BQ25798 TS Cold"
  # ts_cool:
  #   name: "BQ25798 TS Cool"
  # ts_warm:
  #   name: "BQ25798 TS Warm"
  # ts_hot:
  #   name: "BQ25798 TS Hot"
  # status_flags:
  #   name: "BQ25798 Status Flags"
  # charge_enable:
  #   name: "BQ25798 Charge Enable"
  # hiz_mode:
  #   name: "BQ25798 HIZ Mode"
  # otg_mode:
  #   name: "BQ25798 OTG Mode"
  # watchdog:
  #   name: "BQ25798 Watchdog"
  # watchdog_reset:
  #   name: "BQ25798 Watchdog Reset"
  # dump_registers:
  #   name: "BQ25798 Dump Registers"
```

## Entities

- Sensors: `ibus_current`, `ibat_current`, `vbus_voltage`, `vbat_voltage`, `vsys_voltage`, `ts_percent`, `die_temperature`
- Text sensors: `charge_status`, `vbus_status`, `status_flags`
- Binary sensors: `pg_good`, `vbus_present`, `vbat_present`, `watchdog_expired`, `iindpm_active`, `vindpm_active`, `thermal_regulation`, `vsys_regulation`, `ts_cold`, `ts_cool`, `ts_warm`, `ts_hot`
- Switches: `charge_enable`, `hiz_mode`, `otg_mode`
- Select: `watchdog` (`disable`, `0.5s`, `1s`, `2s`, `20s`, `40s`, `80s`, `160s`)
- Buttons: `watchdog_reset`, `dump_registers`

`status_flags` is now a concise comma-separated summary of active conditions (or `none`).
