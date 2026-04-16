#pragma once

#include <string>

#include "esphome/components/number/number.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/core/component.h"

namespace esphome {
namespace web_dial {

class WebDialComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_web_server_base(web_server_base::WebServerBase *web_server_base) { this->web_server_base_ = web_server_base; }
  void set_target_number(number::Number *target_number) { this->target_number_ = target_number; }
  void set_path(const std::string &path) { this->path_ = path; }
  void set_min_value(float min_value) { this->min_value_ = min_value; }
  void set_max_value(float max_value) { this->max_value_ = max_value; }
  void set_step(float step) { this->step_ = step; }
  void set_initial_value(float initial_value) { this->current_value_ = initial_value; }

 protected:
  float clamp_value_(float value) const;
  std::string normalized_path_() const;
  std::string render_html_() const;
  void apply_value_(float value);

  web_server_base::WebServerBase *web_server_base_{nullptr};
  number::Number *target_number_{nullptr};
  std::string path_{"/dial"};
  float min_value_{0.0f};
  float max_value_{100.0f};
  float step_{1.0f};
  float current_value_{50.0f};
};

}  // namespace web_dial
}  // namespace esphome
