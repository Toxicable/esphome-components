# AstralPool Halo active invariants

- This is an experimental, receive-only ESP32 BLE manufacturer-data decoder.
- Keep protocol offsets and scaling documented as provisional until matched against live readings or vendor documentation.
- The advertisement manufacturer ID is `0x0447`; payload type bytes currently identify pH (`0`) and ORP (`1`).
- Keep the ESPHome adapter small and do not infer calibration or chemistry beyond the decoded measurements.
