# BQ76922 Battery Monitor BMS

ESPHome external component for TI BQ76922 (3S to 5S packs over I2C).

It provides:
- core telemetry (cell voltages, BAT voltage, PACK voltage, load-detect pin voltage, current, die temperature, TS1 temperature)
- battery/FET/alarm status entities
- host controls for FET path, sleep-allow, and alarm-clear
- startup policy options for autonomous FET control and sleep mode

## Configuration (optional items commented out)

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq76922 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq76922:

  ## Number of cells in series (1 to 5):
  cell_count: 5

  ## Startup behavior:
  ## preserve = don't change device state on boot
  ## enable/disable send startup subcommands
  autonomous_fet_mode: preserve
  sleep_mode: preserve

  ## Optional current protection thresholds (applied at boot):
  # sense_resistor_milliohm: 1.0
  # charge_current_limit_a: 20.0
  # discharge_current_limit_a: 40.0
  # charge_current_delay_ms: 23
  # discharge_current_delay_ms: 23
  # current_recovery_time_s: 3

  ## Optional sensors:
  # bat_voltage:
  #   name: "BAT Voltage"
  # pack_voltage:
  #   name: "Pack Voltage"
  # ld_voltage:
  #   name: "Load Detect Pin Voltage"
  # cell1_voltage:
  #   name: "Cell 1"
  # cell2_voltage:
  #   name: "Cell 2"
  # cell3_voltage:
  #   name: "Cell 3"
  # cell4_voltage:
  #   name: "Cell 4"
  # cell5_voltage:
  #   name: "Cell 5"
  # current:
  #   name: "Current"
  # die_temperature:
  #   name: "Die Temperature"
  # ts1_temperature:
  #   name: "TS1 Temperature"

  ## Optional text sensors:
  # security_state:
  #   name: "Security State"
  # operating_mode:
  #   name: "Operating Mode"
  # power_path_state:
  #   name: "Power Path State"
  # alarm_flags:
  #   name: "Alarm Flags"

  ## Optional binary sensors:
  # sleep_mode_active:
  #   name: "Sleep Active"
  # cfgupdate_mode:
  #   name: "Config Update"
  # protection_fault:
  #   name: "Safety Fault"
  # permanent_fail:
  #   name: "Permanent Fail"
  # sleep_allowed_state:
  #   name: "Sleep Allowed"
  # alert_pin:
  #   name: "ALERT Pin"
  # chg_fet_on:
  #   name: "CHG FET"
  # dsg_fet_on:
  #   name: "DSG FET"
  # autonomous_fet_enabled:
  #   name: "Autonomous FET"

  ## Optional controls:
  # power_path:
  #   name: "Power Path"
  # autonomous_fet_control:
  #   name: "Autonomous FET Control"
  # sleep_allowed_control:
  #   name: "Sleep Allowed Control"
  # clear_alarms:
  #   name: "Clear Alarms"
```

## Config Options You’ll Likely Tune

- `cell_count`: match your physical stack (`1..5`)
- `autonomous_fet_mode`: boot policy for FET firmware control (`preserve`, `enable`, `disable`)
- `sleep_mode`: boot policy for sleep allow (`preserve`, `enable`, `disable`)
- `sense_resistor_milliohm`: shunt resistor value used to convert current limits to chip thresholds
- `charge_current_limit_a`: charge overcurrent protection threshold
- `discharge_current_limit_a`: discharge overcurrent protection threshold (OCD1)
- `charge_current_delay_ms`: OCC trip delay (10ms to 426ms)
- `discharge_current_delay_ms`: OCD1 trip delay (10ms to 426ms)
- `current_recovery_time_s`: shared recovery timer for OCC/OCD protections (0s to 255s)
- `power_path` entity: runtime host command for `off`, `charge`, `discharge`, `bidirectional`

Current and voltage scaling are automatically detected from the chip configuration.
No manual unit settings are needed.

`bat_voltage` is the top-of-stack battery reading (legacy alias: `stack_voltage`).

`ts1_temperature` expects TS1 to be configured as a thermistor input. If TS1 is set to ADC input mode in the
chip configuration, this command reports TS1 pin voltage instead of temperature.

If you use fewer than 5 cells and jumper unused sense inputs, the component auto-detects
which cell-voltage commands are active on first read. This covers common layouts like 4S
with VC4 tied to VC3 and VC5 at BAT+.

Current limit notes:
- These values are written to device protection settings during boot.
- Setting `charge_current_limit_a` automatically enables OCC and CHG-FET trip-on-OCC.
- Setting `discharge_current_limit_a` automatically enables OCD1 and DSG-FET trip-on-OCD1.
- They require the chip to be in `FULLACCESS`.
- Applying them enters `CONFIG_UPDATE` briefly, which turns FETs off during that short window.
- If requested values are already present in data memory, the component skips `CONFIG_UPDATE` and
  does not re-apply them.
- When ESP32 OTA rollback is active and the image is still pending verification, this component
  now defers these writes until the boot is marked successful to avoid rollback loops.
- If your ESP is powered through that switched FET path, expect one reset when deferred writes are
  finally applied (after boot is already marked successful).

## Autonomous Mode

For BQ76922, autonomous FET operation corresponds to `Manufacturing Status[FET_EN] = 1`
(firmware controls CHG/DSG based on protections/recovery).

Use either approach:

1. Startup policy in YAML:
   - `autonomous_fet_mode: enable`
2. Runtime control entity:
   - enable `autonomous_fet_control` and turn it ON

Important:
- `FET_ENABLE()` is a toggle subcommand and requires `UNSEALED` or `FULLACCESS`.
- If the device is `SEALED`, the component will not be able to change this bit.
- For persistence across full resets/SHUTDOWN, program your desired behavior into OTP (`Mfg Status Init[FET_EN]`) during manufacturing.
