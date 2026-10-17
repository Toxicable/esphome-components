# AstralPool Halo active invariants

- This is an experimental, receive-only ESP32 BLE manufacturer-data decoder.
- Keep protocol offsets and scaling documented as provisional until matched against live readings or vendor documentation.
- The advertisement manufacturer ID is `0x0447`; payload type bytes currently identify pH (`0`) and ORP (`1`).
- An ESPHome scanner captured direct HPH and HORP `0x0447` manufacturer payloads on 17 October 2026. Candidate 24-bit LE measurement scaling: pH raw/25600, ORP raw/256. Candidate temperature: uint16 LE raw/100 - 10 °C. Examples and reference comparisons are in README.md.
- The pH 7.8 and ORP 623 mV HCHLOR readings were over two hours older than these payloads; nearby water temperature was about 20.2 °C. Do not claim exact validation against synchronized readings.
- Keep the ESPHome adapter small and do not infer calibration or chemistry beyond the decoded measurements.
