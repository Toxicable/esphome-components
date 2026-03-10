# mcf8316d_manual

Manual validation component for TI MCF8316D over ESPHome I2C (ESP32 + esp-idf).
`inter_byte_delay_us` is currently informational and not applied when using standard ESPHome I2C transactions.
The component forces MPET control bits off during setup so manual bring-up does not auto-enter MPET.
MCF8316D can still auto-enter MPET on non-zero speed if `CLOSED_LOOP2/3/4` motor parameters are zero (`MOTOR_RES`, `MOTOR_IND`, `MOTOR_BEMF_CONST`, speed-loop `Kp/Ki`).
To avoid that forced MPET path on blank parts, setup now seeds those zero fields with minimal non-zero shadow values (no EEPROM write).
Setup also forces `GD_CONFIG2.BUCK_CL` to the 600mA mode in shadow registers for manual validation, because the 150mA mode can trip immediate `DRV_BUCK_OCP`/`DRV_BUCK_UV` on loaded boards.
For safety, the component forces speed to 0% on persistent faults, but allows controller `LOCK_LIMIT`/`HW_LOCK_LIMIT`-only startup events to auto-retry.
`DRV_BUCK_OCP`/`DRV_BUCK_UV` are condition-active buck faults; `clear_faults` cannot clear them while the buck rail/load issue persists.
Optional `apply_startup_tune` button writes a practical startup profile in RAM (no EEPROM write): forces `speed=0%`, `direction=cw`, `brake=off`, `MTR_STARTUP=double_align`, `ALIGN_TIME=100ms`, `ALIGN_ANGLE=90°`, `MAX_SPEED=0x2710` (1666Hz electrical), `PWM_FREQ_OUT=60kHz`, enables dynamic CSA gain (`DEVICE_CONFIG2.DYNAMIC_CSA_GAIN_EN=1`), sets base CSA gain to `0.15V/A` (`GD_CONFIG1.CSA_GAIN=0`), `HW_LOCK_ILIMIT=8A`, `HW_LOCK_ILIMIT_DEG=7us`, `HW_LOCK_ILIMIT_MODE=retry_hiz`, `LOCK_ILIMIT_DEG=5ms`, `LCK_RETRY=1s`, `ALIGN_OR_SLOW_CURRENT_ILIMIT=2.5A`, `OL_ILIMIT=2.5A`, `OPN_CL_HANDOFF_THR=9%`, `SLOW_FIRST_CYC_FREQ=0.3%`, `FIRST_CYCLE_FREQ_SEL=1`, and disables ISD startup braking path (`ISD_EN=0`, `BRAKE_EN=0`, `RESYNC_EN=0`) to avoid long `MOTOR_BRAKE_ON_START` holds during manual bring-up.
Optional `apply_hw_lock_report_only` button is a temporary diagnostic mode that sets `HW_LOCK_ILIMIT_MODE`, `LOCK_ILIMIT_MODE`, and `MTR_LCK_MODE` to `disabled` (no protective lock shutdown action), forces `direction=cw` + `brake=off`, and forces `MTR_STARTUP=align` with `ALIGN_TIME=100ms`; use only for brief no-load debugging and then run `apply_startup_tune` to restore normal `retry_hiz` modes.
Optional `run_startup_sweep` button runs an automated 4-step startup test (`1.0A`, `1.5A`, `2.0A`, `2.5A` align/open-loop current limits) at `21%` speed command, logging PASS/FAIL/TIMEOUT per step based on state transition (`OPEN_LOOP`/`CLOSED_LOOP`) and fault status. The sweep now inserts inter-step delay and waits for fault-clear before starting the next step so each step is independent.
When commanded duty/voltage magnitude are non-zero and no fault is active, the component logs `[loop_run_state]` with `ALGORITHM_STATE` so startup stalls (for example stuck in `MOTOR_ALIGN`) are visible even without lock-limit faults.
Brake and direction writes now log immediate register readback (`PIN_CONFIG` / `PERI_CONFIG1`), and commanded-run diagnostics log `[loop_control] CTRL diag` with decoded `brake_sel`/`dir_sel`, key `ALGO_DEBUG1` bits (`CLOSED_LOOP_DIS` and force-state bits), and `ISD_CONFIG` fields so you can verify the chip is not being held in startup brake configuration. Lock-limit diagnostics now also include `[loop_lock_limit] DRIVE cfg` with `CLOSED_LOOP1.PWM_FREQ_OUT`, `DEVICE_CONFIG2` dynamic-gain bits, `GD_CONFIG1.CSA_GAIN`, and `CSA_GAIN_FEEDBACK`.
`Duty Cmd %` decodes `ALGO_STATUS[15:4]` per datasheet.

Recommended logger settings for cleaner diagnostics:

```yaml
logger:
  level: INFO
```

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mcf8316d_manual ]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  frequency: 50kHz

mcf8316d_manual:
  id: mcf
  address: 0x01
  update_interval: 250ms
  inter_byte_delay_us: 100
  auto_tickle_watchdog: false

switch:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Brake"

select:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Direction"

number:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Speed %"

button:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    clear_faults:
      name: "MCF Clear Faults"
    # Optional:
    # watchdog_tickle:
    #   name: "MCF Watchdog Tickle"
    # apply_startup_tune:
    #   name: "MCF Apply Startup Tune"
    # apply_hw_lock_report_only:
    #   name: "MCF Locks Disabled (Debug)"
    # run_startup_sweep:
    #   name: "MCF Startup Sweep"

binary_sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    fault_active:
      name: "MCF Fault Active"
      # true means fault asserted (nFAULT active/low)
    sys_enable:
      name: "MCF Sys Enable"
      # ALGO_STATUS[2] SYS_ENABLE_FLAG: 1 means register control is active
      # (for example GUI/manual writes can control the device), not "motor is spinning".

sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    vm_voltage:
      name: "MCF VM Voltage"
    duty_cmd_percent:
      name: "MCF Duty Cmd %"
    volt_mag_percent:
      name: "MCF Volt Mag %"

text_sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    fault_summary:
      name: "MCF Fault Summary"
      # Comma-separated active faults from gate-driver + controller status.
      # Falls back to DRV_FAULT_ACTIVE / CTRL_FAULT_ACTIVE if only summary bits are set.
    # Optional:
    # algorithm_state:
    #   name: "MCF Algorithm State"
```
