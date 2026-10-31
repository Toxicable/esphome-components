#pragma once

#include "esphome/core/defines.h"

#ifdef USE_I2C
namespace esphome::xiao_openthread_sed {

// The XIAO's sole ESPHome I2C controller is i2c1. The application must
// exclusively own this bus when switching its sensor supply.
bool suspend_i2c();
bool resume_i2c();

}  // namespace esphome::xiao_openthread_sed
#endif
