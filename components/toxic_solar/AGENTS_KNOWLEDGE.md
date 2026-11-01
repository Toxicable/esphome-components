# toxic_solar

- The component owns XIAO nRF52840 OpenThread SED startup settings, parent-last-RSSI polling, and the battery percentage sensor.
- `tx_power` defaults to 0 dBm. Range-test profiles may explicitly set it to +8 dBm.
- Battery percentage is a voltage-based 1S estimate from the caller-supplied battery-voltage sensor; it is not a fuel-gauge measurement.
- The `ota/toxic_solar` platform owns the XIAO MCUmgr OTA configuration and partition overlay.
