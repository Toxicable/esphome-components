# AGENTS_KNOWLEDGE: bq25798

Component-scoped notes for `components/bq25798`.

- Top-level polling component defaults: address `0x6B`, `update_interval: 1s`, `disable_watchdog: true`.
- Can publish ADC sensors: `ibus_current`, `ibat_current`, `vbus_voltage`, `vbat_voltage`, `vsys_voltage`, `ts_percent`, `die_temperature`.
- Can publish text status sensors: `charge_status`, `vbus_status`, `status_flags`.
- Can publish split status binary sensors: `pg_good`, `vbus_present`, `vbat_present`, `watchdog_expired`, `iindpm_active`, `vindpm_active`, `thermal_regulation`, `vsys_regulation`, `ts_cold`, `ts_cool`, `ts_warm`, `ts_hot`.
- Can publish controls: `charge_enable`, `hiz_mode`, `otg_mode`, `watchdog` select.
- Can publish buttons: `watchdog_reset`, `dump_registers`.
- Still logs status bytes `0x1B..0x1F` and ADC raws.
- Flag entities (`status_flags` text + split status binary sensors) should use `entity_category: diagnostic`.
