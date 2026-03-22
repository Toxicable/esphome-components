# Datasheet Index: `i2c_guide.pdf`

## Generated Artifacts
- Canonical source: `i2c_guide.pdf`
- Compact source: `i2c_guide.compact.txt`
- Compact line map: `i2c_guide.compact.map.tsv`

## Normalization
- Source text is extracted from the PDF in memory during processing.
- Compact output removes only known page boilerplate/footer lines and collapses repeated blank lines.
- Canonical lines are line numbers in the extracted text stream used for indexing, not a stored transcript.
- Canonical lines: 297; compact lines: 282

## Removed Boilerplate Summary
- Removed lines: 15
- copyright: 8
- revision_tag: 4
- website: 3

## Section Headings (Canonical Line Numbers)
- L29: 1 Introduction
- L47: 2 I2C Related Details for MCx83xx Family
- L61: 2.1 TARGET_ID
- L75: 2 How to Program I2C for MCx83xx Device Family SLLA662 – MARCH 2025
- L104: 2.2 CRC_EN
- L129: 2.2.1 CRC Computational Details
- L138: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR
- L142: 3 I2C Secondary Device Feature Supported by MCx83xx Family
- L143: 3.1 Clock Stretching
- L149: 4 Primary Device Read and Write Expected Flow
- L150: 4.1 Read Sequence
- L162: 4 How to Program I2C for MCx83xx Device Family SLLA662 – MARCH 2025
- L217: 4.2 Write Sequence
- L260: 6 How to Program I2C for MCx83xx Device Family SLLA662 – MARCH 2025
- L263: 5 Summary
- L268: 6 References

## Register Offsets (Canonical Line Numbers)
- No register offsets detected with default pattern.

## Quick Token References (Canonical Line Numbers)
- Selection mode: `auto`; profile: `none`; tokens: 5
- `CRC_EN`:
  - L13: 2.2 CRC_EN.............................................................................................................................................................................3
  - L54: OP_R/W CRC_EN DLEN MEM_SEC MEM_PAGE MEM_ADDR
  - L57: packet we send a 24-bit control word followed by (16/32/64 bit) DATA, and 8-bit CRC if CRC_EN is set in
  - L104: 2.2 CRC_EN
  - L105: If CRC_EN = 1, cyclic redundancy check (CRC) gets enabled otherwise CRC is disabled.
- `MEM_ADDR`:
  - L14: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR......................................................................................................................4
  - L54: OP_R/W CRC_EN DLEN MEM_SEC MEM_PAGE MEM_ADDR
  - L138: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR
  - L140: the register to be read is the MEM_ADDR. For example, to read 0x80 location data, MEM_ADDR needs to be
- `MEM_PAGE`:
  - L14: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR......................................................................................................................4
  - L54: OP_R/W CRC_EN DLEN MEM_SEC MEM_PAGE MEM_ADDR
  - L138: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR
  - L139: For the user accessible status and control registers, MEM_SEC = 0x0 and MEM_PAGE = 0x0. The address of
- `MEM_SEC`:
  - L14: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR......................................................................................................................4
  - L54: OP_R/W CRC_EN DLEN MEM_SEC MEM_PAGE MEM_ADDR
  - L138: 2.3 MEM_SEC, MEM_PAGE, and MEM_ADDR
  - L139: For the user accessible status and control registers, MEM_SEC = 0x0 and MEM_PAGE = 0x0. The address of
- `TARGET_ID`:
  - L12: 2.1 TARGET_ID....................................................................................................................................................................... 2
  - L51: TARGET_ID R/W CONTROL_WORD DATA CRC-8
  - L56: Table 2-1 shows that the format consists of 7-bit TARGET_ID, followed by a read write bit (R/W bit). After this
  - L61: 2.1 TARGET_ID
  - L62: TARGET_ID is a 7-bit value representing the target address of the MCx83xx device. The TARGET_ID default
  - L64: 1. TARGET_ID Configuration: For field oriented control (FOC) devices, TARGET_ID can be configured
  - L66: (MCT83xx), TARGET_ID can be configured in PIN_CONFIG2 register by setting the ‘I2C_TARGET_ADDR’.
  - L68: TARGET_ID change does not take effect at runtime and device continues to communicate with the
  - L69: address burned in EEPROM at the time of power-up. After changing the TARGET_ID, the EEPROM
  - L70: needs to be programmed with the updated TARGET_ID and device needs to be power cycled.
  - L71: 2. TARGET_ID Detection: If primary device fails to communicate with default or preprogrammed TARGET_ID,
  - L72: a search can be implemented to find TARGET_ID. Figure 2-1 explains the TARGET_ID detection steps.

Regenerate with: `./tools/datasheet_prepare.py components/mcf8329a/datasheets/i2c_guide.pdf`
