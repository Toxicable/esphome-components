# AGENTS_KNOWLEDGE: mcf8329a

Component-scoped notes for `components/mcf8329a`.

- Datasheet source is `components/mcf8329a/datasheets/mcf8329a.pdf`; low-context derived text artifacts are generated from the PDF and the extracted text is not stored.
- Register-level control mirrors the `mcf8316d` architecture but uses MCF8329A-specific mappings.
- Important MCF8329A register differences used by this component:
  - `ALGORITHM_STATE` offset is `0x0196`.
  - `CSA_GAIN_FEEDBACK`/`VOLTAGE_GAIN_FEEDBACK`/`VM_VOLTAGE` offsets are `0x0450`/`0x0458`/`0x045C`.
  - `PIN_CONFIG.BRAKE_INPUT` is bits `[3:2]`.
  - `PERI_CONFIG1.DIR_INPUT` is bits `[20:19]`.
  - `ALGO_CTRL1.CLR_FLT` is bit `29`, `WATCHDOG_TICKLE` is bit `10`.
- `VM_VOLTAGE` is decoded as full 32-bit Q27 (`volts = raw * 60 / 2^27`) instead of an 11-bit field.
- Motor config can be set from YAML and is applied at setup:
  - Required in YAML: `mode`, `brake_mode`, `motor_bemf_const`,
    and `max_speed_hz` (explicit startup/stop strategy plus mandatory motor scaling).
  - `motor_bemf_const` -> `CLOSED_LOOP3.MOTOR_BEMF_CONST[30:23]`
  - `brake_mode` -> `CLOSED_LOOP2.MTR_STOP[30:28]`
  - `brake_time` -> `CLOSED_LOOP2.MTR_STOP_BRK_TIME[27:24]`
  - `mode` -> `MOTOR_STARTUP1.MTR_STARTUP[30:29]`
  - `align_time` -> `MOTOR_STARTUP1.ALIGN_TIME[24:21]`
  - `direction_mode` uses `PERI_CONFIG1.DIR_INPUT[20:19]`
  - `csa_gain_v_per_v` -> `GD_CONFIG1.CSA_GAIN[1:0]` (`5|10|20|40` V/V)
  - `base_current_amps` -> `GD_CONFIG2.BASE_CURRENT[14:0]` using
    `code ~= round(amps * 32768 / 1200)` (datasheet: `Base Current (A) = code * 1200 / 32768`)
  - `phase_current_limit_percent` -> `FAULT_CONFIG1.ILIMIT[30:27]` (phase peak current limit, % of BASE_CURRENT)
  - `align_or_slow_current_limit_percent` ->
    `MOTOR_STARTUP1.ALIGN_OR_SLOW_CURRENT_ILIMIT[20:17]`
  - `open_loop_limit_source` -> `MOTOR_STARTUP1.OL_ILIMIT_CONFIG[3]`
    (`ol_ilimit` => `0` uses `MOTOR_STARTUP2.OL_ILIMIT`; `ilimit` => `1` uses `FAULT_CONFIG1.ILIMIT`)
  - `lock_mode` applies the same mode code to:
    - `FAULT_CONFIG1.LOCK_ILIMIT_MODE[18:15]`
    - `FAULT_CONFIG1.MTR_LCK_MODE[6:3]`
    - `FAULT_CONFIG2.HW_LOCK_ILIMIT_MODE[18:15]`
  - Exposed handoff/lock smoothing controls:
    - `open_loop_accel2_hz_per_s2` -> `MOTOR_STARTUP2.OL_ACC_A2[22:19]`
    - `theta_error_ramp_rate` -> `MOTOR_STARTUP2.THETA_ERROR_RAMP_RATE[2:0]`
    - `cl_slow_acc_hz_per_s` -> `INT_ALGO_2.CL_SLOW_ACC[9:6]`
    - `mpet_use_dedicated_params` -> `INT_ALGO_2.MPET_KE_MEAS_PARAMETER_SELECT[1]`
    - `mpet_open_loop_curr_ref_percent` -> `INT_ALGO_1.MPET_OPEN_LOOP_CURR_REF[10:8]`
    - `mpet_open_loop_speed_ref_percent` -> `INT_ALGO_1.MPET_OPEN_LOOP_SPEED_REF[7:6]`
    - `mpet_open_loop_slew_hz_per_s` -> `INT_ALGO_1.MPET_OPEN_LOOP_SLEW_RATE[5:3]`
    - `mpet_timeout_ms` -> runtime timeout override for `run_mpet`
    - `lock_ilimit_deglitch_ms` -> `FAULT_CONFIG1.LOCK_ILIMIT_DEG[14:11]`
    - `hw_lock_ilimit_deglitch_us` -> `FAULT_CONFIG2.HW_LOCK_ILIMIT_DEG[14:12]`
  - Speed-loop override knobs:
    - `speed_loop_kp_code` writes raw 10-bit Kp code split across `CLOSED_LOOP3/4`; `0` keeps auto behavior.
    - `speed_loop_ki_code` writes raw 10-bit Ki code in `CLOSED_LOOP4`; `0` keeps auto behavior.
  - YAML `lock_mode` intentionally excludes `report_only` (mode `8`) for safer defaults during bring-up.
  - `lock_ilimit_percent` -> `FAULT_CONFIG1.LOCK_ILIMIT[22:19]`
  - `hw_lock_ilimit_percent` -> `FAULT_CONFIG1.HW_LOCK_ILIMIT[26:23]`
  - `lock_retry_time` -> `FAULT_CONFIG1.LCK_RETRY[10:7]`
  - `abn_speed_lock_enable`/`abn_bemf_lock_enable`/`no_motor_lock_enable` ->
    `FAULT_CONFIG2.LOCK1_EN[30]`/`LOCK2_EN[29]`/`LOCK3_EN[28]`
  - `lock_abn_speed_threshold_percent` -> `FAULT_CONFIG2.LOCK_ABN_SPEED[27:25]`
  - `abnormal_bemf_threshold_percent` -> `FAULT_CONFIG2.ABNORMAL_BEMF_THR[24:22]`
  - `no_motor_threshold_percent` -> `FAULT_CONFIG2.NO_MTR_THR[21:19]`
  - `max_speed_hz` -> `CLOSED_LOOP4.MAX_SPEED[13:0]` (encoded per datasheet piecewise mapping)
  - `open_loop_ilimit_percent` -> `MOTOR_STARTUP2.OL_ILIMIT[30:27]`
  - `open_loop_accel_hz_per_s` -> `MOTOR_STARTUP2.OL_ACC_A1[26:23]`
  - `auto_handoff_enable` -> `MOTOR_STARTUP2.AUTO_HANDOFF_EN[18]`
  - `open_to_closed_handoff_percent` -> `MOTOR_STARTUP2.OPN_CL_HANDOFF_THR[17:13]`
  - `allow_unsafe_current_limits` (default `false`) is a config guardrail override:
    - Without override, validation blocks `phase_current_limit_percent`, `open_loop_ilimit_percent`,
      `lock_ilimit_percent`, `hw_lock_ilimit_percent` above 50%.
    - Without override, validation also blocks `lock_mode: disabled`.
  - Quick conversion from common motor specs:
    - For `kV` in mechanical `rpm/V` and `pole_pairs`, estimate
      `KtPH_N(mV/Hz) ~= 48990 / (kV * pole_pairs)` then choose nearest `MOTOR_BEMF_CONST` table entry.
    - Estimate electrical speed limit as
      `max_speed_hz ~= (kV * Vbus_max * pole_pairs) / 60` (keep within schema range `1..3295`).
    - Example sanity points: `270kV, 7 pole-pairs -> ~26.0 mV/Hz -> 0x57`; `750kV, 7 pole-pairs -> ~9.3 mV/Hz -> 0x34`;
      `750kV, 12-pole rotor (6 pole-pairs), 4S full -> max_speed_hz ~1260 and bemf code near 0x39`.
