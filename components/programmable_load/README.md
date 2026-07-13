# Programmable Load

ESPHome interface for a programmable electronic load used as both a general-purpose bench load and a battery test instrument.

This branch intentionally breaks the existing API. It defines the configuration and C++ boundaries before the old control implementation is replaced.

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

A fault always stops and releases the current owner. `fault_policy.auto_clear` only controls whether `fault` returns to `idle` after the condition remains absent for `clear_delay`; it never resumes the interrupted operation.

## Hardware and operating voltage limits

`hardware.maximum_voltage` is the absolute voltage rating of the hardware design and is always checked at runtime. `limits.maximum_voltage` is the normal operating limit and must be less than or equal to the hardware limit. The example uses the programmable-load board's 75 V absolute design limit.

## Calibration

Calibration is a core layer, not procedure-specific logic:

- current scale and offset are applied to the raw current sensor;
- voltage scale and offset are applied to the raw voltage sensor;
- DAC zero level and full-scale current map requested current to the output;
- configured values are defaults and the C++ interface allows a future guided calibration procedure to apply, persist or reset the same `Calibration` structure.

This interface does not publish calibration internals as normal user entities.

## DCR test

DCR is an explicit procedure rather than a background calculation attached to normal current changes. It:

1. settles and samples at an absolute baseline current;
2. applies an absolute pulse current;
3. settles and samples loaded voltage/current;
4. returns to baseline for recovery;
5. repeats and publishes one resistance result.

Only the start button and resistance result are exposed. Starting DCR while manual control or another procedure is running is rejected by the core.

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
