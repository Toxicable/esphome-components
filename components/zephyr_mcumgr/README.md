# zephyr_mcumgr ESPHome 2026.8.x override

Temporary override of ESPHome's bundled `zephyr_mcumgr` for the Seeed XIAO nRF52840 factory Adafruit bootloader path.

ESPHome 2026.8.0 generates an MCUboot overlay containing `/delete-node/ &reserved_partition_0;`. With the SDK-NRF 2.9.2 `xiao_ble` devicetree this label is absent, so the build fails before MCUboot config. This override is the 2026.8.0 component with only that invalid delete removed.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [zephyr_mcumgr]

nrf52:
  board: xiao_ble
  dfu: true
  framework:
    libc_nano: false

logger:

zephyr_ble_server:

ota:
  - platform: zephyr_mcumgr
    transport:
      ble: true
```

Remove this override once upstream ESPHome safely handles boards where `reserved_partition_0` is not defined. This patch does not address advertiser ownership between `zephyr_ble_server` and independent custom BLE broadcasters such as the current BTHome validation component.