- Added key-info telemetry fields:
  - `sensor.motor_bemf_constant` (from `MTR_PARAMS[23:16]`)
  - `sensor.speed_fdbk_hz` (from `SPEED_FDBK`, Q27 scaled by effective `MAX_SPEED`)
  - `sensor.speed_ref_open_loop_hz` (from `SPEED_REF_OPEN_LOOP`, Q27 scaled by effective `MAX_SPEED`)
  - `sensor.fg_speed_fdbk_hz` (from `FG_SPEED_FDBK`, Q27 scaled by effective `MAX_SPEED`)
  - `text_sensor.current_fault` publishes decoded active faults (`none` or a comma-separated token list).
- Breaking YAML change (intentional): legacy `startup_*` keys were removed and now raise migration errors pointing to the new non-prefixed key names.
- Schema now enforces a hardware baseline before tuning (`tune_initial_params` or manual tuning keys):
  `csa_gain_v_per_v`, `base_current_amps`, `phase_current_limit_percent`, `open_loop_limit_source`, and `lock_mode`.
- README YAML layout preference:
  - Keep one example, but group optional motor-config knobs by purpose (direction/alignment, scaling, handoff, lock handling).
  - Omit default-valued keys from the example unless they materially aid bring-up.
  - Do not include any `apply_*_config` key; motor config is always applied.
  - Keep motor conversion math inline in the YAML `## Required motor keys` block (not in a separate top-of-README note).
  - Include an inline `mode` options comment near `mode` in the YAML example.
  - Include brief inline guidance on when to choose each `mode` option (`align`, `double_align`, `ipd`, `slow_first_cycle`).
  - Prefer variable names `Bs` (series cells) and `Rp` (rotor poles) in inline README motor equations.
  - Lock-handling comments should reflect that `lock_mode` supports `latched`, `retry`, and `disabled` only.
  - Use monolith integration style: controls/telemetry are configured inline under `mcf8329a:` (for example
    `brake`, `direction`, `speed_percent`, `clear_faults`, sensors/text/binary entries), not in separate platform blocks.
