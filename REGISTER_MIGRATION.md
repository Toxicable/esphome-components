# Register migration status

Every retained component is classified and enforced by `tools/check_register_models.py`.

## Typed register or command components

- `bq25756`
- `bq76952`
- `esc_higher`
- `husb238`
- `lps25hb`
- `mcf8316d`
- `mcf8329a`
- `mcp4726`
- `mlx90614`

These components keep numeric addresses and command codes inside chip-specific metadata or the lowest transport boundary. Shared helpers provide validation and indexing, not device policy.

## Deliberate non-register components

- `drv8243`: GPIO/PWM handshake driver
- `l04xmtw`: UART stream sensor
- `makita_xgt`: UART command/response protocol
- `programmable_load`: typed orchestrator over other device capabilities

## Internal support components

- `component_common`
- `mcf83xx_common`

Small adapters remain lightweight. A typed register model does not require a service layer when the component has no reconciliation, sequencing or reusable device policy.

The host suite validates metadata coverage. The pinned ESPHome matrix includes every retained component with a committed compile fixture, including HUSB238 and MCP4726.
