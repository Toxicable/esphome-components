# BQ76952 Battery Monitor BMS

ESPHome external component for TI BQ76952 (3S to 16S packs over I2C).

It provides:
- core telemetry (cell voltages, BAT voltage, PACK voltage, load-detect pin voltage, current, die temperature, TS1/TS2/TS3 temperatures)
- accumulated passed-charge telemetry from the on-chip coulomb counter
- battery/FET/alarm status entities
- host controls for FET path, sleep-allow, and alarm-clear
- startup policy options for autonomous FET control and sleep mode

## Configuration (optional items commented out)

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq76952 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

bq76952:
  i2c_id: i2c_bus

  ## Number of cells in series (3 to 16):
  cell_count: 16

  ## Startup behavior:
  ## preserve = don't change device state on boot
  ## enable/disable send startup subcommands
  # apply_configuration_on_boot: false
  autonomous_fet_mode: preserve
  sleep_mode: preserve

  ## Optional current protection thresholds (applied at boot):
  # sense_resistor_milliohm: 1.0
  # charge_current_limit_a: 20.0
  # discharge_current_limit_a: 40.0
  # charge_current_delay_ms: 23
  # discharge_current_delay_ms: 23
  # current_recovery_time_s: 3

  ## Optional REG0/REG1 supply setup (applied at boot):
  ## Set reg0_enabled when using the BREG + external NPN preregulator route.
  # reg0_enabled: true
  # reg1_enabled: true
  # reg1_voltage: 3.3V

  ## Optional sensors:
  # bat_voltage:
  #   name: "BAT Voltage"
  # pack_voltage:
  #   name: "Pack Voltage"
  # ld_voltage:
  #   name: "Load Detect Pin Voltage"
  # cell1_voltage:
  #   name: "Cell 1"
  # cell2_voltage:
  #   name: "Cell 2"
  # cell3_voltage:
  #   name: "Cell 3"
  # cell4_voltage:
  #   name: "Cell 4"
  # cell5_voltage:
  #   name: "Cell 5"
  # cell6_voltage:
  #   name: "Cell 6"
  # cell7_voltage:
  #   name: "Cell 7"
  # cell8_voltage:
  #   name: "Cell 8"
  # cell9_voltage:
  #   name: "Cell 9"
  # cell10_voltage:
  #   name: "Cell 10"
  # cell11_voltage:
  #   name: "Cell 11"
  # cell12_voltage:
  #   name: "Cell 12"
  # cell13_voltage:
  #   name: "Cell 13"
  # cell14_voltage:
  #   name: "Cell 14"
  # cell15_voltage:
  #   name: "Cell 15"
  # cell16_voltage:
  #   name: "Cell 16"
  # current:
  #   name: "Current"
  # passed_charge:
  #   name: "Passed Charge"
  # passed_charge_time:
  #   name: "Passed Charge Time"
  # die_temperature:
  #   name: "Die Temperature"
  # ts1_temperature:
  #   name: "TS1 Temperature"
  #   pullup: 18k
  # ts2_temperature:
  #   name: "TS2 Temperature"
  #   pullup: 18k
  # ts3_temperature:
  #   name: "TS3 Temperature"
  #   pullup: 18k

  ## Optional text sensors:
  # security_state:
  #   name: "Security State"
  # operating_mode:
  #   name: "Operating Mode"
  # power_path_state:
  #   name: "Power Path State"
  # alarm_flags:
  #   name: "Alarm Flags"
  # safety_status_flags:
  #   name: "Safety Status Flags"

  ## Optional binary sensors:
  # sleep_mode_active:
  #   name: "Sleep Active"
  # cfgupdate_mode:
  #   name: "Config Update"
  # protection_fault:
  #   name: "Safety Fault"
  # permanent_fail:
  #   name: "Permanent Fail"
  # sleep_allowed_state:
  #   name: "Sleep Allowed"
  # alert_pin:
  #   name: "ALERT Pin"
  # chg_fet_on:
  #   name: "CHG FET"
  # dsg_fet_on:
  #   name: "DSG FET"
  # pdsg_fet_on:
  #   name: "PDSG FET"
  # autonomous_fet_enabled:
  #   name: "Autonomous FET"

  ## Optional controls:
  # power_path:
  #   name: "Power Path"
  # autonomous_fet_control:
  #   name: "Autonomous FET Control"
  # sleep_allowed_control:
  #   name: "Sleep Allowed Control"
  # clear_alarms:
  #   name: "Clear Alarms"
  # reset_passed_charge:
  #   name: "Reset Passed Charge"
  # apply_configuration:
  #   name: "Apply Configuration"
  # program_factory_otp:
  #   name: "OTP Program Factory Defaults"