- Runtime behavior:
  - Bring-up automation is implemented through a dedicated `MCF8329ATuningController` helper class (kept separate from the main hardware component flow) and triggered by button actions.
  - `MCF8329ATuningController` ownership in `MCF8329AComponent` uses a raw pointer with out-of-line `delete` (not `std::unique_ptr`) to avoid incomplete-type `unique_ptr` destructor static-asserts with the ESPHome/toolchain build path.
  - Tuning implementation is split into `mcf8329a_tuning.cpp`/`mcf8329a_tuning.h`; the component keeps only orchestration calls (`start_tune_initial_params`, `start_mpet_characterization`, update hook).
  - Hardware register/bitfield constants are centralized in `mcf8329a_client.h` (`namespace regs`) so `MCF8329AComponent` stays focused on orchestration/HA bindings and does not own mask/shift definitions.
  - Low-level register transport is split into `mcf8329a_client.cpp`/`mcf8329a_client.h`; `MCF8329AComponent::read_reg32/read_reg16/write_reg32/update_bits32` delegate to `MCF8329AClient`.
  - `inter_byte_delay_us` is no longer user-configurable; the write-transaction delay is a fixed internal implementation detail in `mcf8329a_client.cpp` (`100us`) and is not exposed in public headers/API.
  - Build compatibility: `to_code()` adds `-DMCF8329A_EMBED_IMPL`; `mcf8329a.cpp` includes `mcf8329a_client.cpp` and `mcf8329a_tuning.cpp` in that mode, while those files compile as no-op standalone TUs unless included via `MCF8329A_EMBED_IMPL_INCLUDE`. This avoids undefined references on ESPHome setups that only link `mcf8329a.cpp`.
  - Because embed mode merges multiple `.cpp` files into one translation unit, file-scope `static` identifiers in embedded files must stay uniquely prefixed (for example `CLIENT_*`, `TUNING_*`) to avoid redefinition collisions with `mcf8329a.cpp`.
  - `MCF8329AClient` now owns key runtime field-masking APIs (`set_brake_input`, `set_direction_input`, `write_speed_command_raw`, `set_mpet_characterization_bits`, `pulse_clear_faults`, `pulse_watchdog_tickle`, `clear_mpet_bits`), and component control paths call these instead of open-coded `update_bits32` masks.
  - Decode helpers moved to `MCF8329AClient`: max-speed code -> Hz, speed/FG Q27 -> Hz, open-loop accel code -> Hz/s, handoff code -> %, and VM raw -> volts; component/tuning now call client decode APIs instead of local decode methods.
  - Non-zero speed commands auto-release brake (`PIN_CONFIG.BRAKE_INPUT=no_brake`) before writing speed.
  - `set_speed_percent(...)` now logs `INFO` lines with caller reason (`number_control`, `motor_init`, `fault_shutdown`)
    and raw speed code for command-traceability.
  - Optional speed command shaping is available via:
    - `speed_ramp_up_percent_per_s`
    - `speed_ramp_down_percent_per_s`
    - `start_boost_percent`
    - `start_boost_hold_ms`
    Non-zero commands can be ramped internally before register writes.
  - Runtime now emits a 2Hz `INFO` speed diagnostic line while commanded speed is active:
    `cmd`, `speed_ref_open_loop_hz`, `speed_fdbk_hz`, `fg_speed_fdbk_hz`, `max_speed_hz`, and read-valid flags.
  - Idle telemetry guard: if `speed_cmd` is effectively zero and `fg_speed_fdbk_hz` is implausibly high
    (`>150%` of `max_speed_hz`) while signed `speed_fdbk_hz` is near zero/unavailable, firmware now publishes
    `fg_speed_fdbk_hz=0` to suppress stale FG feedback spikes at idle.
  - Idle speed telemetry now applies a stronger stale/noise guard:
    - values under `20Hz` are snapped to `0Hz` when speed command is idle.
    - if speed registers fail to read while idle, speed entities are actively published as `0Hz` (no stale hold).
    - `speed_ref_open_loop_hz` is also forced to `0Hz` at idle when feedback is idle/unavailable.
  - Optional monolith buttons for guided bring-up:
    - `tune_initial_params`: runs a discovery sweep to reach closed-loop, then runs a refinement sweep around the first successful candidate using manual-handoff variants by default; logs the exact recommended YAML keys/values at `INFO` for manual copy.
    - Refinement still evaluates a discrete candidate set (iterative sweep), but candidate ranking now uses measured handoff quality math (`reach_ms`, average tracking error to commanded speed, `speed_fdbk_hz`/`fg_speed_fdbk_hz` mismatch, peak overspeed ratio, unstable/missing telemetry penalties) rather than reach-time-only bias.
    - Discovery candidates are now mid/high open-loop accel first (`250/500Hz/s` class, with conservative fallback) rather than low-accel-first, based on field data where both 270kV and 750kV motors remained stable at much higher accel.
    - `tune_initial_params` computes a per-candidate monitor timeout from `max_speed_hz`, handoff %, and open-loop accel so high-kV/high-voltage setups are not prematurely failed by the legacy fixed 7s window.
    - `tune_initial_params` now enforces a separate open-loop dwell timeout (heating guard) derived from the same handoff estimate; candidates are failed early if they linger in open-loop too long.
    - After candidate failures, `tune_initial_params` cooldown now blocks retries while `fault_active` or severe-current lockout is still asserted, and periodically pulses `CLR_FLT` until recovery is complete (avoids immediate reattempts after `HW_LOCK_LIMIT` before `LCK_RETRY` settles).
    - If `open_loop_limit_source=ilimit`, `tune_initial_params` logs a warning because open-loop current/heating can be significantly higher than with `ol_ilimit`.
    - `tune_initial_params` handoff plausibility guard evaluates post-handoff feedback against commanded electrical speed (not `speed_ref_open_loop_hz`) to avoid false rejects when open-loop ref lags during transition.
    - The same guard rejects candidates when `speed_fdbk_hz` and `fg_speed_fdbk_hz` diverge too far (`abs(delta) > max(35Hz, 55% of higher value)`) for consecutive samples, which catches buzz/stall handoffs with implausible feedback.
    - Candidate success now also requires consecutive plausible handoff samples (`speed_fdbk_hz` and `fg_speed_fdbk_hz` within a commanded-speed band and mutually consistent); closed-loop state alone is no longer enough.
    - If handoff telemetry reads are repeatedly unavailable during the guard window, the candidate is failed as `handoff telemetry unavailable` instead of being silently accepted.
    - `run_mpet`: kicks off MPET (`CMD+KE+MECH+WRITE_SHADOW`), logs 1Hz MPET runtime diagnostics (`ALGO_STATUS_MPET`, `ALGORITHM_STATE`, speed triplet), and emits a one-shot summary (elapsed time, visited-state bitmask, status bits, active MPET profile) on done/fault/timeout.
    - MPET timeout is configurable via `mpet_timeout_ms` (default 120s); startup log and summary include active timeout/profile values.
  - Experimental speed-command reassertion and 1Hz `Run diag` bring-up logging were removed after tuning; use
    algorithm-state transition logs and fault diagnostics for runtime visibility.
  - On detected active faults, firmware forces speed command to `0%` once per fault episode as a safety guard.
  - Severe current faults (`HW_LOCK_LIMIT`, `LOCK_LIMIT`, `BUS_CURRENT_LIMIT`) engage a speed-command safety
    lockout; non-zero speed commands are rejected until `clear_faults` succeeds and faults are no longer active.
  - If the MCF is power-cycled while ESP stays up, firmware now checks for a default-profile signature
    (`CLOSED_LOOP3.MOTOR_BEMF_CONST=0x00` plus `CLOSED_LOOP4.MAX_SPEED=1200`) and automatically re-runs
    post-comms motor setup/config apply to recover without requiring an ESP reboot.
  - Algorithm/FOC phase uses `ALGORITHM_STATE` (`0x0196`); component now logs state transitions at `INFO` level only
    (init + changes) with `speed_cmd`, duty, volt_mag, and `sys_enable` context to keep bring-up logs readable.
  - `dump_config()` now reads `GD_CONFIG1`/`GD_CONFIG2` and logs:
    - `CSA_GAIN` code and V/V mapping
    - `BASE_CURRENT` code and approximate amps
    - approximate amp values for configured motor current limits
    - configured overrides for `CSA_GAIN`, `BASE_CURRENT`,
      `ALIGN_OR_SLOW_CURRENT_ILIMIT`, and `OL_ILIMIT_CONFIG` when set from YAML
    This makes `% of BASE_CURRENT` tuning visible without external calculator.
