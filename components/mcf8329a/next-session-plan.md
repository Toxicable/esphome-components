# Next Session Plan: Validation First, Then MPET

## Updated
- 2026-03-21

## Current Status
- Startup tuning flow has a new cooldown recovery gate after candidate failure:
  - retries now wait for fault/lockout recovery and periodically pulse `CLR_FLT`.
  - this specifically targets `HW_LOCK_LIMIT` cascades where later candidates were attempted too early.
- Idle FG telemetry now suppresses implausible spikes:
  - if command is idle and `fg_speed_fdbk_hz` is clearly impossible while signed `speed_fdbk_hz` is near zero/unavailable, FG is forced to `0Hz`.
- MPET can still stall in `MOTOR_MPET_KE_MEASURE (0x0014)` and remains the next major feature/debug target.

## Session Goals
1. Confirm today’s fault-recovery and FG-idle fixes on hardware.
2. If both are stable, move immediately into MPET observability + control improvements.

## Step 1: Hardware Validation (must pass before new feature work)
1. Flash latest build.
2. With motor idle (`speed_percent: 0`), verify:
   - `fg_speed_fdbk_hz` is not stuck at large bogus values (for example ~10000Hz).
   - `speed_fdbk_hz` and `fg_speed_fdbk_hz` stay near `0Hz` at idle.
3. Run `tune_initial_params` and intentionally exercise failure/retry paths:
   - check for log line `Initial tune waiting for fault recovery before retrying candidate...`
   - ensure next candidate starts only after `Initial tune fault recovery complete...`
4. Capture the full log block from first failure through next retry start.

## Step 2: MPET Improvements (if Step 1 passes)
1. Add MPET config keys:
   - `mpet_use_dedicated_params` -> `INT_ALGO_2.MPET_KE_MEAS_PARAMETER_SELECT`
   - `mpet_open_loop_curr_ref_percent` -> `INT_ALGO_1.MPET_OPEN_LOOP_CURR_REF`
   - `mpet_open_loop_speed_ref_percent` -> `INT_ALGO_1.MPET_OPEN_LOOP_SPEED_REF`
   - `mpet_open_loop_slew_hz_per_s` -> `INT_ALGO_1.MPET_OPEN_LOOP_SLEW_RATE`
   - `mpet_timeout_ms` -> runtime timeout override
2. Add MPET status logging at 1Hz during MPET:
   - `ALGO_STATUS_MPET (0xE8)` decoded fields
   - `ALGORITHM_STATE`
   - `speed_ref_open_loop_hz`, `speed_fdbk_hz`, `fg_speed_fdbk_hz`
3. Add a final MPET summary line on done/fault/timeout:
   - elapsed time
   - last state/status bits
   - active MPET parameter profile

## Step 3: Re-Test Sequence
1. `clear_faults`
2. `tune_initial_params`
3. Apply recommended startup values
4. `run_mpet`
5. Collect logs for:
   - startup tune fault-recovery behavior
   - MPET start, dwell progression, and final result/timeout

## Scope Fallback (only if MPET still stalls after Step 2)
1. VM bus capture through KE dwell window.
2. Phase voltage waveform capture.
3. Current/CSA capture if available.
4. Correlate scope timestamps with firmware MPET status logs.

## Acceptance Criteria
1. Idle telemetry is sane (`fg_speed_fdbk_hz` no longer stuck high at zero command).
2. Tuning retries no longer chain-fail after `HW_LOCK_LIMIT` due to premature restart.
3. MPET either:
   - completes (`MOTOR_MPET_DONE`), or
   - times out with enough structured status data to choose the next parameter change deterministically.
