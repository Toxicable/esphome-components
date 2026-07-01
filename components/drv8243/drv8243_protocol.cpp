#include "drv8243_protocol.h"

#include <cmath>

namespace drv8243_core {

const char *handshake_result_to_string(HandshakeResult result) {
  switch (result) {
    case HandshakeResult::NOT_RUN:
      return "not_run";
    case HandshakeResult::SUCCESS:
      return "success";
    case HandshakeResult::FAILED:
      return "failed";
  }
  return "unknown";
}

float shaped_output_level(float state, float min_level, float exponent) {
  if (state <= 0.0005f)
    return 0.0f;

  float x = state;
  if (x < 0.0f)
    x = 0.0f;
  if (x > 1.0f)
    x = 1.0f;

  float y = 0.0f;
  if (exponent <= 0.0f) {
    y = min_level + (1.0f - min_level) * x;
  } else {
    y = min_level + (1.0f - min_level) * powf(x, exponent);
  }

  if (y < 0.0f)
    y = 0.0f;
  if (y > 1.0f)
    y = 1.0f;
  return y;
}

}  // namespace drv8243_core
