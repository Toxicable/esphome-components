# I2C Interface Specification

## Bus
- Role: STM32 (`ESPHigher`) acts as I2C slave
- 7-bit slave address: `0x43`
- External slave bus in this project: `I2C2`
- STM32 pins:
  - `PC4` = `I2C2_SCL`
  - `PA8` = `I2C2_SDA`

## Transaction Model
- Host writes a command byte to the slave.
- Some commands include a small payload immediately after the command byte.
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

### `0x10` - Get motor state
- Host write payload: `10`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x10`)
- `resp[1]`: motor state (`MCI_State_t`)
- `resp[2]`: current fault bitfield LSB
- `resp[3]`: current fault bitfield MSB
- `resp[4]`: measured mechanical speed RPM LSB (signed int16)
- `resp[5]`: measured mechanical speed RPM MSB
- `resp[6]`: speed reference RPM LSB (signed int16)
- `resp[7]`: speed reference RPM MSB

### `0x11` - Acknowledge motor fault
- Host write payload: `11`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x11`)
- `resp[1]`: result
  - `0`: success
  - `3`: rejected by motor-control state machine
- `resp[2]`: motor state (`MCI_State_t`)
- `resp[3]`: last motor command state (`MCI_CommandState_t`)
- `resp[4]`: current fault bitfield LSB
- `resp[5]`: current fault bitfield MSB
- `resp[6]`: occurred fault bitfield LSB
- `resp[7]`: occurred fault bitfield MSB

### `0x12` - Start motor
- Host write payload: `12`
- Host read length: 8 bytes

Response format matches `0x11`.

### `0x13` - Stop motor
- Host write payload: `13`
- Host read length: 8 bytes

Response format matches `0x11`.

### `0x14` - Program speed ramp
- Host write payload: `14 rr rr dd dd`
  - `rr rr`: target speed in RPM, little-endian signed int16
  - `dd dd`: ramp duration in ms, little-endian uint16
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x14`)
- `resp[1]`: result
  - `0`: success
  - `2`: invalid parameter
- `resp[2]`: motor state (`MCI_State_t`)
- `resp[3]`: last motor command state (`MCI_CommandState_t`)
- `resp[4]`: current fault bitfield LSB
- `resp[5]`: current fault bitfield MSB
- `resp[6]`: occurred fault bitfield LSB
- `resp[7]`: occurred fault bitfield MSB

### `0x20` - Telemetry: state and RPM
- Host write payload: `20`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x20`)
- `resp[1]`: motor state (`MCI_State_t`)
- `resp[2]`: current fault bitfield LSB
- `resp[3]`: current fault bitfield MSB
- `resp[4]`: measured mechanical speed RPM LSB (signed int16)
- `resp[5]`: measured mechanical speed RPM MSB
- `resp[6]`: speed reference RPM LSB (signed int16)
- `resp[7]`: speed reference RPM MSB

### `0x21` - Telemetry: phase currents
- Host write payload: `21`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x21`)
- `resp[1]`: status (`0`)
- `resp[2]`: `Ia` LSB (signed int16)
- `resp[3]`: `Ia` MSB
- `resp[4]`: `Ib` LSB (signed int16)
- `resp[5]`: `Ib` MSB
- `resp[6]`: phase current amplitude LSB (signed int16)
- `resp[7]`: phase current amplitude MSB

### `0x22` - Telemetry: dq currents
- Host write payload: `22`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x22`)
- `resp[1]`: status (`0`)
- `resp[2]`: `Iq` LSB (signed int16)
- `resp[3]`: `Iq` MSB
- `resp[4]`: `Id` LSB (signed int16)
- `resp[5]`: `Id` MSB
- `resp[6]`: `Iq_ref` LSB (signed int16)
- `resp[7]`: `Iq_ref` MSB

### `0x23` - Telemetry: dq voltages
- Host write payload: `23`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x23`)
- `resp[1]`: status (`0`)
- `resp[2]`: `Vq` LSB (signed int16)
- `resp[3]`: `Vq` MSB
- `resp[4]`: `Vd` LSB (signed int16)
- `resp[5]`: `Vd` MSB
- `resp[6]`: phase voltage amplitude LSB (signed int16)
- `resp[7]`: phase voltage amplitude MSB

### `0x24` - Telemetry: bus voltage and angle
- Host write payload: `24`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x24`)
- `resp[1]`: status (`0`)
- `resp[2]`: average bus voltage LSB (`uint16`, volts)
- `resp[3]`: average bus voltage MSB
- `resp[4]`: electrical angle LSB (`int16`, dpp)
- `resp[5]`: electrical angle MSB
- `resp[6]`: `Valpha` LSB (`int16`)
- `resp[7]`: `Valpha` MSB

### `0x25` - Telemetry: control phase and mode
- Host write payload: `25`
- Host read length: 8 bytes

Response bytes:
- `resp[0]`: command echo (`0x25`)
- `resp[1]`: motor state (`MCI_State_t`)
- `resp[2]`: control mode (`MC_ControlMode_t`)
- `resp[3]`: command state (`MCI_CommandState_t`)
- `resp[4]`: current fault bitfield LSB
- `resp[5]`: current fault bitfield MSB
- `resp[6]`: occurred fault bitfield LSB
- `resp[7]`: occurred fault bitfield MSB

Useful `MCI_State_t` values in this project:
- `0`: `IDLE`
- `4`: `START`
- `6`: `RUN`
- `8`: `STOP`
- `10`: `FAULT_NOW`
- `11`: `FAULT_OVER`
- `16`: `CHARGE_BOOT_CAP`
- `17`: `OFFSET_CALIB`
- `19`: `SWITCH_OVER`

Startup/loop interpretation:
- `START`: startup / open-loop startup phase
- `SWITCH_OVER`: transition from startup to closed-loop
- `RUN`: normal closed-loop operation

Useful `MC_ControlMode_t` values:
- `2`: `MCM_OPEN_LOOP_VOLTAGE_MODE`
- `3`: `MCM_OPEN_LOOP_CURRENT_MODE`
- `4`: `MCM_SPEED_MODE`
- `5`: `MCM_TORQUE_MODE`

Useful `MCI_CommandState_t` values:
- `0`: `MCI_BUFFER_EMPTY`
- `1`: `MCI_COMMAND_NOT_ALREADY_EXECUTED`
- `2`: `MCI_COMMAND_EXECUTED_SUCCESSFULLY`
- `3`: `MCI_COMMAND_EXECUTED_UNSUCCESSFULLY`

## Host Validation Rules
- Verify address ACK at `0x43`.
- Verify read length is exactly 8 bytes.
- Verify `resp[0] == command`.
- For `0x01`, verify `resp[1] == 0` before using temperature.
- For motor control commands, inspect `resp[1]`, `resp[2]`, and the returned fault fields.

## Notes
- This spec is implemented in:
  - `Src/app/i2c_slave_app.c`
  - `Inc/app/i2c_slave_app.h`
- If protocol changes, update this file and firmware together.
