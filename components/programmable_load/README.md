# Programmable Load

ESPHome interface for a programmable electronic load used as both a general-purpose bench load and a battery test instrument.

This version is an intentional API break. It defines the v2 configuration and C++ interfaces before the control and procedure implementations are replaced.

## Interface model

The component is split into four layers:

1. **Hardware boundary** — DAC, fan output, and measurement sensor references.
2. **Safety supervisor** — absolute current, voltage, power, temperature, and measurement-freshness limits.
3. **Operation host** — owns the single user-facing state and fault and ensures only one manual operation or procedure controls the load.
4. **Procedures** — explicit tests such as DCR measurement. Procedures request current from the host and cannot bypass safety limits.

The base component intentionally publishes only:

- manual current setpoint;
- load state;
- current or last fault;
- optional clear-fault control.

Measurement entities remain owned by their sensor components, so users publish only the voltage, current, power, energy, or other values they actually need.

## State

The load has one state shared by manual operation and all procedures:

- `idle`
- `running`
- `paused`
- `complete`
- `fault`

A procedure must acquire the operation host before it can request current. Manual current and procedures therefore cannot drive the DAC concurrently.

## Faults

The load has one fault field rather than separate binary entities. Fault values include:

- `none`
- `current_measurement_unavailable`
- `voltage_measurement_unavailable`
- `required_temperature_unavailable`
- `input_undervoltage`
- `input_overvoltage`
- `overcurrent`
- `overpower`
- `overtemperature`
- `control_error`
- `procedure_error`

A fault always stops the active operation. `fault_policy.auto_clear` controls only whether the component returns from `fault` to `idle` after the fault condition has remained absent for `clear_delay`. With auto-clear disabled, the user must use the configured clear-fault button.

## DCR test

DCR is an explicit test procedure, not a calculation attached to normal current changes. The procedure:

1. settles and samples at the baseline current;
2. applies the configured pulse current;
3. settles and samples the loaded voltage/current;
4. returns to baseline for recovery;
5. repeats and publishes one resistance result.

Only the start button and resistance result are exposed.

## Configuration

```yaml
programmable_load:
  id: load_controller

  output:
    dac: dac_command
    dac_full_scale_current: 80.1

  measurements:
    current: load_current
    voltage: load_voltage
    sample_timeout: 250ms
    temperatures:
      - sensor: heatsink_temperature
        required: true

  limits:
    maximum_current: 40
    minimum_voltage: 1
    maximum_voltage: 75
    maximum_power: 500
    maximum_temperature: 100

  control:
    period: 50ms
    deadband: 0.001
    rise_rate: 2
    fall_rate: 4

  cooling:
    fan_output: fan_pwm
    fan_start_temperature: 35
    fan_full_temperature: 70

  fault_policy:
    auto_clear: false
    clear_delay: 2s

  manual_current:
    name: "Load Current Setpoint"

  state:
    name: "Load State"

  fault:
    name: "Load Fault"

  clear_fault:
    name: "Clear Load Fault"

  dcr_test:
    pulse_current: 5
    settle_time: 100ms
    sample_time: 500ms
    recovery_time: 1s
    repeats: 3

    start:
      name: "Run Battery DCR Test"

    resistance:
      name: "Battery DCR"
```
