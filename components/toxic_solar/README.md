# toxic_solar

Board-level XIAO nRF52840 OpenThread settings, parent RSSI and battery sensors,
and MCUmgr OTA platform. Transmit power defaults to 0 dBm; set it explicitly
for a range test. The component keeps the XIAO battery-voltage divider enabled
and selects the onboard charger current through P0.13.

```yaml
external_components:
  - source:
      type: local
      path: ../../esphome-components/components
    components: [toxic_solar]

toxic_solar:
  tx_power: 8
  battery_charge_current_ma: 100
  boot_indicator: true
  battery_voltage:
    id: battery_voltage
    update_interval: 60s

sensor:
  - platform: toxic_solar
    parent_last_rssi:
      name: Thread Parent RSSI
      update_interval: 500ms
    battery_percent:
      name: Battery Level
      voltage_id: battery_voltage

ota:
  - platform: toxic_solar
    transport:
      ble: true

zephyr_ble_server:
```

The battery percentage sensor publishes whenever its voltage source updates;
project sampling sequences only need to trigger the ADC measurement. Its lookup
uses a Samsung INR18650-30Q OCV/SOC model at 25 °C, so 3.5 V maps to about 29%.
This is a rough estimate rather than fuel-gauge SOC; load, temperature, aging,
and cell variation change the voltage. [Reference curve and data](https://www.mdpi.com/2079-9292/12/12/2587).
The OTA platform retains the MCUmgr BLE/UART transports and XIAO bootloader overlay.

`battery_charge_current_ma` accepts 50 or 100. On the XIAO nRF52840, 50 mA
leaves P0.13 high-impedance; 100 mA drives P0.13 low. The default is 50 mA.
XIAOSolar selects 100 mA to match its board design. The component also holds
P0.14 low to enable the battery divider. `battery_voltage` creates the internal
P0.31 ADC sensor, applies the divider scale factor 2.9608, and lets a project
choose periodic or on-demand updates (`never`). `boot_indicator: true` flashes
the active-low red LED on P0.26 twice during startup.
