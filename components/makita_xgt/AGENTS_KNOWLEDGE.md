# AGENTS_KNOWLEDGE: makita_xgt

Component-scoped notes for `components/makita_xgt`.

- This component is UART-based and mirrors the command/response flow from `twaymouth/XGT-Tester`.
- Required bus settings from the upstream implementation are `9600`, `8E1`, with UART inversion enabled; README should keep that explicit.
- Battery responses arrive bit-reversed, so both short and long frames must be nibble-reversed before CRC and payload decoding.
- Short responses are validated with an 8-bit sum CRC using `0xCC` framing; the model response is a long `0xA5 0xA5` frame with a 16-bit additive CRC.
- `send_command_()` now emits debug logs for wake/TX raw/RX raw/RX decoded/CRC status; keep those logs when adjusting UART protocol handling because they are the primary bring-up aid.
- Some UART setups echo transmitted bytes back into RX; `read_frame_()` must ignore an exact echoed command before treating bytes as a battery response.
- `health` is published using the upstream derived formula `raw_health / (cell_size * parallel_count)`, not a directly reported percentage.
- `factory_reset` is a destructive control and should remain optional and clearly labeled unsafe in docs.
