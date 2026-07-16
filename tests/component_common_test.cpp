#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>

#include "components/component_common/bit_field.h"
#include "components/component_common/byte_order.h"
#include "components/component_common/charger.h"
#include "components/component_common/register_manifest.h"
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

constexpr std::array<component_common::RegisterManifestEntry, 2> VALID_MANIFEST{{
    {"config", 0x10, 1, 0x0F, 0x30, 0, 0x40, 0x80},
    {"status", 0x20, 2, 0, 0, 0x00FF, 0, 0xFF00},
}};
constexpr std::array<component_common::RegisterImageEntry, 1> VALID_IMAGE{{
    {"config", 0x10, 1, 0x05, 0x0F, 0x40},
}};
constexpr std::array<component_common::RegisterManifestEntry, 1> UNCLASSIFIED_MANIFEST{{
    {"broken", 0x10, 1, 0x01, 0, 0, 0, 0},
}};
constexpr std::array<component_common::RegisterManifestEntry, 2> OVERLAPPING_MANIFEST{{
    {"wide", 0x10, 2, 0xFFFF, 0, 0, 0, 0},
    {"overlap", 0x11, 1, 0xFF, 0, 0, 0, 0},
}};

static_assert(component_common::register_manifest_valid(VALID_MANIFEST));
static_assert(component_common::configuration_image_layout_complete(VALID_MANIFEST, VALID_IMAGE));
static_assert(!component_common::register_manifest_valid(UNCLASSIFIED_MANIFEST));
static_assert(!component_common::register_manifest_valid(OVERLAPPING_MANIFEST));
static_assert(component_common::register_value_matches(0xA5, 0x05, 0x0F));
static_assert(component_common::merge_register_value(0xA0, 0x05, 0x0F) == 0xA5);

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
  assert(component_common::connection_state_to_string(
             component_common::ConnectionState::CONNECTING) ==
         std::string_view("connecting"));
  assert(component_common::connection_state_to_string(
             component_common::ConnectionState::CONNECTED) ==
         std::string_view("connected"));
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

void test_configuration_fingerprint() {
  const uint32_t first = component_common::configuration_fingerprint(VALID_IMAGE);
  const uint32_t second = component_common::configuration_fingerprint(VALID_IMAGE);
  assert(first == second);
  assert(first != component_common::FNV1A_OFFSET_BASIS);
}

}  // namespace

int main() {
  test_byte_order();
  test_charger_interface();
  test_status_contract();
  test_configuration_fingerprint();
  return 0;
}
