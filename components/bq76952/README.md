# BQ76952 Battery Monitor and Protector

ESPHome external component for TI `BQ76952` 3S–16S battery monitors.

> This branch currently defines the target interfaces. The previous monolithic implementation has not yet been migrated to them, so the component is not expected to compile until the protocol/service/SoC implementation move is completed.

## Design

The component is split into four boundaries:

- `bq76952_protocol.h`: register, subcommand, transfer-buffer, checksum, CRC, and CONFIG_UPDATE transport
- `bq76952_config.h`: complete deterministic desired device state
- `bq76952_service.h`: connection handling, reconciliation, measurement conversion, protections, and runtime actions
- `bq76952_soc.h`: host-derived state-of-charge estimation and persistence
- `bq76952.h`: ESPHome entity facade only

The component uses composition for the service and SoC implementation. There is no compatibility `ConfigState` adapter and no state-bag inheritance.

## Configuration philosophy

This is a hard-cut interface:

- all device policy groups are required
- every feature is explicitly enabled or disabled
- omitted values never mean “preserve whatever happens to be in the chip”
- an `S`-cell pack always maps to `VC1..VC(S-1), VC16`
- sleep is always allowed and is not user-configurable
- configuration is reconciled after communication is established, with no boot delay
- failures remain pending and are retried after communication recovery

## Example

```yaml
external_components:
  - source: github://Toxicable/esphome-components@agent/refactor-bq76952-layers
    refresh: 0s
    components: [bq76952]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq76952:
  i2c_id: i2c_bus
  address: 0x08
  update_interval: 1s

  cell_count: 4
  sense_resistor_milliohm: 1.0
  i2c_crc_enabled: false
  current_gain_policy: preserve_existing

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
    autonomous_enabled: true
    sleep_charge_enabled: true
    body_diode_threshold_ma: 0

    # PCHG: charge a deeply depleted pack through the precharge path.
    precharge:
      enabled: false
      start_cell_voltage_mv: 2500
      stop_cell_voltage_mv: 3000

    # PDSG: limit inrush into a discharged load/DC link.
    predischarge:
      enabled: true
      timeout_ms: 2550
      stop_delta_mv: 500

  balancing:
    charging_enabled: true
    relaxed_enabled: true
    minimum_cell_voltage_mv: 3000
    start_delta_mv: 40
    stop_delta_mv: 20
    idle_current_threshold_a: 0.1
    minimum_temperature_c: 0
    maximum_temperature_c: 45
    maximum_balanced_cells: 2

  protections:
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
  bat_voltage:
    name: "BMS BAT Voltage"
  ld_voltage:
    name: "BMS Load Detect Voltage"
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

  ts1_temperature:
    name: "BMS TS1 Temperature"
  die_temperature:
    name: "BMS Die Temperature"

  output_enabled_control:
    name: "BMS Output Enabled"
  autonomous_fet_control:
    name: "BMS Autonomous FET Control"
  clear_alarms:
    name: "BMS Clear Alarms"
  reset_passed_charge:
    name: "BMS Reset Passed Charge"
  program_factory_otp:
    name: "BMS Program Factory OTP"
```

## Precharge versus predischarge

They solve different problems:

- **Precharge (`PCHG`)** is a reduced-current charging path for a deeply depleted battery. It is controlled using cell-voltage start and stop thresholds.
- **Predischarge (`PDSG`)** is a reduced-current discharge path used to charge load-side capacitance before enabling the main discharge FET. It is controlled by `PDSG_EN`, a timeout, and the remaining voltage delta between the load/PACK side and top-of-stack.

The BQ76952 does not provide a symmetric `PCHG_EN` bit matching `PDSG_EN`; the service translates `precharge.enabled` into deterministic precharge threshold policy instead of pretending the two features are register-identical.

## Hard-cut migration

The previous flat and optional configuration surface is intentionally unsupported. In particular, there is no compatibility handling for:

- omitted protection groups
- `cell_channels`
- sleep mode selection
- boot configuration delay
- event logging or XCHG burst modes
- `otp_*` policy overrides
- the old individual protection keys

The nested schema is the target interface. Implementation code is migrated to these interfaces separately rather than preserving the previous internal member layout.
