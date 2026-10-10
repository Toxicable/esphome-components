# ADP910 component knowledge

- This is the shared owner of the ADP910 ESPHome integration; board firmware must consume it through `external_components`.
- The implementation uses I2C address `0x25`, validates product type `0x4153`, and validates the two-byte response fields with CRC-8 polynomial `0x31` and initial value `0xFF`.
- Published values use the existing sensor scaling: pressure raw / 60 in Pa and temperature raw / 200 in degrees Celsius.
- Keep the component as a direct ESPHome adapter. Do not add compatibility wrappers or a second board-local copy.
