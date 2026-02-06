# BQ769X0 Battery Monitor (Hybrid SOC)

ESPHome external component for the TI BQ76920/30/40 family. This component reads cell/pack voltages, board-approximate temperature, current, and estimates SOC using a hybrid coulomb-counter + OCV approach.

## Configuration (optional items commented out)

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  frequency: 100kHz

bq769x0:
  id: bq
  cell_count: 4
  chemistry: liion_lipo
  # update_interval: 250ms
  # address: 0x08
  # pack_voltage:
  #   name: "Pack Voltage"
  # cell1_voltage:
  #   name: "Cell 1 Voltage"
  # cell2_voltage:
  #   name: "Cell 2 Voltage"
  # cell3_voltage:
  #   name: "Cell 3 Voltage"
  # cell4_voltage:
  #   name: "Cell 4 Voltage"
  # board_temp:
  #   name: "Board Temperature"
  # current:
  #   name: "Pack Current"
  # soc_percent:
  #   name: "State of Charge"
  # min_cell_mv:
  #   name: "Min Cell mV"
  # avg_cell_mv:
  #   name: "Avg Cell mV"
  # fault:
  #   name: "BQ Fault"
  # device_ready:
  #   name: "BQ Device Ready"
  # mode:
  #   name: "BQ Mode"
  # clear_faults:
  #   name: "Clear BQ Faults"
```

## Optional sensors and entities

All optional entities are opt-in:

- Sensors: `pack_voltage`, `cell1_voltage` .. `cell4_voltage`, `board_temp`, `current`, `soc_percent`, `min_cell_mv`, `avg_cell_mv`
- Binary sensors: `fault`, `device_ready`
- Text sensor: `mode`
- Button: `clear_faults`

## Notes

- The CC integration window is 250 ms; the component polls at 250 ms by default.
- SOC uses built-in hybrid CC + OCV defaults for 4S `liion_lipo`; charge integration is suppressed while average cell voltage is rising to avoid polarity configuration.
- `mode` is derived from CHG_ON/DSG_ON bits (`charge`, `discharge`, `charge+discharge`, `standby`, or `safe` before DEVICE_XREADY).
