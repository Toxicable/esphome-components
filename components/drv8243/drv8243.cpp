#include "drv8243.h"

#include <cmath>

#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/components/ledc/ledc_output.h"

namespace esphome
{
    namespace drv8243
    {

        static const char *const TAG = "drv8243";

        // Handshake timings
        static constexpr uint32_t SLEEP_FORCE_MS = 2;
        static constexpr uint32_t READY_WAIT_TIMEOUT_US = 5000;
        static constexpr uint32_t ACK_WAIT_TIMEOUT_US = 5000;
        static constexpr uint32_t POLL_STEP_US = 10;
        // ACK pulse: keep close to lower edge to reduce risk of stretching above ~40us
        static constexpr uint32_t ACK_PULSE_US = 22;

        const char *DRV8243Output::handshake_result_str_(HandshakeResult r) const
        {
            switch (r)
            {
            case HandshakeResult::NOT_RUN:
                return "not_run";
            case HandshakeResult::VERIFIED_OK:
                return "verified_ok";
            case HandshakeResult::VERIFIED_FAIL:
                return "verified_fail";
            case HandshakeResult::UNVERIFIED:
                return "unverified";
            default:
                return "unknown";
            }
        }

        void DRV8243Output::dump_config()
        {
            ESP_LOGCONFIG(TAG, "DRV8243 Output");
            const bool two_channel = out2_output_ != nullptr;
            ESP_LOGCONFIG(TAG, "  Mode: %s", two_channel ? "2-channel (ch1 + ch2 PWM)" : "1-channel (ch1 PWM)");
            ESP_LOGCONFIG(TAG, "  Channel 1 (PWM): %s", out1_output_ ? "configured" : "NOT SET");
            ESP_LOGCONFIG(TAG, "  Channel 2 (PWM): %s", two_channel ? "configured" : "not configured");
            char pin_summary[64];
            if (nsleep_pin_)
            {
                nsleep_pin_->dump_summary(pin_summary, sizeof(pin_summary));
                ESP_LOGCONFIG(TAG, "  nSLEEP pin: %s", pin_summary);
            }
            else
            {
                ESP_LOGCONFIG(TAG, "  nSLEEP pin: NOT SET");
            }

            if (nfault_pin_)
            {
                nfault_pin_->dump_summary(pin_summary, sizeof(pin_summary));
                ESP_LOGCONFIG(TAG, "  nFAULT pin: %s", pin_summary);
            }
            else
            {
                ESP_LOGCONFIG(TAG, "  nFAULT pin: NOT SET");
            }

            if (out2_pin_ != nullptr)
            {
                out2_pin_->dump_summary(pin_summary, sizeof(pin_summary));
                ESP_LOGCONFIG(TAG, "  Polarity pin: %s (flip=%s)", pin_summary,
                              flip_polarity_ ? "HIGH" : "LOW");
            }
            else if (two_channel)
            {
                ESP_LOGCONFIG(TAG, "  Polarity pin: not used (ch2 PWM configured)");
            }
            else
            {
                ESP_LOGCONFIG(TAG, "  Polarity pin: NOT SET (external control)");
            }

            ESP_LOGCONFIG(TAG, "  Handshake: %s", handshake_result_str_(handshake_result_));
        }

        void DRV8243Output::setup()
        {
            // Keep setup very light (no delays/pulses here)
            if (nsleep_pin_)
            {
                nsleep_pin_->setup();
                nsleep_pin_->pin_mode(gpio::FLAG_OUTPUT);
                nsleep_pin_->digital_write(true); // default awake
            }

            if (nfault_pin_)
            {
                nfault_pin_->setup();
                nfault_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
            }

            // OUT2 is optional
            if (out2_pin_)
            {
                out2_pin_->setup();
                out2_pin_->pin_mode(gpio::FLAG_OUTPUT);
                out2_pin_->digital_write(flip_polarity_);
            }
        }

