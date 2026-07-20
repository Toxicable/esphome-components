# AGENTS_KNOWLEDGE: bq25628

- The register source of truth is `bq25628_registers.h`, which contains the
  complete 36-register Table 8-5 map. Use its typed fields and manifest rather
  than raw addresses, masks, or shifts elsewhere.
- Service APIs take `RegisterId`; only its register-bus adapter resolves an ID
  to an I2C address.
- `component_common` is an internal load dependency because the register manifest
  and typed fields are shared infrastructure; consumers must allowlist it.
- The BQ25628E has a fixed 7-bit I2C address of `0x6A`; setup validates
  `REG0x38.PN[5:3] == 0b100` before enabling the ADC.
- Battery voltage comes from `REG0x30_VBAT_ADC`: bits 12:1 have a 1.99 mV LSB.
- `REG0x26.ADC_EN` is changed through a read-modify-write so the charger-owned
  ADC rate, sample, and averaging settings remain untouched.
