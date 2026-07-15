# component_common Context Index

## Read order

1. `ARCHITECTURE.md`
2. `components/component_common/AGENTS_KNOWLEDGE.md`
3. `components/component_common/README.md`
4. the helper header being changed
5. `tests/component_common_test.cpp`

## Edit map

- `bit_field.h`: generic contiguous field and masked-bit operations.
- `byte_order.h`: unsigned fixed-width endian load/store.
- `charger.h`: typed charger capabilities, snapshots, states, and enable command.
- `status.h`: generic lifecycle and typed fault-snapshot contract.
- `README.md`: ESPHome loading, allowlist, and include-path contract.
- `tests/component_common_test.cpp`: host-side helper behaviour and compile-time checks.
