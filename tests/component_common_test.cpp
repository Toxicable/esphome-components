#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>

#include "components/component_common/bit_field.h"
#include "components/component_common/byte_order.h"
#include "components/component_common/charger.h"
#include "components/component_common/status.h"

namespace {

using Field = component_common::RegisterField<uint8_t, 0x70>;

static_assert(Field::SHIFT == 4);
static_assert(Field::VALUE_MASK == 0x07);
static_assert(Field::fits(0x07));
static_assert(!Field::fits(0x08));
static_assert(Field::decode(0x5A) == 0x05);
static_assert(Field::encode(0x03) == 0x30);
static_assert(Field::replace(0x8F, 0x04) == 0xCF);
static_assert(component_common::replace_masked<uint8_t>(0xAA, 0x0F, 0x05) == 0xA5);
static_assert(component_common::any_set<uint8_t>(0x40, 0x60));
static_assert(!component_common::any_set<uint8_t>(0x10, 0x60));

class FakeCharger final : public component_common::ChargerInterface {
 public:
  component_common::ChargerCapabilities capabilities() const override {
    return {true, true, true, true, true, true};
  }

  component_common::ChargerSnapshot snapshot() const override {
    return snapshot_;
  }

  bool request_enabled(bool enabled) override {
    requested_enabled_ = enabled;
    return true;
  }

  component_common::ChargerSnapshot snapshot_{};
  bool requested_enabled_{false};
};

void test_charger_interface() {
  FakeCharger charger;
  charger.snapshot_.state = component_common::ChargerState::FAST_CC;
  charger.snapshot_.valid = true;

  assert(charger.capabilities().enable_control);
  assert(charger.snapshot().valid);
  assert(component_common::charger_state_to_string(charger.snapshot().state) ==
         std::string_view("fast_cc"));
  assert(charger.request_enabled(true));
  assert(charger.requested_enabled_);
}

void test_status_contract() {
  using Snapshot = component_common::FaultSnapshot<uint8_t>;
  const Snapshot snapshot{2, 0x03, 0x01};
  assert(snapshot.primary == 2);
  assert(snapshot.active_flags == 0x03);
  assert(snapshot.latched_flags == 0x01);
  assert(component_common::lifecycle_state_to_string(
             component_common::LifecycleState::READY) ==
         std::string_view("ready"));
}

void test_byte_order() {
  const std::array<uint8_t, 4> little{{0x78, 0x56, 0x34, 0x12}};
  const std::array<uint8_t, 4> big{{0x12, 0x34, 0x56, 0x78}};

  assert(component_common::load_le<uint16_t>(little.data()) == 0x5678U);
  assert(component_common::load_le<uint32_t>(little.data()) == 0x12345678UL);
  assert(component_common::load_be<uint16_t>(big.data()) == 0x1234U);
  assert(component_common::load_be<uint32_t>(big.data()) == 0x12345678UL);

  std::array<uint8_t, 4> encoded{};
  component_common::store_le<uint32_t>(0x12345678UL, encoded.data());
  assert(encoded == little);

  component_common::store_be<uint32_t>(0x12345678UL, encoded.data());
  assert(encoded == big);
}

}  // namespace

int main() {
  test_byte_order();
  test_charger_interface();
  test_status_contract();
  return 0;
}
