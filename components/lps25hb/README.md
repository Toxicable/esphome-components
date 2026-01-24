# LPS25HB

Datasheet: https://www.st.com/resource/en/datasheet/lps25hb.pdf

## What it does
Reads temperature and barometric pressure from an ST LPS25HB sensor over I²C.

## How to use it
Minimal configuration:

```yaml
sensor:
  - platform: lps25hb
    address: 0x5C  # Optional / default
    temperature:
      name: "LPS25HB Temperature"
    pressure:
      name: "LPS25HB Pressure"
```

Notes:
- Update interval defaults to `60s`.
