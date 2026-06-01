# I2C Interface Specification

## Bus
- Role: STM32 (`ESPHigher`) acts as I2C slave
- 7-bit slave address: `0x43`
- External slave bus in this project: `I2C2`
- STM32 pins:
  - `PC4` = `I2C2_SCL`
  - `PA8` = `I2C2_SDA`

## Transaction Model
- Host writes a 1-byte command to the slave.
- Host then reads an 8-byte response from the slave.
- Multi-byte fields are little-endian.

## Commands
### `0x01` - Get temperature state
- Host write payload: `01`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x01`)
- `resp[1]`: status
  - `0`: temperature state valid / no over-temp fault
  - `1`: temperature fault active (`MC_OVER_TEMP`)
- `resp[2]`: temperature Celsius LSB (signed int16)
- `resp[3]`: temperature Celsius MSB (signed int16)
- `resp[4]`: fault bitfield LSB (`MC_NO_ERROR` or fault mask such as `MC_OVER_TEMP`)
- `resp[5..7]`: reserved (`0x00`)

Temperature reconstruction:
- `temp_c = (int16_t)(resp[2] | (resp[3] << 8))`
- `fault = resp[4]` (bitmask from motor-control fault state, LSB)

## Host Validation Rules
- Verify address ACK at `0x43`.
- Verify read length is exactly 8 bytes.
- Verify `resp[0] == command`.
- Verify `resp[1] == 0` before using data.
- If `resp[1] != 0`, inspect `resp[4]` for fault bits.

## Notes
- This spec is implemented in:
  - `Src/app/i2c_slave_app.c`
  - `Inc/app/i2c_slave_app.h`
- If protocol changes, update this file and firmware together.
