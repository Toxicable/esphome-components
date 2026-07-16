#include <cassert>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "components/mcf8316d/mcf8316d_service.h"
#include "components/mcf8329a/mcf8329a_service.h"

namespace {

class FakeBus : public mcf83xx_common::RegisterBus {
 public:
  bool read_register32(uint16_t offset, uint32_t *value) override {
    if (value == nullptr) return false;
    *value = registers[offset];
    return true;
  }

  bool read_register16(uint16_t offset, uint16_t *value) override {
    if (value == nullptr) return false;
    *value = static_cast<uint16_t>(registers[offset]);
    return true;
  }

  bool write_register32(uint16_t offset, uint32_t value) override {
    registers[offset] = value;
    writes.emplace_back(offset, value);
    return true;
  }

  void delay_microseconds(uint32_t delay_us) override { delays.push_back(delay_us); }

  std::map<uint16_t, uint32_t> registers;
  std::vector<std::pair<uint16_t, uint32_t>> writes;
  std::vector<uint32_t> delays;
};

void test_mcf8316d_service() {
  using namespace mcf8316d_core;
  using namespace mcf8316d_core::regs;

  FakeBus bus;
  MCF8316DService service(&bus);

  assert(service.set_brake_input(true));
  assert((bus.registers[REG_PIN_CONFIG] & PIN_CONFIG_BRAKE_INPUT_MASK) == PIN_CONFIG_BRAKE_INPUT_BRAKE);
  assert(bus.delays.empty());

  assert(service.set_direction_input(DirectionInputMode::CCW));
  assert((bus.registers[REG_PERI_CONFIG1] & PERI_CONFIG1_DIR_INPUT_MASK) == PERI_CONFIG1_DIR_INPUT_CCW);

  assert(service.write_speed_command_percent(50.0F));
  const uint32_t speed = bus.registers[REG_ALGO_DEBUG1];
  assert((speed & ALGO_DEBUG1_OVERRIDE_MASK) != 0U);
  assert(((speed & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16U) == 16384U);

  bus.delays.clear();
  assert(service.pulse_clear_faults());
  assert((bus.delays == std::vector<uint32_t>{2000U, 2000U}));
  assert((bus.registers[REG_ALGO_CTRL1] & ALGO_CTRL1_CLR_FLT_MASK) == 0U);
}

void test_mcf8329a_service() {
  using namespace mcf8329a_core;
  using namespace mcf8329a_core::regs;

  FakeBus bus;
  MCF8329AService service(&bus);

  assert(service.set_brake_input(true));
  assert((bus.registers[REG_PIN_CONFIG] & PIN_CONFIG_BRAKE_INPUT_MASK) == PIN_CONFIG_BRAKE_INPUT_BRAKE);
  assert((bus.delays == std::vector<uint32_t>{100U}));

  bus.delays.clear();
  assert(service.set_direction_input(DirectionInputMode::CW));
  assert((bus.registers[REG_PERI_CONFIG1] & PERI_CONFIG1_DIR_INPUT_MASK) == PERI_CONFIG1_DIR_INPUT_CW);
  assert((bus.delays == std::vector<uint32_t>{100U}));

  bus.delays.clear();
  assert(service.write_speed_command_raw(0x1234U));
  assert((bus.registers[REG_ALGO_DEBUG1] & ALGO_DEBUG1_OVERRIDE_MASK) != 0U);
  assert(((bus.registers[REG_ALGO_DEBUG1] & ALGO_DEBUG1_DIGITAL_SPEED_CTRL_MASK) >> 16U) == 0x1234U);
  assert((bus.delays == std::vector<uint32_t>{100U}));

  bus.delays.clear();
  assert(service.write_mpet_results_to_shadow());
  assert((bus.registers[REG_ALGO_DEBUG2] & ALGO_DEBUG2_MPET_WRITE_SHADOW_MASK) == 0U);
  assert((bus.delays == std::vector<uint32_t>{100U, 2000U, 100U}));
}

}  // namespace

int main() {
  test_mcf8316d_service();
  test_mcf8329a_service();
  return 0;
}
