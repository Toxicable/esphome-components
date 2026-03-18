# BQ76922 Battery Monitor

ESPHome external component for TI BQ76922 (3S to 5S packs over I2C).

It provides:
- core telemetry (cell voltages, BAT voltage, PACK voltage, load-detect pin voltage, current, die temperature)
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

  # Number of cells in series (1 to 5):
  cell_count: 5

  # Startup behavior:
  # preserve = don't change device state on boot
  # enable/disable send startup subcommands
  autonomous_fet_mode: preserve
  sleep_mode: preserve

  # Optional current protection thresholds (applied at boot):
  # sense_resistor_milliohm: 1.0
  # charge_current_limit_a: 20.0
  # discharge_current_limit_a: 40.0
  # charge_current_delay_ms: 23
  # discharge_current_delay_ms: 23
  # current_recovery_time_s: 3

  # Optional sensors:
  # bat_voltage:
  #   name: "BQ76922 BAT Voltage"
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

If you use fewer than 5 cells and jumper unused sense inputs, the component auto-detects
which cell-voltage commands are active on first read. This covers common layouts like 4S
with VC4 tied to VC3 and VC5 at BAT+.

Current limit notes:
- These values are written to device protection settings during boot.
- Setting `charge_current_limit_a` automatically enables OCC and CHG-FET trip-on-OCC.
- Setting `discharge_current_limit_a` automatically enables OCD1 and DSG-FET trip-on-OCD1.
- They require the chip to be in `FULLACCESS`.
- Applying them enters `CONFIG_UPDATE` briefly, which turns FETs off during that short window.

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
