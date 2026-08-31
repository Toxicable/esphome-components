# zephyr_mcumgr context

Read in this order:
1. `AGENTS_KNOWLEDGE.md` — reason the override exists and its compatibility boundary.
2. `ota/__init__.py` — ESPHome codegen/partition overlay; the project-specific fix is here.
3. `ota/ota_zephyr_mcumgr.h` and `ota/ota_zephyr_mcumgr.cpp` — upstream 2026.8.0 OTA implementation, intentionally unmodified.

Upstream baseline: `esphome/esphome` tag `2026.8.0`, `esphome/components/zephyr_mcumgr`.
