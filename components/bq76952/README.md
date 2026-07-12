# BQ76952 Battery Monitor BMS

ESPHome external component for TI `BQ76952` battery monitor / protector devices on I2C.

It exposes:
- pack telemetry: BAT, PACK, LD, cell voltages, largest inter-cell spread, current, die temperature, TS1/TS2/TS3
- derived metrics: state of charge and charge throughput from `DASTATUS6`
- concise status entities: BMS state, fault, and FET status
- live controls: output enabled, autonomous FET control, and clear alarms
- connection-applied protection and regulator configuration
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

  ## OTP-backed autonomous-FET startup default
  # otp_autonomous_fet_mode: enable

  ## Live config
  sleep_charge_enabled: true
  predischarge_enabled: true
  autonomous_balancing_enabled: true

  ## Regulator config
  reg1_voltage: 3.3V
  reg0_enabled: true

  ## Protection config
  sense_resistor_milliohm: 1.0

  # Each protection is configured as a complete group. Supply every field in
  # the group or omit the whole group and preserve the chip's existing setting.
  cell_undervoltage_limit_mv: 2800
  cell_undervoltage_delay_ms: 200
  cell_overvoltage_limit_mv: 4200
  cell_overvoltage_delay_ms: 200

  charge_current_limit_a: 20
  charge_current_delay_ms: 23
  discharge_current_limit_a: 40
  discharge_current_delay_ms: 23
  discharge_current_limit_2_a: 60
  discharge_current_delay_2_ms: 23
  discharge_current_limit_3_a: 100
  discharge_current_delay_3_s: 1

  short_circuit_in_discharge_threshold_mv: 100
  short_circuit_in_discharge_delay_us: 210
  short_circuit_in_discharge_recovery_time_s: 5
  current_recovery_time_s: 3

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

## Configuration Model

ESPHome creates one `BQ76952Config` object and passes it to the component once. Optional values mean “leave the chip's existing setting unchanged.” Related threshold and delay values are grouped internally so a partially specified protection cannot be applied.

The component has fixed product assumptions rather than configurable escape hatches:

- an `S`-cell pack is wired to `VC1..VC(S-1), VC16`
- sleep is always allowed
- configuration is applied immediately after the first successful Control/Battery/FET communication probe
- failed configuration remains pending and is retried after communication recovers
- drift is audited periodically using read-before-write checks
- state changes are logged automatically; there is no event-logging mode or XCHG burst option

## Entity Set

Recommended user-facing entities:
- `pack_voltage`: actual pack terminal voltage
- `current`: positive for discharge, negative for charge
- `state_of_charge`: host-derived estimate using coulomb counting with learned full/empty endpoints and voltage-curve fallback
- `fault`: active protection or permanent-failure reason using expanded names
- `bms_state`: high-level device state such as `normal`, `sleep`, `deep_sleep`, or `config_update`
- `output_enabled_control`: simple on/off path control where `on` means both `CHG` and `DSG` are enabled

Useful diagnostics:
- `bat_voltage`: top-of-stack battery reading
- `ld_voltage`: load-detect pin voltage
- `largest_intercell_voltage`: maximum-minus-minimum spread across active cells
- `fet_status_flags`: raw live FET and pin state summary
- `energy_time`: accumulator integration time window
- `die_temperature` and any TS thermistor inputs that are wired

Entities intentionally not exposed:
- raw `alarm_flags` and `safety_status_flags`: replaced by the clearer `fault` sensor
- `power_path_state`: redundant with output control, fault, and FET status
- a mode select for `off` / `charge` / `discharge` / `bidirectional`: replaced by a simple output switch
- a sleep control: sleep is an always-enabled device capability, not a user mode

## Connection-Applied RAM Config

Configured RAM settings are reconciled as soon as the BQ76952 responds successfully. There is no arbitrary boot delay.

The reconciliation covers:
- `Settings:Configuration:Vcell Mode`, derived from `cell_count` using the fixed `VC1..VC(S-1), VC16` layout
- sleep enable
- `sleep_charge_enabled`
- `predischarge_enabled`
- `autonomous_balancing_enabled`
- `reg0_enabled`
- `reg1_enabled` and `reg1_voltage`
- TS1/TS2/TS3 thermistor pin setup
- configured CUV, COV, OCC, OCD1, OCD2, OCD3, and SCD protection groups
- current and short-circuit recovery times
- optional autonomous-FET startup policy

Notes:
- data-memory writes require `FULLACCESS`
- the component may enter `CONFIG_UPDATE` while applying settings
- entering `CONFIG_UPDATE` briefly turns the BQ76952 FETs off
- read-before-write checks avoid rewriting matching settings
- reconnection forces restoration of live regulator and mode state
- periodic audits repair configuration drift without cycling already-correct regulators
- `reg1_voltage` implies `reg1_enabled: true` unless explicitly disabled
- `reg0_enabled` remains optional because the external BREG path is board-specific
- normal YAML changes affect live RAM; they are not permanent until OTP is deliberately programmed

## Factory OTP

`program_factory_otp` is separate because OTP programming is irreversible.

It first reconciles the requested live configuration, then programs:

1. sleep enabled as a fixed product policy
2. the optional `otp_autonomous_fet_mode` startup default
3. the currently requested data-memory configuration

The button runs `OTP_WR_CHECK()` before `OTP_WRITE()`.

## Key Options

- `sleep_charge_enabled`: permits charging while the chip is in sleep
- `predischarge_enabled`: enables the persistent pre-discharge FET option
- `autonomous_balancing_enabled`: enables charging and relaxed automatic balancing
- `otp_autonomous_fet_mode`: optional autonomous-FET startup default to burn into OTP
- CUV/COV limits must be supplied with their matching delays
- OCC/OCD limits must be supplied with their matching delays
- SCD threshold, delay, and recovery time form one required group

## Sensor Notes

Charge accumulator:
- `energy` is derived from the integrated charge accumulator in `DASTATUS6`
- it is reported in amp-hours because the chip exposes charge accumulation, not true watt-hours
- `reset_passed_charge` sends `RESET_PASSQ()`

Current sign:
- the chip reports discharge as negative in `CC2 Current()`
- this component flips the sign so exposed current is positive for discharge

State of charge:
- no nominal-capacity setting is required
- full and empty endpoints are learned from cell voltage, protection status, current direction, and coulomb-count history
- learned state is persisted through ESPHome preferences

Thermistors:
- use `pullup: 18k` for a typical `10 kOhm NTC`
- use `pullup: 180k` for a higher-value thermistor such as `100 kOhm`
- TS2 shares hardware functions with wake; do not configure it as a thermistor when the board uses TS2 for wake

Cell mapping:
- 3S maps to raw channels `[1, 2, 16]`
- 4S maps to raw channels `[1, 2, 3, 16]`
- 16S maps to raw channels `[1, 2, ..., 16]`
- the same mapping is used for telemetry and the `Vcell Mode` protection mask

## Migration

Remove these obsolete settings from older YAML:
- `cell_channels`
- `otp_sleep_mode` and `sleep_mode`
- `boot_config_apply_delay`
- `nominal_capacity_ah`
- `event_logging`
- `xchg_debug_burst`
- `security_state`

Other renamed entities:
- `operating_mode` → `bms_state`
- `passed_charge` → `energy`
- `passed_charge_time` → `energy_time`
- `power_path` → `output_enabled_control`
