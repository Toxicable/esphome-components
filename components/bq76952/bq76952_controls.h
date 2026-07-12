#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bq76952 {

class BQ76952Component;

class BQ76952OutputEnabledSwitch : public switch_::Switch, public Parented<BQ76952Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76952AutonomousSwitch : public switch_::Switch, public Parented<BQ76952Component> {
 protected:
  void write_state(bool state) override;
};

class BQ76952ClearAlarmsButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

class BQ76952ProgramFactoryOtpButton : public button::Button, public Parented<BQ76952Component> {
 protected:
  void press_action() override;
};

}  // namespace bq76952
}  // namespace esphome