        DRV8243Output::HandshakeResult DRV8243Output::do_handshake_()
        {
            if (!nsleep_pin_)
                return HandshakeResult::VERIFIED_FAIL;

            if (nfault_pin_)
            {
                const bool nfault_initial = nfault_pin_->digital_read();
                ESP_LOGI(TAG, "nFAULT initial=%s", nfault_initial ? "HIGH" : "LOW");
            }

            // Force sleep then wake
            nsleep_pin_->digital_write(false);
            delay(SLEEP_FORCE_MS);
            nsleep_pin_->digital_write(true);

            if (nfault_pin_)
            {
                const bool nfault_after_wake = nfault_pin_->digital_read();
                ESP_LOGI(TAG, "nFAULT after wake=%s", nfault_after_wake ? "HIGH" : "LOW");
            }

            // Wait for nFAULT LOW if available (device-ready indication)
            bool saw_ready_low = false;
            int checks = 0;
            if (nfault_pin_)
            {
                uint32_t start = micros();
                while ((micros() - start) < READY_WAIT_TIMEOUT_US)
                {
                    ++checks;
                    if (!nfault_pin_->digital_read())
                    { // LOW
                        saw_ready_low = true;
                        break;
                    }
                    delayMicroseconds(POLL_STEP_US);
                }
            }
            if (nfault_pin_)
            {
                const bool nfault_after_poll = nfault_pin_->digital_read();
                ESP_LOGI(TAG, "nFAULT after poll=%s (checks=%d saw_ready_low=%s)",
                         nfault_after_poll ? "HIGH" : "LOW", checks, saw_ready_low ? "true" : "false");
            }

            // ACK pulse
            nsleep_pin_->digital_write(false);
            delayMicroseconds(ACK_PULSE_US);
            nsleep_pin_->digital_write(true);

            if (nfault_pin_)
            {
                const bool nfault_after_ack = nfault_pin_->digital_read();
                ESP_LOGI(TAG, "nFAULT after ACK=%s", nfault_after_ack ? "HIGH" : "LOW");
            }

            if (!nfault_pin_)
            {
                ESP_LOGI(TAG, "do_handshake: UNVERIFIED nfault_pin_=false");
                return HandshakeResult::UNVERIFIED;
            }

            if (!saw_ready_low)
            {
                ESP_LOGI(TAG, "do_handshake: UNVERIFIED saw_ready_low=false");
                return HandshakeResult::UNVERIFIED;
            }

            // Confirm nFAULT HIGH after ACK
            uint32_t start = micros();
            while ((micros() - start) < ACK_WAIT_TIMEOUT_US)
            {
                if (nfault_pin_->digital_read()) // HIGH
                    return HandshakeResult::VERIFIED_OK;
                delayMicroseconds(POLL_STEP_US);
            }

            return HandshakeResult::VERIFIED_FAIL;
        }

        void DRV8243Output::write_state(float state)
        {
            if (!out1_output_)
                return;

            // Run handshake once, first time we're asked to turn on
            if (!handshake_ran_ && state > 0.0005f)
            {
                ESP_LOGI(TAG, "DRV8243 start");
                handshake_result_ = do_handshake_();
                handshake_ran_ = true;

                if (handshake_result_ == HandshakeResult::VERIFIED_OK)
                {
                    ESP_LOGI(TAG, "DRV8243 ready (verified via nFAULT)");
                }
                else if (handshake_result_ == HandshakeResult::UNVERIFIED)
                {
                    ESP_LOGW(TAG, "DRV8243 started (nFAULT not verified)");
                }
                else
                {
                    ESP_LOGE(TAG, "DRV8243 failed to start (check wiring / nSLEEP / nFAULT)");
                }
            }

            // Only drive OUT2 polarity if configured as a GPIO pin.
            if (out2_pin_)
            {
                out2_pin_->digital_write(flip_polarity_);
            }

            if (state <= 0.0005f)
            {
                out1_output_->set_level(0.0f);
                if (out2_output_ != nullptr)
                {
                    out2_output_->set_level(0.0f);
                }
                return;
            }

            float x = state;
            if (x < 0.0f)
                x = 0.0f;
            if (x > 1.0f)
                x = 1.0f;

            float y;
            if (exponent_ <= 0.0f)
            {
                y = min_level_ + (1.0f - min_level_) * x;
            }
            else
            {
                y = min_level_ + (1.0f - min_level_) * powf(x, exponent_);
            }

            if (y < 0.0f)
                y = 0.0f;
            if (y > 1.0f)
                y = 1.0f;

            out1_output_->set_level(y);

            if (out2_output_ != nullptr)
            {
                out2_output_->set_level(y);
            }
        }

    } // namespace drv8243
} // namespace esphome
