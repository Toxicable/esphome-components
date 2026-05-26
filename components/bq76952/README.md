# BQ76952 Battery Monitor BMS

ESPHome external component for TI `BQ76952` battery monitor / protector devices on I2C.

It exposes:
- pack telemetry: BAT, PACK, LD, cell voltages, largest inter-cell spread, current, die temperature, TS1/TS2/TS3
- derived metrics: state of charge and energy-style throughput from `DASTATUS6`
- concise status entities: BMS state, current fault, FET status
- live controls: output enabled, autonomous FET control, clear alarms
- boot-applied protection and regulator configuration
- a separate factory-only OTP programming action

## Configuration

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq76952 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq76952:
  i2c_id: i2c_bus

  ## Core
  cell_count: 4
  update_interval: 20ms

  ## OTP-backed startup defaults
  # otp_autonomous_fet_mode: enable
  # otp_sleep_mode: enable

  ## Boot-applied live config
  # sleep_charge_enabled: true
  # predischarge_enabled: true

  ## Regulator config
  # reg1_voltage: 3.3V
  # reg0_enabled: true

  ## Current / protection config
  sense_resistor_milliohm: 1.0
  # cell_undervoltage_limit_mv: 2800
  # cell_undervoltage_delay_ms: 200
  # cell_overvoltage_limit_mv: 4200
  # cell_overvoltage_delay_ms: 200
  # charge_current_limit_a: 20
  # charge_current_delay_ms: 23
  # discharge_current_limit_a: 40
  # discharge_current_delay_ms: 23
  # discharge_current_limit_2_a: 60
  # discharge_current_delay_2_ms: 23
  # discharge_current_limit_3_a: 100
  # discharge_current_delay_3_s: 1
  # short_circuit_in_discharge_threshold_mv: 100
  # short_circuit_in_discharge_delay_us: 210
  # short_circuit_in_discharge_recovery_time_s: 5
  # current_recovery_time_s: 3

  ## Derived metrics
  nominal_capacity_ah: 5

  ## Buttons
  program_factory_otp:
    name: "BMS OTP Program Factory Config"
  clear_alarms:
    name: "BMS Clear Alarms"
  reset_passed_charge:
    name: "BMS Reset Energy Counter"

  ## Status text sensors
  bms_state:
    name: "BMS State"
  fault:
    name: "BMS Fault"
  fet_status_flags:
    name: "BMS FET Status"

  ## Voltage sensors
  bat_voltage:
    name: "BMS BAT Voltage"
  pack_voltage:
    name: "BMS Pack Voltage"
  ld_voltage:
    name: "BMS Load Detect Pin Voltage"
  largest_intercell_voltage:
    name: "BMS Largest Inter-Cell Voltage"
  cell1_voltage:
    name: "BMS Cell 1"
  cell2_voltage:
    name: "BMS Cell 2"
  cell3_voltage:
    name: "BMS Cell 3"
  cell4_voltage:
    name: "BMS Cell 4"
  # cell5_voltage:
  #   name: "Cell 5"
  # cell6_voltage:
  #   name: "Cell 6"
  # cell7_voltage:
  #   name: "Cell 7"
  # cell8_voltage:
  #   name: "Cell 8"
  # cell9_voltage:
  #   name: "Cell 9"
  # cell10_voltage:
  #   name: "Cell 10"
  # cell11_voltage:
  #   name: "Cell 11"
  # cell12_voltage:
  #   name: "Cell 12"
  # cell13_voltage:
  #   name: "Cell 13"
  # cell14_voltage:
  #   name: "Cell 14"
  # cell15_voltage:
  #   name: "Cell 15"
  # cell16_voltage:
  #   name: "Cell 16"

  ## Current / derived sensors
  current:
    name: "BMS Current"
  state_of_charge:
    name: "BMS State of Charge"
  energy:
    name: "BMS Energy"
  # energy_time:
  #   name: "BMS Energy Time"

  ## Temperature sensors
  ts1_temperature:
    name: "BMS TS1 Temperature"
    pullup: 18k
  # ts2_temperature:
  #   name: "BMS TS2 Temperature"
  #   pullup: 18k
  # ts3_temperature:
  #   name: "BMS TS3 Temperature"
  #   pullup: 18k
  die_temperature:
    name: "BMS Die Temperature"

  ## Live controls
  output_enabled_control:
    name: "BMS Output Enabled"
  autonomous_fet_control:
    name: "BMS Autonomous FET Control"
