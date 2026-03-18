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
  #   name: "IBUS Current"
  # ibat_current:
  #   name: "IBAT Current"
  # vbus_voltage:
  #   name: "VBUS Voltage"
  # vbat_voltage:
  #   name: "VBAT Voltage"
  # vsys_voltage:
  #   name: "VSYS Voltage"
  # ts_percent:
  #   name: "TS Percent"
  # die_temperature:
  #   name: "Die Temperature"
  # charge_status:
  #   name: "Charge Status"
  # vbus_status:
  #   name: "VBUS Status"
  # pg_good:
  #   name: "PG Good"
  # vbus_present:
  #   name: "VBUS Present"
  # vbat_present:
  #   name: "VBAT Present"
  # watchdog_expired:
  #   name: "Watchdog Expired"
  # iindpm_active:
  #   name: "IINDPM Active"
  # vindpm_active:
  #   name: "VINDPM Active"
  # thermal_regulation:
  #   name: "Thermal Regulation"
  # vsys_regulation:
  #   name: "VSYS Regulation"
  # ts_cold:
  #   name: "TS Cold"
  # ts_cool:
  #   name: "TS Cool"
  # ts_warm:
  #   name: "TS Warm"
  # ts_hot:
  #   name: "TS Hot"
  # status_flags:
  #   name: "Status Flags"
  # charge_enable:
  #   name: "Charge Enable"
  # hiz_mode:
  #   name: "HIZ Mode"
  # otg_mode:
  #   name: "OTG Mode"
  # watchdog:
  #   name: "Watchdog"
  # watchdog_reset:
  #   name: "Watchdog Reset"
  # dump_registers:
  #   name: "Dump Registers"
```

## Entities

- Sensors: `ibus_current`, `ibat_current`, `vbus_voltage`, `vbat_voltage`, `vsys_voltage`, `ts_percent`, `die_temperature`
- Text sensors: `charge_status`, `vbus_status`, `status_flags`
- Binary sensors: `pg_good`, `vbus_present`, `vbat_present`, `watchdog_expired`, `iindpm_active`, `vindpm_active`, `thermal_regulation`, `vsys_regulation`, `ts_cold`, `ts_cool`, `ts_warm`, `ts_hot`
- Switches: `charge_enable`, `hiz_mode`, `otg_mode`
- Select: `watchdog` (`disable`, `0.5s`, `1s`, `2s`, `20s`, `40s`, `80s`, `160s`)
- Buttons: `watchdog_reset`, `dump_registers`
