#include <array>
#include <cassert>
#include <cstdint>

#include "components/component_common/bit_field.h"
#include "components/component_common/byte_order.h"

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
  return 0;
}
