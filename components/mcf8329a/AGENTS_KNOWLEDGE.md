# AGENTS_KNOWLEDGE: mcf8329a

Component-scoped active guidance for `components/mcf8329a`.

## Scope and Context
- Keep this file to current invariants only.
- Historical bring-up logs, experiments, and session-level notes live in `components/mcf8329a/notes/historical-notes.md`.
- Use `components/mcf8329a/CONTEXT_INDEX.md` for quick file targeting before opening large files.
- Datasheet source: `components/mcf8329a/datasheets/mcf8329a.pdf`.
- Datasheet workflow: treat the PDF as canonical and regenerate `*.compact.txt`, `*.index.md`, and `*.compact.map.tsv` with `tools/datasheet_prepare.py` (do not hand-edit generated files).

## Architecture
- Monolith ESPHome integration style: controls/telemetry/buttons are configured inline under `mcf8329a:`.
- Register/bitfield constants are centralized in `mcf8329a_client.h` (`namespace regs`).
- Transport and register helpers live in `mcf8329a_client.cpp/.h`; component register methods delegate to `MCF8329AClient`.
- Tuning logic is isolated in `mcf8329a_tuning.cpp/.h` (`MCF8329ATuningController`); component owns orchestration.
- Shared decode/lookup tables are centralized in `mcf8329a_tables.h`.

## Build Mode
- `to_code()` adds `-DMCF8329A_EMBED_IMPL`.
- In embed mode, `mcf8329a.cpp` includes `mcf8329a_client.cpp` and `mcf8329a_tuning.cpp`.
- Embedded file-scope `static` identifiers must stay uniquely prefixed to avoid redefinition collisions.

## Config and Guardrails
- Required YAML keys: `mode`, `brake_mode`, `motor_bemf_const`, `max_speed_hz`.
- Hardware baseline required before tuning keys: `csa_gain_v_per_v`, `base_current_amps`, `phase_current_limit_percent`, `open_loop_limit_source`, `lock_mode`.
- `allow_unsafe_current_limits` defaults to `false` and gates >50% current-limit settings plus `lock_mode: disabled`.
- Legacy `startup_*` keys are intentionally removed and must raise migration errors.

## Runtime Safety and Behavior
- Non-zero speed commands auto-release brake before writing speed.
- Detected active faults force speed command to `0%` once per fault episode.
- Severe current faults (`HW_LOCK_LIMIT`, `LOCK_LIMIT`, `BUS_CURRENT_LIMIT`) enable a non-zero speed lockout until faults are cleared.
- Startup can auto-recover from detected MCF default-profile reset signature by reapplying post-comms setup.

## Telemetry and Logs
- Algorithm-state transitions are logged at `INFO` (init + changes) using `ALGORITHM_STATE`.
- Runtime emits active-speed diagnostic logs (cmd/ref/fdbk/fg/max-speed/read-valid flags).
- Speed telemetry (`speed_fdbk_hz`, `speed_ref_open_loop_hz`, `fg_speed_fdbk_hz`) publishes direct decoded register values (no brake/idle zero-clamping); failed reads publish `NaN`.
- Startup/algorithm numeric `*_code` sensors and per-fault bit entities are intentionally removed; use logs + aggregate entities.

## Register/Decode Notes
- `ALGORITHM_STATE` offset is `0x0196`.
- `CSA_GAIN_FEEDBACK`/`VOLTAGE_GAIN_FEEDBACK`/`VM_VOLTAGE` are `0x0450`/`0x0458`/`0x045C`.
- `PIN_CONFIG.BRAKE_INPUT` is bits `[3:2]`; `PERI_CONFIG1.DIR_INPUT` is bits `[20:19]`.
- `ALGO_CTRL1.CLR_FLT` is bit `29`; `WATCHDOG_TICKLE` is bit `10`.
- `VM_VOLTAGE` decode uses full 32-bit Q27 scaling (`volts = raw * 60 / 2^27`).

## Docs and README Conventions
- Keep one YAML example with optional keys commented, grouped by purpose.
- Use `##` for section/explanatory comment lines and `#` for commented keys.
- Keep motor conversion math inline in YAML `## Required motor keys` block.

## Troubleshooting Defaults
- Prefer within-guardrail troubleshooting first; do not suggest `allow_unsafe_current_limits: true` unless explicitly requested.
- If startup overshoots after open-loop to closed-loop transition, prioritize `MOTOR_STARTUP2` handoff/current/accel tuning before steady-state PI tuning.
- If speed loop decays despite active closed-loop, verify `CLOSED_LOOP4.MAX_SPEED` scaling first.
