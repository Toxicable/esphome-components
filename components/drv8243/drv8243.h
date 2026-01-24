#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace drv8243 {

class DRV8243Output : public Component, public output::FloatOutput {
 public:
  void set_nsleep_pin(GPIOPin *pin) { nsleep_pin_ = pin; }
  void set_nfault_pin(GPIOPin *pin) { nfault_pin_ = pin; }

  // Optional when ch2 is used; otherwise OUT2 must be driven or tied.
  void set_out2_pin(GPIOPin *pin) { out2_pin_ = pin; }
  void set_flip_polarity(bool v) { flip_polarity_ = v; }

  void set_out1_output(output::FloatOutput *out) { out1_output_ = out; }
  void set_out2_output(output::FloatOutput *out) { out2_output_ = out; }

  void set_min_level(float v) { min_level_ = v; }
  void set_exponent(float e) { exponent_ = e; }

  void setup() override;
  void dump_config() override;
  void write_state(float state) override;

 protected:
  enum class HandshakeResult : uint8_t { NOT_RUN = 0, VERIFIED_OK, VERIFIED_FAIL, UNVERIFIED };

  HandshakeResult do_handshake_();
  const char *handshake_result_str_(HandshakeResult r) const;

  GPIOPin *nsleep_pin_{nullptr};
  GPIOPin *nfault_pin_{nullptr};
  GPIOPin *out2_pin_{nullptr};                   // Optional polarity pin when using ch2
  output::FloatOutput *out1_output_{nullptr};    // REQUIRED PWM output
  output::FloatOutput *out2_output_{nullptr};    // OPTIONAL second PWM output

  float min_level_{0.014f};
  float exponent_{1.8f};
  bool flip_polarity_{false};   // Only used if out2_pin_ is configured

  bool handshake_ran_{false};
  HandshakeResult handshake_result_{HandshakeResult::NOT_RUN};
};

}  // namespace drv8243
}  // namespace esphome
