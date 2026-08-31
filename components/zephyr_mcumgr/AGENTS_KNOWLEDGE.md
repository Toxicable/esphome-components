# zephyr_mcumgr override

- This component intentionally overrides ESPHome's bundled `zephyr_mcumgr` for ESPHome 2026.8.x nRF52 builds using the Adafruit nRF52 bootloader path.
- Source baseline is ESPHome `2026.8.0` `esphome/components/zephyr_mcumgr`.
- Project patch: omit `/delete-node/ &reserved_partition_0;` from the generated MCUboot partition overlay because the `xiao_ble` Zephyr DTS used by SDK-NRF 2.9.2 does not define that node label, causing devicetree parsing to fail before MCUboot configuration.
- Keep the rest of the upstream implementation unchanged unless another verified incompatibility is found.
- Re-check this override against upstream ESPHome before upgrading past 2026.8.x; remove the override once upstream handles the absent partition label safely.
- BLE MCUmgr OTA still requires `zephyr_ble_server:`. Coexistence with custom BTHome advertising is a separate runtime concern and is not solved by this partition-overlay patch.
