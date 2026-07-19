#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "components/component_common/block_register_info.h"
#include "components/esc_higher/esc_higher_registers.h"
#include "components/husb238/husb238_service.h"
#include "components/lps25hb/lps25hb_registers.h"
#include "components/mcp4726/mcp4726_protocol.h"
#include "components/mlx90614/mlx90614_registers.h"

namespace {

static_assert(husb238_core::registers::register_address(
                  husb238_core::registers::RegisterId::SELECTED_PDO) == 0x08);
static_assert(husb238_core::registers::command_code(
                  husb238_core::registers::CommandId::HARD_RESET) == 0x10);
static_assert(lps25hb_core::registers::register_address(
                  lps25hb_core::registers::RegisterId::TEMP_OUT_H) == 0x2C);
static_assert(mlx90614_core::registers::register_address(
                  mlx90614_core::registers::RegisterId::OBJECT2_TEMPERATURE) == 0x08);
static_assert(esc_higher_core::registers::register_info(
                  esc_higher_core::registers::RegisterId::BRINGUP).read_size == 64);
static_assert(esc_higher_core::registers::command_code(
                  esc_higher_core::registers::CommandId::CONFIG_COMMIT) == 0x13);
static_assert(component_common::payload_size_matches(component_common::VARIABLE_PAYLOAD_SIZE, 37));
static_assert(!component_common::payload_size_matches(16, 15));

constexpr auto MCP_ZERO = mcp4726_core::encode_volatile_write(
    {.vref = 0b11, .power_down = 0, .gain = 0}, 0);
static_assert(MCP_ZERO[0] == 0x58);
static_assert(MCP_ZERO[1] == 0x00);
static_assert(MCP_ZERO[2] == 0x00);
constexpr auto MCP_FULL = mcp4726_core::encode_volatile_write(
    {.vref = 0b10, .power_down = 0b01, .gain = 1}, 4095);
static_assert(MCP_FULL[0] == 0x53);
static_assert(MCP_FULL[1] == 0xFF);
static_assert(MCP_FULL[2] == 0xF0);

class FakeHusbBus final : public husb238_core::RegisterBus {
 public:
  bool read_register(uint8_t reg, uint8_t *value) override {
    reads.push_back(reg);
    if (value == nullptr) return false;
    *value = registers[reg];
    return true;
  }

  bool write_register(uint8_t reg, uint8_t value) override {
    writes.push_back({reg, value});
    registers[reg] = value;
    return true;
  }

  void delay_ms(uint32_t delay) override { delays.push_back(delay); }

  std::array<uint8_t, 256> registers{};
  std::vector<uint8_t> reads;
  std::vector<std::array<uint8_t, 2>> writes;
  std::vector<uint32_t> delays;
};

void test_husb_typed_service_boundary() {
  FakeHusbBus bus;
  husb238_core::HusbService service(&bus);
  assert(service.request_voltage(20));
  assert(bus.writes.size() == 2);
  assert(bus.writes[0][0] == 0x08);
  assert(bus.writes[0][1] == 0xA0);
  assert(bus.writes[1][0] == 0x09);
  assert(bus.writes[1][1] == 0x01);
  assert(bus.delays == std::vector<uint32_t>({5}));
}

}  // namespace

int main() {
  test_husb_typed_service_boundary();
  return 0;
}
