# AGENTS_KNOWLEDGE: component_common

- This is an internal ESPHome component package with no `CONFIG_SCHEMA`; users must not add a top-level `component_common:` block.
- Consumers load it with `AUTO_LOAD = ["component_common"]` and explicit external-component allowlists must also list `component_common`.
- Keep the package header-only unless a shared implementation genuinely needs a `.cpp`; ESPHome source discovery only collects files directly in the package directory.
- Helpers must remain C++17, host-independent, allocation-free, and free of ESPHome headers, logging, entities, chip policy, and register addresses.
- Host-independent consumers use sibling-relative includes so the same core builds from the repository and from ESPHome's generated source tree.
