# BQ769X0 Battery Monitor (Hybrid SOC)

ESPHome external component for the TI BQ76920/30/40 family. This component reads cell/pack voltages, board-approximate temperature, current, and estimates SOC using a hybrid coulomb-counter + OCV approach.

## Basic Configuration (all required fields shown)

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  frequency: 100kHz

bq769x0:
  id: bq
  address: 0x08
  cell_count: 4
  crc: true
  rsense_milliohm: 1

  # Required SOC configuration
  ocv_table:
    - mv: 3300
      soc: 0
    - mv: 3600
      soc: 50
    - mv: 4200
      soc: 100
  rest_current_threshold_ma: 50
  rest_min_seconds: 60
  rest_full_weight_seconds: 600
  rest_dvdt_threshold_mv_per_s: 0.5
  ocv_source: min_cell
  full_cell_mv: 4180
  full_hold_seconds: 30
  empty_cell_mv: 3300
  empty_hold_seconds: 10
  empty_discharge_current_ma: 200
  current_positive_is_discharge: true
  coulombic_eff_discharge: 1.0
  coulombic_eff_charge: 1.0
  learn_alpha: 0.2
  use_hw_fault_anchors: false

  # Optional sensors
  pack_voltage: # Optional
    name: "Pack Voltage"
  cell1_voltage: # Optional
    name: "Cell 1"
  cell2_voltage: # Optional
    name: "Cell 2"
  cell3_voltage: # Optional
    name: "Cell 3"
  cell4_voltage: # Optional
    name: "Cell 4"
  board_temp: # Optional
    name: "Board Temperature (approx)"
  current: # Optional
    name: "Pack Current"
  soc_percent: # Optional
    name: "SOC (%)"
  min_cell_mv: # Optional
    name: "Min Cell (mV)"
  avg_cell_mv: # Optional
    name: "Avg Cell (mV)"
  soc_confidence: # Optional
    name: "SOC Confidence"
  rest_state: # Optional
    name: "Rest State"

  # Optional binary sensors
  fault: # Optional
    name: "Fault"
  device_ready: # Optional
    name: "Device Ready"
  cc_ready: # Optional
    name: "CC Ready"
  soc_valid: # Optional
    name: "SOC Valid"

  # Optional text sensors
  mode: # Optional
    name: "Charge/Discharge Mode"

  # Optional controls
  clear_faults: # Optional
    name: "Clear Faults"
  cc_oneshot: # Optional
    name: "CC One Shot"
  force_full_anchor: # Optional
    name: "Force Full Anchor"
  force_empty_anchor: # Optional
    name: "Force Empty Anchor"
  clear_learned_capacity: # Optional
    name: "Clear Learned Capacity"

  # Optional balance correction (explicit)
  balance_correction:
    enabled: false
    balance_current_ma_per_cell: 50
    balance_duty: 0.70
```

## Notes

- The CC integration window is 250 ms; the component polls at 250 ms by default.
- SOC is estimated by combining CC integration and OCV corrections during rest periods.
- `rest_state` reports `rest` or `active`, while `soc_confidence` reports the current OCV blend weight (0–1).
- `mode` is derived from CHG_ON/DSG_ON bits (`charge`, `discharge`, `charge+discharge`, `standby`, or `safe` before DEVICE_XREADY).
