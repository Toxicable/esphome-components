# MCF8329A Context Index

Quick file map for low-context edits in `components/mcf8329a`.

## Read Order
1. `AGENTS_KNOWLEDGE.md` (active invariants only)
2. `CONTEXT_INDEX.md` (this file)
3. Only the target file(s) below for your change

## Edit Map
- YAML schema and validation:
  `__init__.py`
- YAML -> C++ wiring (`to_code` and entities):
  `__init__.py`
- Main runtime/component orchestration:
  `mcf8329a.cpp`, `mcf8329a.h`
- Motor config register application path:
  `mcf8329a.cpp` (`apply_motor_config_`)
- I2C/register transport + low-level helpers:
  `mcf8329a_client.cpp`, `mcf8329a_client.h`
- Tuning state machine and MPET flow:
  `mcf8329a_tuning.cpp`, `mcf8329a_tuning.h`
- Shared lookup/decode tables used by runtime+tuning:
  `mcf8329a_tables.h`
- User-facing docs/config example:
  `README.md`

## Historical Deep Context
- Field chronology, detailed troubleshooting notes, superseded experiments:
  `notes/historical-notes.md`
- Session TODO snapshot:
  `next-session-plan.md`

## Datasheet Sources
- Canonical PDF: `datasheets/mcf8329a.pdf`
- Low-context derived artifacts (generated from the PDF; extracted text is not stored):
  - `datasheets/*.compact.txt` (boilerplate-stripped searchable text)
  - `datasheets/*.index.md` (section/register/token index with canonical line refs)
  - `datasheets/*.compact.map.tsv` (compact-line -> canonical-line mapping)
- Supplemental docs: `datasheets/i2c_guide.*`, `datasheets/tuning-guide.*`, `datasheets/pre-startup-tuning-guide.*`, `datasheets/open-loop-to-closed-loop-handoff-guide.*`, `datasheets/sllu374.*`
