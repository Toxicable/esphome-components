#!/usr/bin/env python3
"""Host-check the actual FDC1004 power-cycle API and XIAO I2C PM adapter."""

from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
STUBS = {
    "esphome/core/defines.h": "#pragma once\n#define USE_I2C\n",
    "esphome/core/component.h": """
#pragma once
namespace esphome {
class PollingComponent {
 public:
  virtual ~PollingComponent() = default;
  virtual void setup() {} virtual void update() {} virtual void dump_config() {}
  bool is_failed() const { return failed_; }
  void mark_failed() { failed_ = true; }
  void status_set_warning() {} void status_clear_warning() {}
 private: bool failed_ = false;
};
}
""",
    "esphome/core/hal.h": """
#pragma once
#include <cstdint>
inline uint32_t millis() { return 1000; }
""",
    "esphome/core/log.h": """
#pragma once
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGD(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGV(...) ((void)0)
#define ESP_LOGCONFIG(...) ((void)0)
#define LOG_SENSOR(...) ((void)0)
#define LOG_I2C_DEVICE(...) ((void)0)
#define LOG_UPDATE_INTERVAL(...) ((void)0)
""",
    "esphome/components/sensor/sensor.h": """
#pragma once
namespace esphome::sensor {
struct Sensor { float state = 0; void publish_state(float value) { state = value; } };
}
""",
    "esphome/components/button/button.h": """
#pragma once
namespace esphome::button {
class Button { protected: virtual void press_action() = 0; };
}
""",
    "esphome/components/i2c/i2c.h": """
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <vector>
namespace fake {
inline std::array<uint16_t, 256> registers{};
inline std::vector<uint8_t> writes;
inline bool online = true;
inline int failed_read = -1, failed_write = -1;
}
namespace esphome::i2c {
class I2CDevice {
 public:
  bool read_bytes(uint8_t reg, uint8_t *data, size_t size) {
    if (!fake::online || reg == fake::failed_read || size != 2) return false;
    data[0] = fake::registers[reg] >> 8; data[1] = fake::registers[reg];
    return true;
  }
  bool write_bytes(uint8_t reg, const uint8_t *data, size_t size) {
    if (!fake::online || reg == fake::failed_write || size != 2) return false;
    fake::registers[reg] = (uint16_t(data[0]) << 8) | data[1];
    fake::writes.push_back(reg); return true;
  }
};
}
""",
    "zephyr/device.h": """
#pragma once
struct device {};
inline device fake_device;
inline bool fake_device_ready = true;
inline bool device_is_ready(const device *) { return fake_device_ready; }
#define DT_NODELABEL(name) name
#define DEVICE_DT_GET(name) (&fake_device)
""",
    "zephyr/pm/device.h": """
#pragma once
#include <vector>
#include <zephyr/device.h>
enum pm_device_action { PM_DEVICE_ACTION_SUSPEND, PM_DEVICE_ACTION_RESUME };
inline std::vector<pm_device_action> fake_pm_actions;
inline int fake_pm_result = 0;
inline int pm_device_action_run(const device *, pm_device_action action) {
  fake_pm_actions.push_back(action); return fake_pm_result;
}
""",
}

TEST = """
#include <cassert>
#include <cmath>
#include <cerrno>
#include "components/fdc1004/fdc1004.h"
#include "components/xiao_openthread_sed/i2c_power.h"
#include <zephyr/pm/device.h>
using esphome::fdc1004::FDC1004Component;

void reset_chip() {
  fake::registers.fill(0);
  fake::registers[0xFE] = 0x5449;
  fake::registers[0xFF] = 0x1004;
  fake::writes.clear();
}

int main() {
  FDC1004Component converter;
  esphome::sensor::Sensor capacitance;
  converter.set_channel_sensor(0, &capacitance);
  converter.set_channel_capdac(0, 4);
  for (int cycle = 0; cycle < 3; ++cycle) {
    reset_chip();
    assert(converter.reinitialize());
    assert(fake::registers[0x08] == 0x1080); // CIN1, CAPDAC 4
    assert(fake::registers[0x0C] == 0x0580); // 100 S/s, repeat, MEAS1
    assert(!converter.read_measurements()); // no completed conversion
    fake::registers[0x0C] |= 8;
    fake::registers[0x00] = 0x1000; // 2 pF plus 12.5 pF CAPDAC
    assert(converter.read_measurements());
    assert(std::fabs(capacitance.state - 14.5f) < 0.0001f);
  }
  fake::online = false;
  assert(!converter.reinitialize());
  converter.invalidate_measurements();
  assert(std::isnan(capacitance.state));
  fake::online = true;
  reset_chip();
  fake::failed_write = 0x08;
  assert(!converter.reinitialize());
  fake::failed_write = -1;
  assert(converter.reinitialize());
  fake::registers[0x0C] |= 8;
  fake::failed_read = 1;
  assert(!converter.read_measurements());
  converter.invalidate_measurements();
  assert(std::isnan(capacitance.state));

  using namespace esphome::xiao_openthread_sed;
  assert(suspend_i2c());
  assert(resume_i2c());
  assert(fake_pm_actions.size() == 2);
  assert(fake_pm_actions[0] == PM_DEVICE_ACTION_SUSPEND);
  assert(fake_pm_actions[1] == PM_DEVICE_ACTION_RESUME);
  fake_pm_result = -EALREADY;
  assert(suspend_i2c());
  assert(resume_i2c());
  fake_pm_result = -EIO;
  assert(!suspend_i2c());
  assert(!resume_i2c());
  const auto attempts = fake_pm_actions.size();
  fake_device_ready = false;
  assert(!suspend_i2c());
  assert(fake_pm_actions.size() == attempts);
}
"""


def main():
    with tempfile.TemporaryDirectory(prefix="sensor-power-") as directory:
        temp = Path(directory)
        for relative, contents in STUBS.items():
            path = temp / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(contents)
        (temp / "test.cpp").write_text(TEST)
        binary = temp / "test"
        subprocess.run([
            os.environ.get("CXX", "g++"), "-std=c++20", "-I", str(temp),
            "-I", str(ROOT), str(temp / "test.cpp"),
            str(ROOT / "components/fdc1004/fdc1004.cpp"),
            str(ROOT / "components/xiao_openthread_sed/i2c_power.cpp"),
            "-o", str(binary),
        ], check=True)
        subprocess.run([str(binary)], check=True)
    print("sensor power: repeated resets, configuration, fresh reads, errors and PM transitions passed")


if __name__ == "__main__":
    main()
