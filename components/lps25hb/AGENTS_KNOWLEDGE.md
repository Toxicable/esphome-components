# LPS25HB active invariants

- Keep this as a lightweight simple adapter; it does not need protocol/service classes.
- All register access uses `lps25hb_core::registers::RegisterId`; numeric addresses belong only in `lps25hb_registers.h`.
- Measurements use one-shot mode with BDU enabled and wait for both pressure and temperature ready bits.
