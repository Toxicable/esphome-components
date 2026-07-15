# Programmable Load

ESPHome component for a programmable electronic load, explicit electrical test procedures, and coordinated battery cycling with a Charger_14 board.

## Architecture

The core owns the DAC, measurements, calibration, cooling, safety limits, state, fault, and one exclusive operation slot. A procedure receives read-only measurement snapshots and returns requests; it cannot access the DAC, charger, or component directly.

Only one of these may run at a time:

- manual constant-current operation;
- DCR test;
- battery drain/charge cycle.

The public state remains `idle`, `running`, or `fault`. Procedure-specific progress is optional and is not added to the core state model.

## Charger_14 battery cycle

The battery-cycle procedure performs this sequence:

1. force Charger_14 charging off;
2. discharge at the configured constant current;
3. require battery voltage to remain at or below the cutoff for the configured hold time;
4. stop the load and rest the battery;
5. enable Charger_14;
6. wait for a valid BQ25756 charging state;
7. integrate charge until BQ25756 reports `termination_done` for the configured hold time;
8. disable charging and return the load to `idle`.

The cutoff must be above `limits.minimum_voltage`. This lets the procedure end normally before the core undervoltage protection trips.

Charger_14 is used in its **external ESPHome control mode**. The cycle references the entities published by the board's `bq25756` component:

- `charge_enable`;
- `ibat_current` in mA;
- `vbat_voltage` in mV;
- `charge_status`;
- `status_flags`.

Charge voltage, charge current, input-current and input-voltage limits remain owned by the `bq25756` configuration for the stuffed Charger_14 variant. The cycle only enables or disables an already configured charger. It does not write raw charger registers or duplicate board policy. The Charger_14 enable control may also be used for manual charging while the programmable load is idle; the load output remains forced off in that state.

When a Charger_14 adapter is configured, the programmable-load core owns its charge-enable switch and keeps it off outside the charging phase. The core also guarantees that a non-zero load request and charger enable cannot be active together.

The onboard Charger_14 STM32 firmware is a separate autonomous mode and currently has no host command protocol. This procedure therefore does not target the onboard STM32.

### Completion and faults

Normal completion requires BQ25756 `charge_status` to remain `termination_done` for `termination_hold_time`.

The cycle faults and disables both load and charger on:

- stale or missing Charger_14 telemetry;
- Charger_14 failing to acknowledge charge enable/disable;
- BQ25756 watchdog, reverse-mode, timer, voltage, current, thermal, or driver faults;
- failure to begin charging within `charge_start_timeout`;
- charging stopping for longer than `charge_stall_timeout` after activity has begun;
- exceeding `charge_timeout`.

Input-current and input-voltage DPM states are not treated as faults because they are normal regulation modes.

## Calibration

Current and voltage scale/offset are applied before limits or procedures see measurements. DAC zero level and full-scale current map a requested current to the output. Calibration can be restored from preferences, replaced atomically, persisted, or reset to configured defaults.

## Configuration

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ programmable_load, bq25756 ]

# Charger_14 BQ25756 control. These limits must match the stuffed board variant.
bq25756:
  id: charger14_bq
  i2c_id: i2c_ext
  address: 0x6B
  update_interval: 1s
  disable_watchdog: true
  disable_ce_pin: true
  disable_ilim_hiz_pin: true
  disable_ichg_pin: true
  charge_voltage_limit_mv: 1536
  charge_current_limit_ma: 5000
  input_current_dpm_limit_ma: 3000
  input_voltage_dpm_limit_mv: 12000

  ibat_current:
    id: charger14_ibat
    internal: true
  vbat_voltage:
    id: charger14_vbat
    internal: true
  charge_status:
    id: charger14_charge_status
    internal: true
  status_flags:
    id: charger14_status_flags
    internal: true
  charge_enable:
    id: charger14_charge_enable
    internal: true

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
    minimum_voltage: 40
    maximum_voltage: 60
    maximum_power: 500
    maximum_temperature: 100

  control:
    period: 50ms
    deadband: 0.01
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

    battery_cycle:
      charger14:
        charge_enable: charger14_charge_enable
        ibat_current: charger14_ibat
        vbat_voltage: charger14_vbat
        charge_status: charger14_charge_status
        status_flags: charger14_status_flags
        sample_timeout: 3s
        control_timeout: 5s

      discharge_current: 10
      discharge_cutoff_voltage: 42
      discharge_cutoff_hysteresis: 0.5
      discharge_cutoff_hold_time: 10s
      rest_time: 5min
      charge_start_timeout: 1min
      charge_stall_timeout: 2min
      charge_timeout: 24h
      termination_hold_time: 10s

      start:
        name: "Run Battery Drain and Charge Cycle"
      phase:
        name: "Battery Cycle Phase"
      result:
        name: "Battery Cycle Result"
      discharged_capacity:
        name: "Discharged Capacity"
      discharged_energy:
        name: "Discharged Energy"
      charged_capacity:
        name: "Charged Capacity"
      charged_energy:
        name: "Charged Energy"
```

The example voltages are illustrative. Use cutoff and charge settings appropriate for the actual cell count, chemistry, BMS and Charger_14 hardware variant.
