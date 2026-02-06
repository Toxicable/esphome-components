# BQ769X0 Battery Monitor (Hybrid SOC)

ESPHome external component for the TI BQ76920/30/40 family. This component reads cell/pack voltages, board-approximate temperature, current, and estimates SOC using a hybrid coulomb-counter + OCV approach.

## Basic Configuration (required)

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  frequency: 100kHz

bq769x0:
  id: bq
  cell_count: 4
  chemistry: liion_lipo
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
