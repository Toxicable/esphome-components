#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[2]

protocol_path = root / "components/bq76952/bq76952_protocol.cpp"
protocol = protocol_path.read_text()
old = "  this->crc_enabled_ = true;\n"
new = "  // Probe without CRC first; read_bytes() learns an existing CRC-enabled image.\n  this->crc_enabled_ = false;\n"
if protocol.count(old) != 1:
    raise RuntimeError("expected one CRC initialization")
protocol_path.write_text(protocol.replace(old, new, 1))

service_path = root / "components/bq76952/bq76952_service.cpp"
service = service_path.read_text()
old = """  if ((safety_b & static_cast<uint8_t>(hw::bits::protection_b::UTC | hw::bits::protection_b::UTD | hw::bits::protection_b::OTC |
                                       hw::bits::protection_b::OTD | (1U << 7) | (1U << 6) | (1U << 2))) != 0) {
"""
new = """  if ((safety_b & hw::bits::protection_b::ANY_TEMPERATURE) != 0) {
"""
if service.count(old) != 1:
    raise RuntimeError("expected one raw temperature mask")
service_path.write_text(service.replace(old, new, 1))

(root / ".github/workflows/bq76952-cleanup-once.yml").unlink(missing_ok=True)
Path(__file__).unlink(missing_ok=True)
