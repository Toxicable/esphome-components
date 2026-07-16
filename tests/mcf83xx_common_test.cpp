#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "components/mcf83xx_common/protocol.h"
#include "components/mcf83xx_common/register_access.h"

namespace {

class FakeBus : public mcf83xx_common::RegisterBus {
 public:
  bool read_register32(uint16_t offset, uint32_t *value) override {
    ++read32_count;
    if (fail_reads || value == nullptr || offset != register_offset) return false;
    *value = register_value;
    return true;
  }

  bool read_register16(uint16_t offset, uint16_t *value) override {
    ++read16_count;
    if (fail_reads || value == nullptr || offset != register_offset) return false;
    *value = static_cast<uint16_t>(register_value);
    return true;
  }

  bool write_register32(uint16_t offset, uint32_t value) override {
    if (fail_writes || offset != register_offset) return false;
    writes.emplace_back(offset, value);
    register_value = value;
    return true;
  }

  void delay_microseconds(uint32_t delay_us) override { delays.push_back(delay_us); }

  uint16_t register_offset{0x0123};
  uint32_t register_value{0};
  bool fail_reads{false};
  bool fail_writes{false};
  int read32_count{0};
  int read16_count{0};
  std::vector<std::pair<uint16_t, uint32_t>> writes;
  std::vector<uint32_t> delays;
};

void test_protocol_frames() {
  using mcf83xx_common::RegisterWidth;

  static_assert(mcf83xx_common::build_control_word(true, 0x00E2, RegisterWidth::BITS_32) == 0x9000E2U);
  static_assert(mcf83xx_common::build_control_word(true, 0x00E2, RegisterWidth::BITS_16) == 0x8000E2U);
  static_assert(mcf83xx_common::build_control_word(false, 0x00E2, RegisterWidth::BITS_32) == 0x1000E2U);

  const auto read32 = mcf83xx_common::make_control_frame(true, 0x00E2, RegisterWidth::BITS_32);
  assert((read32 == std::array<uint8_t, 3>{0x90, 0x00, 0xE2}));

  const auto write32 = mcf83xx_common::make_write32_frame(0x00E2, 0x12345678U);
  assert((write32 == std::array<uint8_t, 7>{0x10, 0x00, 0xE2, 0x78, 0x56, 0x34, 0x12}));

  const uint8_t data16[] = {0x34, 0x12};
  const uint8_t data32[] = {0x78, 0x56, 0x34, 0x12};
  assert(mcf83xx_common::decode_read16(data16) == 0x1234U);
  assert(mcf83xx_common::decode_read32(data32) == 0x12345678U);
}

void test_register_access() {
  FakeBus bus;
  mcf83xx_common::RegisterAccess access(&bus, 100U);

  bus.register_value = 0xA5A50000U;
  uint32_t value32 = 0;
  uint16_t value16 = 0;
  assert(access.read32(bus.register_offset, value32));
  assert(value32 == 0xA5A50000U);
  assert(access.read16(bus.register_offset, value16));
  assert(value16 == 0x0000U);

  assert(access.update_bits32(bus.register_offset, 0x0000FF00U, 0x00005500U));
  assert(bus.register_value == 0xA5A55500U);
  assert(bus.writes.size() == 1U);
  assert((bus.delays == std::vector<uint32_t>{100U}));

  assert(access.update_bits32(bus.register_offset, 0x0000FF00U, 0x00005500U));
  assert(bus.writes.size() == 1U);

  bus.writes.clear();
  bus.delays.clear();
  bus.register_value = 0U;
  assert(access.pulse_bits32(bus.register_offset, 0x4U, 2000U, 3000U));
  assert(bus.writes.size() == 2U);
  assert(bus.writes[0].second == 0x4U);
  assert(bus.writes[1].second == 0U);
  assert((bus.delays == std::vector<uint32_t>{100U, 2000U, 100U, 3000U}));

  mcf83xx_common::RegisterAccess missing(nullptr);
  assert(!missing.available());
  assert(!missing.read32(bus.register_offset, value32));
  assert(!missing.write32(bus.register_offset, 1U));
}

}  // namespace

int main() {
  test_protocol_frames();
  test_register_access();
  return 0;
}
