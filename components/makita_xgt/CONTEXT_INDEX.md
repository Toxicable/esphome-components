# CONTEXT_INDEX: makita_xgt

Read this component in the following order:

1. [README.md](README.md) for wiring, UART settings, and exposed entities.
2. [__init__.py](__init__.py) for YAML schema and entity surface.
3. [makita_xgt.h](makita_xgt.h) for class layout and command definitions.
4. [makita_xgt.cpp](makita_xgt.cpp) for UART transaction flow, bit reversal, CRC handling, and publish logic.

Edit map:

- Update YAML keys or entity definitions in `__init__.py`.
- Update protocol handling, decoding, or button behavior in `makita_xgt.cpp`.
- Update sensor/button members or commands in `makita_xgt.h`.
- Update user-facing setup guidance and safety notes in `README.md`.
