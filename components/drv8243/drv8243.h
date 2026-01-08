#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace drv8243 {

class DRV8243Output : public Component, public output::FloatOutput {
 public:
  // ----- Injected by codegen -----
  void set_nsleep_pin(GPIOPin *pin) { nsleep_pin_ = pin; }
  void set_nfault_pin(GPIOPin *pin) { nfault_pin_ = pin; }
  void set_out2_pin(GPIOPin *pin) { out2_pin_ = pin; }
  void set_out1_output(output::FloatOutput *out) { out1_output_ = out; }

  void set_min_level(float v) { min_level_ = v; }
  void set_exponent(float e) { exponent_ = e; }

  // Config semantics (kept for compatibility)
  // This sets the default polarity and also updates the runtime polarity.
  void set_flip_polarity(bool flip);
  bool get_flip_polarity() const { return flip_polarity_; }

  // Runtime polarity control (for HA buttons)
  void set_polarity_level(bool level);
  bool get_polarity_level() const { return polarity_level_; }
  void toggle_polarity();

  // ESPHome lifecycle
  void setup() override;
  void dump_config() override;
  void write_state(float state) override;

 protected:
  enum class HandshakeResult : uint8_t { NOT_RUN = 0, VERIFIED_OK, VERIFIED_FAIL, UNVERIFIED };

  HandshakeResult do_handshake_();
  const char *handshake_result_str_(HandshakeResult r) const;

  GPIOPin *nsleep_pin_{nullptr};
  GPIOPin *nfault_pin_{nullptr};
  GPIOPin *out2_pin_{nullptr};                 // polarity pin
  output::FloatOutput *out1_output_{nullptr};  // PWM mapping

  float min_level_{0.014f};
  float exponent_{1.8f};

  bool flip_polarity_{false};    // configured default
  bool polarity_level_{false};   // runtime actual OUT2 level

  bool handshake_ran_{false};
  HandshakeResult handshake_result_{HandshakeResult::NOT_RUN};
};

}  // namespace drv8243
}  // namespace esphome