```

## Entity Set

Recommended user-facing entities:
- `pack_voltage`: actual pack terminal voltage
- `current`: positive for discharge, negative for charge
- `state_of_charge`: host-derived estimate from the charge accumulator and `nominal_capacity_ah`
- `fault`: active protection or permanent-failure reason using expanded names
- `bms_state`: high-level device state such as `normal`, `sleep`, `deep_sleep`, or `config_update`
- `output_enabled_control`: simple on/off path control where `on` means both `CHG` and `DSG` are enabled

Useful diagnostics to keep:
- `bat_voltage`: top-of-stack battery reading
- `ld_voltage`: load-detect pin voltage
- `largest_intercell_voltage`: max-min spread across active cells (quick imbalance view)
- `fet_status_flags`: raw live FET and pin state summary
- `energy_time`: accumulator integration time window
- `die_temperature` and any TS thermistor inputs you actually wire

Entities intentionally not exposed:
- raw `alarm_flags` and `safety_status_flags`: replaced by the clearer `fault` sensor
- `power_path_state`: redundant once `output_enabled_control`, `fault`, and `fet_status_flags` exist
- a mode select for `off` / `charge` / `discharge` / `bidirectional`: replaced by a simpler output on/off control

## Boot-Applied RAM Config

These settings are normal YAML config and are written into the chip's live RAM configuration automatically on boot.

These writes are not permanent across a full hardware reset or a chip that boots from different OTP defaults.

Boot-applied RAM config includes:
- `sleep_charge_enabled`
- `predischarge_enabled`
- `reg0_enabled`
- `reg1_voltage`
- `ts1_temperature` / `ts2_temperature` / `ts3_temperature` thermistor pin setup
- `sense_resistor_milliohm`
- `cell_undervoltage_limit_mv`
- `cell_undervoltage_delay_ms`
- `cell_overvoltage_limit_mv`
- `cell_overvoltage_delay_ms`
- `charge_current_limit_a`
- `charge_current_delay_ms`
- `discharge_current_limit_a`
- `discharge_current_delay_ms`
- `discharge_current_limit_2_a`
- `discharge_current_delay_2_ms`
- `discharge_current_limit_3_a`
- `discharge_current_delay_3_s`
- `short_circuit_in_discharge_threshold_mv`
- `short_circuit_in_discharge_delay_us`
- `short_circuit_in_discharge_recovery_time_s`
- `current_recovery_time_s`

Notes:
- these writes require `FULLACCESS`
- the component may enter `CONFIG_UPDATE` to apply them
- entering `CONFIG_UPDATE` briefly turns FETs off, so do not power the ESP from a path that depends on switched PACK output during configuration writes
- `reg1_voltage` implies `REG1` should be enabled if you do not explicitly set `reg1_enabled`
- `reg0_enabled` remains explicit because whether the external BREG preregulator path is populated is board-specific and cannot be inferred safely
- the chip has a persistent `PDSG_EN` option for predischarge, but it does not expose a separate matching persistent `PCHG_EN` bit for precharge

## OTP Settings

These config keys are specifically for OTP-backed startup defaults:
- `otp_autonomous_fet_mode`
- `otp_sleep_mode`

They control what the chip should boot up doing after OTP programming, not just what the running device does right now.

## Factory OTP

`program_factory_otp` is separate because it changes what the chip boots with after a power cycle, and it is irreversible.

What `program_factory_otp` persists into OTP:
1. startup-default boot bits from `otp_sleep_mode` and `otp_autonomous_fet_mode`
2. the current data-memory configuration that the component has already applied live, including regulator, FET-option, TS-pin, and protection settings

What it does not mean:
- normal YAML changes are not persistent by themselves
- persistence happens only when `program_factory_otp` is pressed successfully

Safe mental model:
- boot: component applies your YAML into the running chip
- OTP button: burns the currently requested live config plus the explicit `otp_*` startup defaults so the chip powers up that way later

## Key Options

- `sleep_charge_enabled`: sets `FET Options[SLEEPCHG]` so charging can remain allowed while the chip is in `sleep`
- `predischarge_enabled`: sets `FET Options[PDSG_EN]`
- `otp_autonomous_fet_mode`: startup-default autonomous FET behavior to burn into OTP
- `otp_sleep_mode`: startup-default sleep-allow behavior to burn into OTP
- `cell_undervoltage_limit_mv` / `cell_overvoltage_limit_mv`: configure `CUV` / `COV`
- `discharge_current_limit_a`, `discharge_current_limit_2_a`, `discharge_current_limit_3_a`: configure `OCD1`, `OCD2`, and `OCD3`
- `charge_current_limit_a`: configures `OCC`
- `short_circuit_in_discharge_threshold_mv`: configures short-circuit-in-discharge threshold
- `nominal_capacity_ah`: required only if you want `state_of_charge`

## Sensor Notes

Charge accumulator:
- `energy` is derived from the same integrated charge accumulator in `DASTATUS6`
- it is still reported in amp-hours because the chip exposes charge accumulation, not true watt-hours
- `reset_passed_charge` sends `RESET_PASSQ()`

Current sign:
- the chip reports discharge as negative in `CC2 Current()`
- this component flips the sign so the exposed `current` sensor is positive for discharge and negative for charge

BMS state:
- `normal`: awake and not in a special mode
- `sleep`: `Battery Status[SLEEP]=1`
- `deep_sleep`: `CONTROL_STATUS[DEEPSLEEP]=1`
- `config_update`: `Battery Status[CFGUPDATE]=1`
- `shutdown_pending`: shutdown command latched

Fault naming:
- `fault` expands safety causes into names such as `cell_overvoltage`, `cell_undervoltage`, `overcurrent_in_charge`, `short_circuit_in_discharge`, or `permanent_failure`
- if multiple active causes are present, they are reported as a comma-separated list

Thermistors:
- use `pullup: 18k` for a typical `10 kOhm NTC`
- use `pullup: 180k` for higher-value thermistors such as `100 kOhm`
- `ts2_temperature` shares the TS2 pin with the BQ76952 wake function, so do not use it as a thermistor if your hardware uses TS2 for wake

Cell mapping:
- `cell1_voltage` through `cell16_voltage` are mapped to the first populated differential cell-voltage commands seen at startup
- this supports sparse physical layouts such as a 4S pack wired onto higher-numbered VC pins

## Migration

If you are updating from an older config:
- `operating_mode` -> `bms_state`
- `autonomous_fet_mode` -> `otp_autonomous_fet_mode`
- `sleep_mode` -> `otp_sleep_mode`
- `security_state` -> remove
- `passed_charge` -> `energy`
- `passed_charge_time` -> `energy_time`
- `power_path` -> `output_enabled_control`
- `scd_threshold_mv` -> `short_circuit_in_discharge_threshold_mv`
- `scd_delay_us` -> `short_circuit_in_discharge_delay_us`
- `scd_recovery_time_s` -> `short_circuit_in_discharge_recovery_time_s`
- remove `power_path_state`
- remove `alarm_flags` and `safety_status_flags`; use `fault` instead
- remove `sleep_allowed_control`
