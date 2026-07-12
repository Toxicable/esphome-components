# BQ76952 Battery Monitor and Protector

ESPHome external component for TI `BQ76952` 3S–16S battery monitors.

The component implements the hard-cut configuration interfaces directly. The previous monolithic state bag and compatibility paths have been removed.

## Architecture

- `bq76952_protocol.cpp`: register, subcommand, data-memory, checksum, CRC, and CONFIG_UPDATE transport
- `bq76952_config.h`: complete deterministic desired device state
- `bq76952_service.cpp`: connection recovery, configuration synchronization, measurements, protections, controls, and SoC ownership
- `bq76952_soc.cpp`: ancillary SoC estimation and persistence logic
- `bq76952.cpp`: ESPHome entity facade only

There is no compatibility `ConfigState`, state-bag inheritance, or preserve-by-omission behaviour.

## Configuration

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
    # The BQ76952 controls CHG/DSG automatically from protection state.
    autonomous: true
    sleep_charge_enabled: true

    # Reduced-current charging path for deeply depleted cells.
    precharge:
      enabled: false
      start_cell_voltage_mv: 2500
      stop_cell_voltage_mv: 3000

    # Reduced-current discharge path used to limit load/DC-link inrush.
    predischarge:
      enabled: true
      timeout_ms: 2550       # Maximum; 0 disables timeout, resolution is 10 ms.
      stop_delta_mv: 500     # 0 disables voltage termination, resolution is 10 mV.

  # Balancing is always enabled while charging. Relaxed/idle balancing is off.
  balancing:
    minimum_cell_voltage_mv: 3000
    start_delta_mv: 40
    stop_delta_mv: 20
    minimum_temperature_c: 0
    maximum_temperature_c: 45
    maximum_balanced_cells: 2

  # All listed primary protections are always enabled.
  protections:
    # CUV/COV apply independently to every active cell.
    cell_undervoltage:
      threshold_mv: 2800
      delay_ms: 200
      recovery_hysteresis_mv: 100

    cell_overvoltage:
      threshold_mv: 4200
      delay_ms: 200
      recovery_hysteresis_mv: 100

    charge_overcurrent:
      threshold_a: 20
      delay_ms: 100

    # OCD1: ordinary overload, lower threshold and longer delay.
    discharge_overcurrent:
      threshold_a: 40
      delay_ms: 100

    # OCD2: severe overload, higher threshold and shorter delay.
    discharge_severe_overcurrent:
      threshold_a: 60
      delay_ms: 23

    # OCD3: sustained overload measured over seconds.
    discharge_sustained_overcurrent:
      threshold_a: 30
      delay_s: 5

    discharge_short_circuit:
      threshold_mv: 100
      delay_us: 210
      recovery_time_s: 5

    temperature:
      charge_minimum_c: 0
      charge_maximum_c: 45
      discharge_minimum_c: -20
      discharge_maximum_c: 60
      recovery_hysteresis_c: 5

    current_recovery_time_s: 3

  # High-level operating mode: offline, normal, sleep, deep_sleep,
  # config_update, or shutdown_pending.
  state:
    name: "BMS State"

  # Active protection reason(s), or none.
  fault:
    name: "BMS Fault"

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

  output_enabled_control:
    name: "BMS Output Enabled"

  clear_alarms:
    name: "BMS Clear Alarms"
```

## Runtime behaviour

The service waits until the device answers its communication probe, then compares the complete desired configuration with data memory. It enters `CONFIG_UPDATE` only when drift is found. Failed synchronization remains pending and retries after communication recovery. Runtime-only sleep, regulator and autonomous-FET state is restored after reconnect or reset.

## State versus fault

- `state` reports the device operating mode, such as `normal`, `sleep`, or `offline`.
- `fault` reports active protection causes, such as cell undervoltage, overcurrent, overtemperature, or permanent failure.
- Raw Safety Status A/B/C and FET status bits are internal diagnostics. They are decoded into `fault` and logged when useful rather than exposed as a third text entity.

## Fixed product policy

The following are intentionally not configurable:

- series-FET body-diode threshold: fixed to TI's 50 mA default
- balancing while charging: always enabled
- relaxed/idle balancing: disabled
- configured primary protections: always enabled
- autonomous operation: startup configuration only, not a runtime switch
- cell map: `VC1..VC(S-1), VC16`
- sleep permission: always enabled

## Overcurrent tiers

The device provides three discharge-overcurrent mechanisms:

- `discharge_overcurrent` maps to OCD1: lower threshold, longer delay
- `discharge_severe_overcurrent` maps to OCD2: higher threshold, shorter delay
- `discharge_sustained_overcurrent` maps to OCD3: seconds-scale sustained overload

The schema rejects an OCD2 setup that is not both higher-current and faster than OCD1, because otherwise the second tier is redundant.

## Precharge and predischarge

- `PCHG` is a reduced-current charging path for deeply depleted cells and uses start/stop cell-voltage thresholds.
- `PDSG` is a reduced-current discharge path for charging load-side capacitance before enabling DSG.
- Predischarge timeout and stop delta are each stored in one byte with 10-unit resolution, so both have a maximum value of 2550.

## SoC and coulomb counter

The BQ76952 integrated-charge value is an internal coulomb-counter position in Ah. It is not energy in Wh and is not exposed as a lifetime counter or reset button. The service feeds counter deltas into the SoC estimator.

`cell_chemistry: lithium_ion` is required because lithium-ion is currently the only implemented voltage fallback curve.

## OTP programming

### ⚠️ DANGER: ONE-TIME OTP PROGRAMMING

> **The BQ76952 OTP can only be programmed once. This operation is irreversible. A wrong setting can permanently make the device unusable for the intended board.**
>
> Do not expose or press the OTP button during normal development. Verify the complete live configuration, regulator outputs, FET behaviour, protection thresholds, cell map, thermistors, and communication mode on production-representative hardware first.

Only add the factory action to a dedicated manufacturing configuration:

```yaml
bq76952:
  # ...fully validated configuration...

  # DANGER: irreversible, one-time device programming.
  program_factory_otp:
    name: "DANGER - Program BMS OTP Once"
```

The normal component configuration is applied live and does not require OTP programming.
