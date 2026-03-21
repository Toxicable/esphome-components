# Next Session Plan: MPET Completion + Tuning Workflow

## Context Snapshot
- Motor: 5065, 270kV, 12-pole (6 pole-pairs).
- `tune_initial_params` reached stable closed-loop and produced usable startup parameters.
- `run_mpet` repeatedly timed out while spinning in `MOTOR_MPET_KE_MEASURE` (state `0x0014`) for a long dwell, without controller/gate faults.

## Decision For Next Session
- Start with firmware/config diagnostics and MPET parameter controls first.
- Do scope captures only if MPET still cannot complete after software-side MPET controls/status visibility are added.

## Execution Plan (Next Session)

### 1) Add MPET-specific controls (first priority)
1. Add YAML keys for MPET-specific parameter path:
- `mpet_use_dedicated_params` (maps to `INT_ALGO_2.MPET_KE_MEAS_PARAMETER_SELECT`)
- `mpet_open_loop_curr_ref_percent` (maps to `INT_ALGO_1.MPET_OPEN_LOOP_CURR_REF`)
- `mpet_open_loop_speed_ref_percent` (maps to `INT_ALGO_1.MPET_OPEN_LOOP_SPEED_REF`)
- `mpet_open_loop_slew_hz_per_s` (maps to `INT_ALGO_1.MPET_OPEN_LOOP_SLEW_RATE`)
- `mpet_timeout_ms` (runtime timeout override)
2. Keep safe defaults for first pass:
- dedicated MPET params enabled
- speed ref moderate/high (35-50% bucket)
- current ref moderate (40-60% bucket)
- slew moderate (3-10 Hz/s bucket)

### 2) Add MPET status observability
1. Read and log `ALGO_STATUS_MPET (0xE8)` during MPET at 1 Hz:
- `MPET_KE_STATUS`
- `MPET_MECH_STATUS`
- `MPET_PWM_FREQ`
2. Include `ALGORITHM_STATE` and key speed telemetry in same diagnostic line:
- `speed_ref_open_loop_hz`, `speed_fdbk_hz`, `fg_speed_fdbk_hz`
3. Add a one-shot summary on completion/failure/timeout that prints:
- states visited
- final status bits
- active MPET parameter profile

### 3) Improve timeout behavior
1. Keep timeout, but make it configurable (`mpet_timeout_ms`) and log absolute elapsed time.
2. Add an explicit `abort_mpet` button to stop MPET cleanly without waiting for timeout.
3. On timeout, emit exact suggested next adjustment (for example: increase MPET open-loop current bucket by one step).

### 4) Re-test flow
1. Run `tune_initial_params`.
2. Apply recommended startup config.
3. Run MPET with dedicated params and status logging.
4. Collect logs covering:
- MPET start
- KE dwell
- completion or timeout summary.

### 5) Scope fallback (only if still stalled)
1. Capture VM bus behavior through KE dwell.
2. Capture phase voltage waveform.
3. Capture current/shunt/CSA output if available.
4. Correlate scope timestamps with MPET state/status logs.

## Acceptance Criteria
- MPET reaches `MOTOR_MPET_DONE (0x0017)` and logs result values.
- Or, if timeout still occurs, logs must clearly show why (status stuck pattern + parameter profile + elapsed timing), giving deterministic next adjustment steps.

---

# Session Retrospective: Improvement Opportunities

## A) LLM Setup / Workflow Improvements
1. Add a strict edit guardrail in workflow:
- after any large `apply_patch`, immediately run check scripts before making additional edits.
2. Prefer smaller, atomic patches:
- reduces risk of accidental duplicate class blocks or stale fragments.
3. Add a local "compile-risk checklist" for C++ refactors:
- ownership/incomplete-type review (`forward declare` + destructor placement)
- header/API compatibility
- state machine transition sanity check.
4. Keep one running "active assumptions" note:
- avoids silent drift when configuration semantics change mid-session.
5. Add a log-ingestion helper script (future):
- parse ESPHome logs into state/fault/timing timeline automatically to reduce manual interpretation errors.

## B) Code / Architecture Improvements
1. Split tuning/MPET controller into dedicated files:
- `mcf8329a_tuning_controller.h/.cpp`
- keep `mcf8329a.cpp` focused on transport + core component lifecycle.
2. Introduce explicit typed config groups in code:
- hardware baseline
- startup tuning
- optimization/performance
- MPET profile.
3. Add register snapshot structs:
- one struct for startup/handoff params
- one for MPET params/status
- simplifies logging and regression checks.
4. Add state-machine transition instrumentation:
- count retries, candidate attempt durations, failure reasons.
5. Consider optional persistence of "last known good tuning profile" for quick recovery after resets.

## C) Algorithm / Tuning Strategy Improvements
1. Keep two phases separate by design:
- "Reach closed-loop reliably" (startup)
- "Optimize performance" (post-MPET)
2. For optimization scoring, use measurable metrics:
- time-to-closed-loop
- fault-free dwell time
- speed error/stability
- preference bonus for `auto_handoff_enable=true` only when reliability is preserved.
3. Add an optional "validation sweep" button:
- verifies recommended config at multiple commands (`12% -> 16% -> 20%`) and reports pass/fail per step.
4. Add MPET-aware recommendations:
- if stuck in KE without faults, recommend MPET parameter bucket changes instead of generic fault advice.

## D) Documentation Improvements
1. Document layer model explicitly in README:
- Hardware baseline (required before tuning)
- Tuning config (closed-loop entry)
- Optimization config (post-MPET).
2. Add an MPET troubleshooting matrix:
- state stuck vs likely cause vs next parameter change.
3. Add a "minimum data to share for debugging" section:
- config block
- state transitions
- MPET status bits
- timeout line.
