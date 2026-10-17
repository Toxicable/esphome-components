# AstralPool Halo BLE water-quality probes

This experimental ESPHome external component listens for AstralPool Halo pH and ORP probe advertisements and publishes pH, ORP in millivolts, and each probe's reported temperature.

## External component

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [astral_ble_ph_orp]
```

The node must use an ESP32 with Bluetooth enabled and an `esp32_ble_tracker` configured. When the tracker has an explicit ID, set `esp32_ble_id` to that ID.

## Example

```yaml
esp32_ble_tracker:
  id: ble_tracker

astral_ble_ph_orp:
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
| pH | 15–17 | Little-endian unsigned 24-bit value, divided by 25,600 |
| ORP | 15–17 | Little-endian unsigned 24-bit value, divided by 256, reported as mV |
| Probe temperature | 18–19 | Little-endian unsigned 16-bit value, divided by 100 then minus 10 °C |

### Captured advertisements (17 October 2026)

The following manufacturer data was captured directly from the HPH and HORP devices with an ESPHome BLE scanner (the two-byte manufacturer ID `0x0447` is not part of the hex strings):

| Probe | Manufacturer data | Decoded values |
| --- | --- | --- |
| HPH | `040001008b862300f0010001000200e20403ca0b00000000` | pH 7.7288, temperature 20.18 °C |
| HORP | `040101006c762300f0010001000200ca6c02bc0b00000000` | ORP 620.79 mV, temperature 20.04 °C |

At the time, the last HCHLOR readings available were **pH 7.8** and **ORP 623 mV**, but they were more than two hours older than these advertisements. Nearby pool water was measured at approximately **20.2 °C**.

**The byte offsets and scales are still provisional.** The decoded values match the available reference measurements reasonably closely, but the reference readings were not simultaneous and vendor documentation has not confirmed the encoding. Treat the entities as experimental until further verified.
