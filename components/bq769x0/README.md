# BQ769X0 Battery Monitor

ESPHome external component for the TI BQ76920/30/40 family. This component reads cell/pack voltages, a board-approximate temperature, and current from the coulomb counter ADC. It can also clear fault/status bits and report charge/discharge mode.

## Basic Configuration

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
  update_interval: 5s
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
  current: # Optional (requires rsense_milliohm)
    name: "Pack Current"
  soc: # Optional (requires capacity_mah)
    name: "State of Charge"
  rsense_milliohm: 1 # Optional (required when current is set)
  capacity_mah: 2500 # Optional (required when soc is set)
  initial_soc: 100 # Optional (default 100)
  fault: # Optional
    name: "Fault"
  device_ready: # Optional
    name: "Device Ready"
  cc_ready: # Optional
    name: "CC Ready"
  mode: # Optional
    name: "Charge/Discharge Mode"
  clear_faults: # Optional
    name: "Clear Faults"
  cc_oneshot: # Optional
    name: "CC One Shot"
```

## Configuration Options

- **cell_count** (**Required**, 3–5): Number of cells in the stack.
- **crc** (*Optional*, default `true`): Enable CRC-on-I²C (for CRC-capable variants).
- **address** (*Optional*, default `0x08`): I²C address.
- **update_interval** (*Optional*, default `5s`).
- **rsense_milliohm** (*Required when `current` is configured*): Sense resistor value in milliohms.
- **capacity_mah** (*Required when `soc` is configured*): Pack capacity in mAh.
- **initial_soc** (*Optional*, default `100`): Initial SOC estimate for integration (%).

## Optional Sensors

- **pack_voltage**: Pack voltage sensor (V).
- **cell1_voltage** ... **cell5_voltage**: Cell voltage sensors (V).
- **board_temp**: Board-approximate temperature (°C).
- **current**: Current in mA (requires `rsense_milliohm`).
- **soc**: State of charge (%) estimated by integrating CC current over time.

## Optional Binary Sensors

- **fault**: True when any UV/OV/SCD/OCD status bit is set.
- **device_ready**: SYS_STAT.DEVICE_XREADY.
- **cc_ready**: SYS_STAT.CC_READY.

## Optional Controls

- **clear_faults**: Button to clear SYS_STAT fault bits.
- **cc_oneshot**: Button to set CC_ONESHOT and wait for CC_READY.

## Optional Text Sensors

- **mode**: Charge/discharge mode derived from CHG_ON/DSG_ON bits (`charge`, `discharge`, `charge+discharge`, `standby`, or `safe` before DEVICE_XREADY).

## Notes on CC and CHG/DSG

- **CC** is the coulomb counter ADC that integrates the SRP/SRN sense voltage; the `current` sensor converts it using `rsense_milliohm` (polarity depends on SRP/SRN wiring).
- **cc_oneshot** triggers a single CC conversion by setting `CC_ONESHOT` and then waiting for `CC_READY`.
- **mode** reflects the CHG_ON/DSG_ON bits in `SYS_CTRL2`, which indicate whether the external charge/discharge FET drivers are enabled.
- **SOC** is computed by integrating CC current over time, so it depends on accurate `capacity_mah`, a reasonable `initial_soc`, and a stable update interval.
