# HUSB238 active invariants

- Include `esphome/core/hal.h` in `husb238.cpp` when using timing helpers.
- Do not issue PD renegotiation from `setup()`; wait for attachment and the startup grace period.
- Service code uses `registers::RegisterId` and `registers::CommandId`; numeric addresses/codes stop at `RegisterBus`.
- The six source PDO registers are explicit typed IDs. Do not recover base-address arithmetic in service code.