- Startup/algorithm numeric `*_code` sensors were removed from YAML exposure; use logs (`motor_config` summary and fault logs) instead.
- `binary_sensor` now only exposes aggregate signals (`fault_active`, `sys_enable`); per-fault bit entities were removed.
- On `MPET_BEMF_FAULT`, component logs one-shot diagnostics (speed command, brake input decode, MPET bits, motor BEMF const)
  plus a startup hint to command >10% then ramp down.
- On `HW_LOCK_LIMIT`, component logs one-shot diagnostics including effective `ILIMIT`, lock current limits, lock modes,
  retry, lock enable bits, and ABN/no-motor thresholds.
- User preference: default troubleshooting guidance should stay within guardrails and should not suggest
  `allow_unsafe_current_limits: true` unless explicitly requested.
- MPET mitigation:
  - `clear_mpet_on_startup` (default `true`) clears `ALGO_DEBUG2` MPET command bits (`MPET_CMD`, `MPET_KE`, `MPET_MECH`,
    `MPET_WRITE_SHADOW`) during post-comms setup.
  - With `clear_mpet_on_startup: true`, non-zero speed commands also defensively clear MPET bits first.
  - When `motor_bemf_const` is applied and `MOTOR_RES`/`MOTOR_IND` are zero, startup config seeds both to `1`.
  - When `motor_bemf_const` is applied and speed-loop PI is zero, startup config seeds `SPD_LOOP_KP=1` and
    `SPD_LOOP_KI=1` to avoid forced MPET coasting faults on first non-zero speed command.
  - `clear_faults` button path also clears MPET bits before pulsing `CLR_FLT`, preventing immediate MPET_BEMF relatch.
  - If `MPET_BEMF_FAULT` is still present immediately after setup, component pulses `CLR_FLT` once automatically.
