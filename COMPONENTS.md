# Component Inventory

This file records the maintenance state of every component in this repository. It is an engineering inventory, not a promise of API stability or production readiness.

Update this table when a component is added, adopted by a real device, superseded, or removed.

## Status definitions

- **active** — used by a current board, project, or device configuration and expected to receive fixes.
- **internal** — implementation support loaded by other components; it has no user-facing YAML block of its own.
- **experimental** — retained for evaluation or hardware bring-up, but not yet relied on by a deployed configuration.
- **legacy** — still used or contains functionality that has not yet been migrated, but new work should prefer its replacement.
- **candidate-removal** — no current consumer was found during the repository review. Confirm local, private, old-branch, and unindexed configurations before deleting it.
- **deprecated** — a replacement has been selected and consumers should migrate. Removal still requires a deliberate migration change.

Status describes maintenance intent only. It does not imply that an active component is complete, fully tested, or suitable for safety-critical use.

## Inventory

| Component | Status | Known use or reason retained | Direction |
| --- | --- | --- | --- |
| `bq25756` | active | Programmable-load charger and boat configuration | Primary charger implementation; refactor configuration and service boundaries without changing YAML unnecessarily. |
| `bq25798` | legacy | Pool-cleaner configuration | Freeze feature development; migrate the consumer to `bq25756`, then remove this monolithic driver. |
| `bq76922` | experimental | No deployed configuration or compile fixture found | Require an owner, real board YAML, and tests before promotion; otherwise retire instead of maintaining parallel BQ769x2 implementations. |
| `bq76952` | active | Boat/BMS configurations | Primary BMS implementation. Preserve complete desired state, typed connection/operating/fault snapshots, and the explicit manufacturing boundary. |
| `bq769x0` | candidate-removal | No current consumer found | Confirm external use before removal. |
| `component_common` | internal | Header-only register, charger, and connection-state contracts used by active components | Keep small, host-independent, allocation-free, and policy-free. |
| `drv8243` | active | Train-controller configuration | Keep small and device-focused; do not force a heavyweight architecture onto it. |
| `esc_higher` | active | ESC Higher board and boat configuration | Maintain as the board-facing ESC integration. |
| `fdc1004` | candidate-removal | No current consumer found | Confirm external use before removal. |
| `husb238` | active | Programmable-load configurations | Maintain as the USB-PD sink integration. |
| `l04xmtw` | active | Pool-cleaner configuration | Maintain while the current sensor hardware remains in use. |
| `lps25hb` | experimental | Present in current hardware work, but no deployed YAML consumer was found | Keep lightweight; promote when a real configuration is committed. |
| `makita_xgt` | active | Project and hardware references | Maintain while the battery-interface project remains current. |
| `mcf8316d` | active | ESC Low hardware | Public MCF8316D integration; keep chip registers, tuning and YAML here while using `mcf83xx_common` for family mechanics. |
| `mcf8329a` | active | Boat and pool-cleaner configurations | Public MCF8329A integration; keep chip registers, tuning and YAML here while using `mcf83xx_common` for family mechanics. |
| `mcf83xx_common` | internal | Shared by active MCF8316D and MCF8329A components | Keep limited to the register-bus contract, family framing, read-modify-write and pulse mechanics. |
| `mcp4726` | active | Programmable-load configuration | Maintain as the DAC integration used by the load controller. |
| `mlx90614` | legacy | Local implementation provides dual-zone/object-2 behavior not yet accounted for in the upstream migration | Migrate the required behavior upstream or consciously drop it, then remove the local shadow. |
| `programmable_load` | active | Programmable-load board/project | Typed orchestrator with exclusive manual/procedure ownership, hard hardware limits, aggregate faults and explicit calibration diagnostics/actions. |
| `web_dial` | candidate-removal | No current consumer found | Confirm external use before removal. |

## Removal checklist

A component may move from `candidate-removal` to deletion only after all of the following are checked:

1. Search the default branches of `toxic-boards` and other known consumer repositories.
2. Check local YAML files, private repositories, old branches, and uncommitted hardware bring-up configurations where practical.
3. Check whether the component contains behavior not available in the proposed replacement.
4. Document any required consumer migration in the removal pull request.
5. Remove or update examples, test configurations, documentation, and external-component allowlists at the same time.

Repository code search is evidence, not proof, that a component is unused.

## Adding a component

New components should be entered here in the same pull request that introduces them. Start as `experimental` unless a committed consumer configuration and compile coverage are included. Record the intended hardware or project so future cleanup does not depend on guesswork.
