# Makita XGT Battery

ESPHome external component for reading Makita XGT battery telemetry over the battery UART interface.

This component is based on the protocol handling and command set used in:
- `twaymouth/XGT-Tester`
- `Malvineous/makita-xgt-serial`

It provides:
- model text
- charge count
- estimated health
- state of charge
- both battery temperature sensors
- lock status
- pack voltage
- per-cell voltages for all 10 series cells
- optional factory-reset button

## Configuration (optional items commented out)

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ makita_xgt ]

uart:
  id: xgt_uart
  tx_pin:
    number: GPIO2
    inverted: true
  rx_pin:
    number: GPIO1
    inverted: true
  baud_rate: 9600
  parity: EVEN
  stop_bits: 1

makita_xgt:
  uart_id: xgt_uart
  update_interval: 60s

  ## Optional entities:
  # model:
  #   name: "Model"
  # charge_count:
  #   name: "Charge Count"
  # health:
  #   name: "Health"
  # charge:
  #   name: "Charge"
  # temperature_1:
  #   name: "Temperature 1"
  # temperature_2:
  #   name: "Temperature 2"
  # lock_status:
  #   name: "Lock Status"
  # pack_voltage:
  #   name: "Pack Voltage"
  # cell_size:
  #   name: "Cell Size"
  # parallel_count:
  #   name: "Parallel Count"
  # cell1_voltage:
  #   name: "Cell 1"
  # cell2_voltage:
  #   name: "Cell 2"
  # cell3_voltage:
  #   name: "Cell 3"
  # cell4_voltage:
  #   name: "Cell 4"
  # cell5_voltage:
  #   name: "Cell 5"
  # cell6_voltage:
  #   name: "Cell 6"
  # cell7_voltage:
  #   name: "Cell 7"
  # cell8_voltage:
  #   name: "Cell 8"
  # cell9_voltage:
  #   name: "Cell 9"
  # cell10_voltage:
  #   name: "Cell 10"

  ## Unsafe control. This sends the battery factory-reset sequence.
  ## Only enable it if you understand the consequences.
  # factory_reset:
  #   name: "Factory Reset"
```

## Notes

- The upstream example used an ESP32-C3 because the battery link needs `9600 8E1` UART with inversion enabled.
- A 5V to 3.3V level shifter is required between the battery data pin and the MCU UART.
- The battery is polled synchronously; keep `update_interval` moderate, for example `30s` or `60s`.
- `health` is derived the same way as the upstream sketch: raw capacity divided by `cell_size * parallel_count`.
- `factory_reset` is intentionally optional because it can clear battery state and may damage or brick a pack.