- Boot/runtime warns that MCx83xx requires ~100us byte-gap timing and recommends `i2c.frequency <= 50kHz` for safer bring-up.
  - Bring-up troubleshooting insight from field logs:
  - Field baseline: this motor/system previously started successfully using `mcf8316d_manual` with the GUI-style
    startup tune flow (`apply_motor_tune`). Treat that as a known-good hardware baseline when diagnosing
    `mcf8329a` startup faults; if `mcf8329a` still trips `HW_LOCK_LIMIT`, prioritize register/config parity and
    startup sequence differences before assuming motor/wiring damage.
  - Sweeping only `motor_bemf_const` (for example `0x52` down to `0x38`) may not change startup behavior when
    faults are `ABN_SPEED`/`MTR_LCK` or `HW_LOCK_LIMIT`; in that case prioritize lock detector tuning (`LOCK1/2/3`
    enables and thresholds), startup mode/direction, and current limit tuning over further BEMF-constant sweeps.
  - If closed-loop stays active with constant non-zero `speed_cmd`/`duty` but `volt_mag` decays and speed feedback is unstable,
    verify `CLOSED_LOOP4.MAX_SPEED`; too-low max speed for a high-kV motor can make the speed loop back off voltage and coast.
  - If startup overshoots hard right after `MOTOR_OPEN_LOOP -> MOTOR_CLOSED_LOOP_ALIGNED`, tune `MOTOR_STARTUP2`
    (`OL_ACC_A1`, `OL_ILIMIT`, `AUTO_HANDOFF_EN`, `OPN_CL_HANDOFF_THR`) before changing steady-state loop gains.
  - For high-kV/high-voltage setups (for example `750kV` on `16S`), fixed 7s initial-tune monitor windows can fail late candidates even when startup behavior is otherwise acceptable; adaptive timeout from `MAX_SPEED`, handoff threshold, and open-loop accel resolves this class of false timeout.
  - Back-voltage/regen stress is most sensitive to stop and current/accel settings: aggressive `brake_mode`
    (especially `active_spin_down`), long `brake_time`, high `phase_current_limit_percent`/`open_loop_ilimit_percent`,
    and aggressive open-loop handoff tuning can all raise VM transients.
  - For the 270kV 5065 test motor at ~27V, `max_speed_hz: 900` produced correct steady-state scaling
    (e.g. 15% command settling around ~135 Hz electrical), but overshoot still originated at open-loop handoff; this
    separates steady-state scaling (`MAX_SPEED`) from transient handoff tuning (`MOTOR_STARTUP2`).
  - MPET can dwell in `MOTOR_MPET_KE_MEASURE (0x0014)` for long periods while spinning and without faults; this is a convergence/measurement stall pattern (not an immediate protection trip). Prioritize MPET-specific parameter path (`MPET_KE_MEAS_PARAMETER_SELECT` and MPET open-loop speed/current/slew fields) and `ALGO_STATUS_MPET` visibility before moving to hardware scope debug.
