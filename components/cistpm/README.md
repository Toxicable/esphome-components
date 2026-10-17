# CISTPM BLE water-quality probes

This experimental ESPHome external component listens for CISTPM pH and ORP probe advertisements and publishes pH, ORP in millivolts, and each probe's reported temperature.

## External component

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [cistpm]
```

The node must use an ESP32 with Bluetooth enabled and an `esp32_ble_tracker` configured. When the tracker has an explicit ID, set `esp32_ble_id` to that ID.

## Example

```yaml
esp32_ble_tracker:
  id: ble_tracker

cistpm:
  esp32_ble_id: ble_tracker
  ph:
    name: Pool pH
  ph_temperature:
    name: Pool pH Probe Temperature
  # Optional when more than one pH probe is in range:
  # ph_mac_address: "AA:BB:CC:DD:EE:FF"
  orp:
    name: Pool ORP
  orp_temperature:
    name: Pool ORP Probe Temperature
  # Optional when more than one ORP probe is in range:
  # orp_mac_address: "AA:BB:CC:DD:EE:FF"
```

The MAC filters apply separately to the pH and ORP advertisements. If omitted, the component accepts every matching probe of that type in range, so the last advertisement received updates the sensor.

## Payload interpretation

The component currently filters manufacturer ID `0x0447` and a 24-byte payload beginning with `04 00` for pH or `04 01` for ORP. The offsets below are zero-based from the start of the manufacturer payload, after ESPHome has removed the two-byte company identifier:

| Field | Bytes | Current interpretation |
| --- | --- | --- |
| pH | 15–16 | Little-endian unsigned Q8.8, divided by 256 |
| ORP | 15–17 | Little-endian unsigned Q16.8, divided by 256, reported as mV |
| Probe temperature | 18–19 | Little-endian unsigned integer, divided by 100, reported as °C |

These offsets and scales are provisional. They produce plausible readings from captured advertisements, but they have not yet been confirmed against vendor documentation or synchronized measurements from a calibrated reference. Treat resulting entities as experimental until that comparison is done.
