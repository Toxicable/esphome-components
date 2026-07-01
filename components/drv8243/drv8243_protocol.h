#pragma once

#include <cstdint>

namespace drv8243_core {

enum class HandshakeResult : uint8_t { NOT_RUN = 0, SUCCESS, FAILED };

const char *handshake_result_to_string(HandshakeResult result);
float shaped_output_level(float state, float min_level, float exponent);

}  // namespace drv8243_core
