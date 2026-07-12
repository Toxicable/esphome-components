# BQ76952 Battery Monitor and Protector

ESPHome external component for TI `BQ76952` 3S–16S battery monitors.

> This branch currently defines the target interfaces. The previous monolithic implementation has not yet been migrated, so the component is not expected to compile until the protocol/service/SoC move is complete.

## Architecture

- `bq76952_protocol.h`: register, subcommand, transfer-buffer, checksum, CRC, and CONFIG_UPDATE transport
- `bq76952_config.h`: complete deterministic desired device state
- `bq76952_service.h`: connection recovery, configuration synchronization, measurements, protections, runtime actions, and ownership of SoC
- `bq76952_soc.h`: ancillary SoC estimation/persistence logic used by the service
- `bq76952.h`: ESPHome entity facade only

There is no compatibility `ConfigState`, state-bag inheritance, or preserve-by-omission behaviour.

## Example target configuration

```yaml
bq76952:
  i2c_id: i2c_bus
  address: 0x08
  update_interval: 1s

  cell_count: 4
  cell_chemistry: lithium_ion
  sense_resistor_milliohm: 1.0
  i2c_crc_enabled: false
  current_gain_policy: factory_calibration

  regulators:
    reg0_enabled: true
    reg1_enabled: true
    reg1_voltage: 3.3V
    reg2_enabled: false
    reg2_voltage: 3.3V

  thermistors:
    ts1: 18k
    ts2: disabled
    ts3: disabled

  fet:
    # The BQ76952 autonomously controls CHG/DSG from its protection state.
    autonomous: true
    sleep_charge_enabled: true
    body_diode_threshold_ma: 0

    # Reduced-current charging path for a deeply depleted cell.
    precharge:
      enabled: false
      start_cell_voltage_mv: 2500
      stop_cell_voltage_mv: 3000

    # Reduced-current discharge path used to limit load/DC-link inrush.
    predischarge:
      enabled: true
      timeout_ms: 2550
      stop_delta_mv: 500

  balancing:
    charging_enabled: true

    # Also balance after charge current falls below this threshold.
    relaxed_balancing_enabled: true
    relaxed_current_threshold_a: 0.1

    minimum_cell_voltage_mv: 3000
    start_delta_mv: 40
    stop_delta_mv: 20
    minimum_temperature_c: 0
    maximum_temperature_c: 45
    maximum_balanced_cells: 2

  protections:
    # CUV/COV thresholds apply independently to every active cell.
    cell_undervoltage:
      enabled: true
      threshold_mv: 2800
      delay_ms: 200
      recovery_hysteresis_mv: 100

    cell_overvoltage:
      enabled: true
      threshold_mv: 4200
      delay_ms: 200
      recovery_hysteresis_mv: 100

    charge_overcurrent:
      enabled: true
      threshold_a: 20
      delay_ms: 23

    discharge_overcurrent_1:
      enabled: true
      threshold_a: 40
      delay_ms: 23

    discharge_overcurrent_2:
      enabled: true
      threshold_a: 60
      delay_ms: 23

    # OCD3 is the slower, long-duration overcurrent tier.
    discharge_overcurrent_3:
      enabled: true
      threshold_a: 100
      delay_s: 1

    discharge_short_circuit:
      enabled: true
      threshold_mv: 100
      delay_us: 210
      recovery_time_s: 5

    temperature:
      charge_enabled: true
      discharge_enabled: true
      charge_minimum_c: 0
      charge_maximum_c: 45
      discharge_minimum_c: -20
      discharge_maximum_c: 60
      recovery_hysteresis_c: 5

    current_recovery_time_s: 3

  bms_state:
    name: "BMS State"
  fault:
    name: "BMS Fault"
  fet_status_flags:
    name: "BMS FET Status"

  pack_voltage:
    name: "BMS Pack Voltage"
  current:
    name: "BMS Current"
  state_of_charge:
    name: "BMS State of Charge"

  cell1_voltage:
    name: "BMS Cell 1"
  cell2_voltage:
    name: "BMS Cell 2"
  cell3_voltage:
    name: "BMS Cell 3"
  cell4_voltage:
    name: "BMS Cell 4"

  autonomous_control:
    name: "BMS Autonomous"
  output_enabled_control:
    name: "BMS Output Enabled"
  clear_alarms:
    name: "BMS Clear Alarms"
  program_factory_otp:
    name: "BMS Program Factory OTP"
```

## Less-obvious terms

### Relaxed balancing

TI calls the pack **relaxed** when current has fallen below a configured threshold. `relaxed_balancing_enabled` allows balancing in that state instead of restricting balancing to active charging. `relaxed_current_threshold_a` defines the current below which the pack is considered relaxed.

### Cell-voltage protection

`BQ76952CellVoltageProtectionConfig` is per cell. The same CUV or COV threshold is evaluated independently against every active cell selected by `Vcell Mode`; it is not a whole-pack voltage threshold.

### Long-duration current protection

`BQ76952LongDurationCurrentProtectionConfig` represents OCD3. OCD1 and OCD2 are fast millisecond-scale discharge-overcurrent protections; OCD3 is a slower tier with a delay measured in seconds.

### Safety Status A/B/C

The BQ76952 divides raw safety bits across three registers named Safety Status A, B, and C. These are protocol details. The service decodes all three, plus battery/permanent-failure state, into one normalized `fault_flags` field before the ESPHome facade sees it.

### Configuration synchronization

The old `reconcile_configuration(force_live_state)` interface is gone.

Configuration synchronization is private to the service and has two explicit internal modes:

- `AUDIT_AND_REPAIR`: compare desired data-memory settings and repair drift
- `RESTORE_RUNTIME_STATE`: after reconnect/reset, also resend runtime-only commands even when stored data-memory bytes already match

### Coulomb counter and SoC

The BQ76952 `DASTATUS6` value is an integrated **charge** counter in amp-hours, not energy in watt-hours and not a useful user-facing lifetime total. The service consumes it internally as one input to SoC estimation.

The SoC code keeps a `relative_charge_ah` coordinate. That is an internal continuous position built from coulomb-counter deltas so SoC learning survives device-counter resets or wraparound. It is not exposed to the user.

The voltage fallback curve is chemistry-specific. `cell_chemistry: lithium_ion` is currently required because lithium-ion is the only implemented curve; additional chemistries should add separate curves rather than silently reusing it.

## Fixed assumptions

- an `S`-cell pack maps to `VC1..VC(S-1), VC16`
- sleep is always allowed
- every configuration group is required and explicitly enabled or disabled
- configuration starts after communication is established; there is no arbitrary boot delay
- failed configuration remains pending and is retried after communication recovery
- logging uses normal INFO/DEBUG/WARN levels without logging-mode options
- passed-charge reset is not a user control