```

## Config Options You’ll Likely Tune

- `i2c_id`: required whenever your ESPHome node defines more than one I2C bus
- `cell_count`: match your physical stack (`3..16`)
- `autonomous_fet_mode`: boot policy for FET firmware control (`preserve`, `enable`, `disable`)
- `sleep_mode`: boot policy for sleep allow (`preserve`, `enable`, `disable`)
- `predischarge_enabled`: boot-time setting for `Settings:FET:FET Options[PDSG_EN]` so the chip can turn on PDSG before DSG
- `apply_configuration_on_boot`: when `false`, skip all boot-time config writes and use the `apply_configuration` button instead
- `program_factory_otp` button: one-time factory operation that stages the requested config in RAM and then burns it into OTP startup storage; exposed as a diagnostic/factory action rather than a normal config control
- `sense_resistor_milliohm`: shunt resistor value used to convert current limits to chip thresholds
- `charge_current_limit_a`: charge overcurrent protection threshold
- `discharge_current_limit_a`: discharge overcurrent protection threshold (OCD1)
- `charge_current_delay_ms`: OCC trip delay (10ms to 426ms)
- `discharge_current_delay_ms`: OCD1 trip delay (10ms to 426ms)
- `current_recovery_time_s`: shared recovery timer for OCC/OCD protections (0s to 255s)
- `reg0_enabled`: enable the on-chip preregulator that drives the external BREG transistor
- `reg1_enabled`: enable the REG1 LDO output
- `reg1_voltage`: set REG1 to `1.8V`, `2.5V`, `3.0V`, `3.3V`, or `5.0V`
- `power_path` entity: runtime host command for `off`, `charge`, `discharge`, `bidirectional`
- `passed_charge`: accumulated coulomb-count total converted to amp-hours using the chip's configured user-current scale
- `passed_charge_time`: integration interval reported by the chip in seconds
- `ts1/ts2/ts3_temperature.pullup`: select `18k` or `180k` internal pull-up for the BQ thermistor bias

Current and voltage scaling are automatically detected from the chip configuration.
No manual unit settings are needed.

Passed charge notes:
- `passed_charge` reads `DASTATUS6` and reports the integrated passed charge in signed amp-hours.
- `passed_charge_time` reports the same accumulator window in seconds.
- `reset_passed_charge` sends the chip's `RESET_PASSQ()` subcommand to zero the integration state and restart timing.
- The device reports passed charge in `userAh`, so this component converts through the detected `userA` scaling from `DA Configuration`.

Diagnostic notes:
- `alarm_flags` is the coarse, latched alarm summary from `Alarm Status (0x62)`.
- `safety_status_flags` is the live decoded protection cause from `Safety Status A/B/C (0x03/0x05/0x07)`, for example `ocd1`, `scd`, `cuv`, `otd`, or `hwdf`.
- `pdsg_fet_on` shows whether the predischarge PFET is currently being driven during a load bring-up event.

REG1 notes:
- `reg1_enabled` and `reg1_voltage` program `Settings:Configuration:REG12 Config`.
- `reg0_enabled` programs `Settings:Configuration:REG0 Config`.
- If you use the BREG + external NPN preregulator route, enable both `reg0_enabled` and `reg1_enabled`.
- If REGIN is supplied externally, leave `reg0_enabled: false` and only enable `reg1_enabled`.
- These writes require `FULLACCESS` and briefly enter `CONFIG_UPDATE`.
- Boot-time regulator, TS-pin, and current-limit writes are intentionally delayed by 10 seconds after startup when `apply_configuration_on_boot: true`.
- If `apply_configuration_on_boot: false`, use the `apply_configuration` button to push regulator, TS-pin, current-limit, and boot-mode settings in one shot.
- `program_factory_otp` is separate from `apply_configuration` because OTP writes are irreversible bit-programming operations.

`bat_voltage` is the top-of-stack battery reading (legacy alias: `stack_voltage`).

Cell voltage entities are mapped to the first `cell_count` populated cell-voltage commands seen at startup, in
ascending command order. This supports sparse layouts such as `VC1-VC0`, `VC2-VC1`, `VC3-VC2`, and `VC16-VC15`
for a 4S pack, so `cell1..cell4` follow the actual populated differential measurements.

When `ts1_temperature`, `ts2_temperature`, or `ts3_temperature` is configured, this component programs that BQ76952
pin into thermistor mode at boot using the selected internal pull-up (`18k` default, or `180k`). The measurement
is configured as report-only thermistor data and is not assigned to cell/FET protections by this component.

`ts2_temperature` shares the TS2 pin with the BQ76952 SHUTDOWN wake function. If your design uses TS2 for wake,
do not also use it as a thermistor input.

Thermistor wiring:
- Wire the thermistor directly between `TS1`/`TS2`/`TS3` and `VSS`.
- Do not add an external pull-up on the TS pin; the BQ76952 biases the thermistor with its internal pull-up.
- Set `pullup: 18k` for a typical `10 kOhm NTC`.
- Set `pullup: 180k` for higher-value thermistors such as `100 kOhm`/`200 kOhm`.
- These boot writes require `FULLACCESS` and briefly enter `CONFIG_UPDATE`, just like the current-limit settings.

Current limit notes:
- These values are written when boot auto-apply runs, or when `apply_configuration` is pressed.
- Setting `charge_current_limit_a` automatically enables OCC and CHG-FET trip-on-OCC.
- Setting `discharge_current_limit_a` automatically enables OCD1 and DSG-FET trip-on-OCD1.
- They require the chip to be in `FULLACCESS`.
- Applying them enters `CONFIG_UPDATE` briefly, which turns FETs off during that short window.
- If requested values are already present in data memory, the component skips `CONFIG_UPDATE` and
  does not re-apply them.
- This component now uses a fixed 10-second post-boot delay for automatic writes instead of waiting on OTA state.

Control grouping:
- The runtime controls (`power_path`, `autonomous_fet_control`, `sleep_allowed_control`, `clear_alarms`, `reset_passed_charge`, `apply_configuration`) are normal config entities.
- `program_factory_otp` is intentionally separate and should be labeled with an `OTP` prefix because it is an irreversible factory-only action.

## Autonomous Mode

For BQ76952, autonomous FET operation corresponds to `Manufacturing Status[FET_EN] = 1`
(firmware controls CHG/DSG based on protections/recovery).

Use either approach:

1. Startup policy in YAML:
   - `autonomous_fet_mode: enable`
2. Runtime control entity:
   - enable `autonomous_fet_control` and turn it ON

Important:
- `FET_ENABLE()` is a toggle subcommand and requires `UNSEALED` or `FULLACCESS`.
- If the device is `SEALED`, the component will not be able to change this bit.
- For persistence across full resets/SHUTDOWN, use `program_factory_otp` during manufacturing.

## Factory OTP

`program_factory_otp` is intended for one-time factory use.

What it does:
- applies the requested live configuration first
- writes startup-default boot-mode bits for `autonomous_fet_mode` and `sleep_mode`
- runs `OTP_WR_CHECK()`
- runs `OTP_WRITE()`

What persists after a full BQ power cycle:
- regulator settings already written into data memory
- TS pin config already written into data memory
- current-limit data-memory settings
- startup default for autonomous FET mode via `Mfg Status Init[FET_EN]`
- startup default for sleep allow via `Power Config[SLEEP]`

Caution:
- OTP programming is not a normal runtime control path
- OTP bits are one-way programmed and cannot be reverted back to `0`
- run this only when the board is in its intended final factory configuration
