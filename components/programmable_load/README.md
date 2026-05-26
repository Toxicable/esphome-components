# Programmable Load

ESPHome external component for a programmable electronic load with closed-loop current control, safety monitoring, and DCR estimation.

Provides:
- Current setpoint control with adaptive ramping (fast/medium/slow based on error)
- Unconfirmed-move tracking to wait for INA response before overdriving
- Safety interlocks: NTC presence, minimum input voltage, over-temperature
- Battery/lead DCR estimation during upward ramping
- Temperature-proportional fan PWM control

## Configuration (optional items commented out)

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ programmable_load ]

## Hardware wiring (example):
#
# i2c:
#   sda: GPIO20
#   scl: GPIO10
#   frequency: 400kHz
#
# output:
#   - platform: ledc
#     id: fan_pwm
#     pin: GPIO8
#     frequency: 25000 Hz
#
#   - platform: mcp4726
#     id: dac_command
#     address: 0x60
#     vref: vref_buffered
#     gain: 1x
#
# sensor:
#   - platform: ina2xx_i2c
#     model: INA228
#     address: 0x40
#     shunt_resistance: 0.001 ohm
#     current:
#       id: ina_current
#     bus_voltage:
#       id: ina_bus_voltage
#
#   ## Temperature sensors (TS1..TS4, any temperature source):
#   # - platform: ...
#   #   id: ts1_temperature
#   # - platform: ...
#   #   id: ts2_temperature

programmable_load:
  ## Required: DAC output that drives the load (0..1 maps to 0..max_current_a).
  dac_output: dac_command
  ## Required: PWM output for fan control.
  fan_output: fan_pwm
  ## Required: Sensor providing measured load current in amperes.
  current_sensor: ina_current
  ## Required: Sensor providing bus/input voltage in volts.
  voltage_sensor: ina_bus_voltage
  ## Required: List of temperature sensors (°C). Max is used for safety.
  temperature_sensors:
    - ts1_temperature
    - ts2_temperature
  ## Optional: Binary sensors that must be ON for load to run (one per NTC).
  # ntc_present_sensors:
  #   - ts1_ntc_present
  #   - ts2_ntc_present

  ## --- Safety limits ---
  # max_current_a: 40.0
  # voltage_min_v: 1.0
  # max_temp_c: 100.0

  ## --- Control loop ---
  # control_period_ms: 50
  # deadband_min_a: 0.010
  # deadband_ratio: 0.002
  # current_response_min_a: 0.020
  # near_target_min_band_a: 0.160
  # max_unconfirmed_rise_a: 1.000
  # max_unconfirmed_fall_a: 2.000
  # ramp_fast_a_per_s: 8.0
  # ramp_medium_a_per_s: 4.0

  ## --- DCR estimation ---
  # dcr_min_delta_current_a: 0.500

  ## --- Fan ---
  # fan_start_temp_c: 35.0
  # fan_full_temp_c: 65.0

  ## --- Generated entities ---
  # setpoint:
  #   name: "Load Current Setpoint"
  # current_command:
  #   name: "Load Current Command"
  # dcr:
  #   name: "Battery Ramp DCR"
  # voltage_drop:
  #   name: "Battery Ramp Voltage Drop"
  # current_delta:
  #   name: "Battery Ramp Current Delta"
  # fault_ntc_missing:
  #   name: "Load Fault NTC Missing"
  # fault_no_voltage:
  #   name: "Load Fault No Voltage"
  # fault_over_temp:
  #   name: "Load Fault Over Temperature"
```

## Notes

- The control loop runs at `control_period_ms` (default 50 ms) via an internal interval.
- Ramp rates are tiered: fast (>5 A error), medium (>2 A), then fixed steps that decrease as the target is approached.
- Near the target, the loop waits for visible INA response before adding more DAC command, preventing overshoot.
- DCR estimation captures the V/I baseline when a new non-zero setpoint is applied and updates continuously during upward ramping.
- Fan PWM ramps linearly between `fan_start_temp_c` and `fan_full_temp_c`.
- All generated entities are optional; omit any that are not needed.
