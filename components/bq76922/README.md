# BQ76922 Battery Monitor

ESPHome external component for TI BQ76922 (3S to 5S packs over I2C).

It provides:
- core telemetry (cell voltages, stack voltage, pack voltage, load-detect pin voltage, current, die temperature)
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
  id: bms
  i2c_id: i2c_bus
  address: 0x08
  update_interval: 1s

  # Number of cells in series (3 to 5):
  cell_count: 5

  # Startup behavior:
  # preserve = don't change device state on boot
  # enable/disable send startup subcommands
  autonomous_fet_mode: preserve
  sleep_mode: preserve

  # Optional sensors:
  # stack_voltage:
  #   name: "BQ76922 Stack Voltage"
  # pack_voltage:
  #   name: "BQ76922 Pack Voltage"
  # ld_voltage:
  #   name: "BQ76922 Load Detect Pin Voltage"
  # cell1_voltage:
  #   name: "BQ76922 Cell 1"
  # cell2_voltage:
  #   name: "BQ76922 Cell 2"
  # cell3_voltage:
  #   name: "BQ76922 Cell 3"
  # cell4_voltage:
  #   name: "BQ76922 Cell 4"
  # cell5_voltage:
  #   name: "BQ76922 Cell 5"
  # current:
  #   name: "BQ76922 Current"
  # die_temperature:
  #   name: "BQ76922 Die Temperature"

  # Optional text sensors:
  # security_state:
  #   name: "BQ76922 Security State"
  # operating_mode:
  #   name: "BQ76922 Operating Mode"
  # power_path_state:
  #   name: "BQ76922 Power Path State"
  # alarm_flags:
  #   name: "BQ76922 Alarm Flags"

  # Optional binary sensors:
  # sleep_mode_active:
  #   name: "BQ76922 Sleep Active"
  # cfgupdate_mode:
  #   name: "BQ76922 Config Update"
  # protection_fault:
  #   name: "BQ76922 Safety Fault"
  # permanent_fail:
  #   name: "BQ76922 Permanent Fail"
  # sleep_allowed_state:
  #   name: "BQ76922 Sleep Allowed"
  # alert_pin:
  #   name: "BQ76922 ALERT Pin"
  # chg_fet_on:
  #   name: "BQ76922 CHG FET"
  # dsg_fet_on:
  #   name: "BQ76922 DSG FET"
  # autonomous_fet_enabled:
  #   name: "BQ76922 Autonomous FET"

  # Optional controls:
  # power_path:
  #   name: "BQ76922 Power Path"
  # autonomous_fet_control:
  #   name: "BQ76922 Autonomous FET Control"
  # sleep_allowed_control:
  #   name: "BQ76922 Sleep Allowed Control"
  # clear_alarms:
  #   name: "BQ76922 Clear Alarms"
```

## Config Options You’ll Likely Tune

- `cell_count`: match your physical stack (`3..5`)
- `autonomous_fet_mode`: boot policy for FET firmware control (`preserve`, `enable`, `disable`)
- `sleep_mode`: boot policy for sleep allow (`preserve`, `enable`, `disable`)
- `power_path` entity: runtime host command for `off`, `charge`, `discharge`, `bidirectional`

Current and voltage scaling are automatically detected from the chip configuration.
No manual unit settings are needed.

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
