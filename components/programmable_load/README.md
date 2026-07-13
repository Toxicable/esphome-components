# Programmable Load

ESPHome component for a programmable electronic load used as both a general-purpose bench load and a battery test instrument.

This version intentionally breaks the old API and replaces the previous monolithic ramp controller and background DCR estimator.

## Architecture

The core owns all hardware and safety-critical behaviour:

1. read raw measurements;
2. apply measurement calibration;
3. enforce the absolute hardware voltage limit;
4. enforce configured current, voltage, power and temperature limits;
5. run either manual control or one procedure;
6. clamp the requested current and drive the DAC using output calibration;
7. publish one state and one fault.

Control direction is **core to procedure**. Procedures do not hold a parent pointer, call core functions, access the DAC, or bypass limits. The core supplies calibrated measurements and consumes a `ProcedureResult` containing only status, requested current and an optional fault.

## Exclusive ownership

The core has one internal owner slot:

- none;
- manual control;
- one procedure.

A non-zero manual start or procedure start is rejected unless the load is idle. Procedures therefore lock out manual control and every other procedure. User stop, manual current returning to zero, or procedure completion releases ownership and returns directly to `idle`.

## State

The user-facing state is deliberately limited to:

- `idle`;
- `running`;
- `fault`.

Procedure completion is a result, not a persistent device state. There is no pause state or pause/resume API.

## Faults

The component publishes one fault field rather than separate binary entities. Fault values include:

- `none`;
- `current_measurement_unavailable`;
- `voltage_measurement_unavailable`;
- `required_temperature_unavailable`;
- `input_undervoltage`;
- `input_overvoltage`;
- `hardware_overvoltage`;
- `overcurrent`;
- `overpower`;
- `overtemperature`;
- `control_error`;
- `procedure_error`.

A fault always stops and releases the current owner. `fault_policy.auto_clear` only controls whether `fault` returns to `idle` after the original condition remains absent for `clear_delay`; it never resumes the interrupted operation.

## Hardware and operating voltage limits

`hardware.maximum_voltage` can describe a board-specific limit below 75 V, but the core has an independent 75 V absolute ceiling that configuration cannot raise. The hardware ceiling is checked even while the load is idle because the input circuitry remains electrically connected.

`limits.maximum_voltage` is the normal operating limit and must be less than or equal to the configured hardware limit. While running, the core checks both limits independently and reports either `input_overvoltage` or `hardware_overvoltage`.

## Calibration

Calibration is a core layer, not procedure-specific logic:

- current scale and offset are applied to the raw current sensor;
- voltage scale and offset are applied to the raw voltage sensor;
- DAC zero level and full-scale current map requested current to the output;
- configured values are retained as reset defaults;
- a valid persisted `Calibration` structure can replace those defaults at startup;
- applying and persisting a new calibration is atomic and is rejected while the load is running.

The component does not publish calibration internals as normal user entities. A later guided calibration procedure can submit a complete calibration to the same core API without accessing the DAC directly.

## Control

The current controller rate-limits the DAC-equivalent current command using the configured rise and fall rates. The measured-current error determines the direction and magnitude of each step, while the command remains bounded by:

- configured maximum current;
- calibrated DAC full-scale current;
- the instantaneous `maximum_power / voltage` limit.

Stale current or voltage samples fault the active operation. Required temperature sensors must be present and finite before an operation can start and throughout the run.

## DCR test

DCR is an explicit procedure rather than a background calculation attached to normal current changes. It:

1. settles and samples at an absolute baseline current;
2. applies an absolute pulse current;
3. settles and samples loaded voltage/current;
4. returns to baseline for recovery;
5. repeats and publishes the mean resistance result.

Only distinct electrical measurement frames are accumulated, so a fast control loop cannot count the same INA conversion repeatedly. Only the start button and resistance result are exposed. Starting DCR while manual control or another procedure is running is rejected by the core.

## Configuration

```yaml
programmable_load:
  id: load_controller

  hardware:
    dac: dac_command
    maximum_voltage: 75

  measurements:
    current: load_current
    voltage: load_voltage
    sample_timeout: 250ms
    temperatures:
      - sensor: heatsink_temperature
        required: true

  calibration:
    restore: true
    current:
      scale: 1.0
      offset: 0.0
    voltage:
      scale: 1.0
      offset: 0.0
    output:
      zero_level: 0.0
      full_scale_current: 80.1

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

  procedures:
    dcr:
      baseline_current: 0
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
