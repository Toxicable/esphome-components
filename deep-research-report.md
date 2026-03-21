# Technical review of the MCF8329A ESC PCB and ESPHome firmware in Toxicable/esphome-components

## Executive summary

The **Toxicable/esphome-components** `mcf8329a` component is not a “motor control algorithm” in the traditional sense: it is an **ESPHome + I²C control/telemetry shim** that writes configuration registers on a **TI MCF8329A** sensorless FOC gate driver and sends a **digital speed reference**. The **FOC, current loop, estimator, open-loop start and the open→closed-loop handoff are executed inside the MCF8329A silicon**, not in the ESP32 firmware. citeturn6search0

That architectural fact strongly shapes what “firmware shortcomings” means here: the main risks are **unsafe/abrupt setpoint changes**, **insufficient guardrails around the chip’s tuning registers**, **insufficient observability (speed/current) during handoff**, and **incomplete exposure of the MCF8329A’s handoff/lock-detection parameters that materially affect HW_LOCK_LIMIT events**.

Your symptoms align with a very common failure mode for sensorless FOC bring-up:

* The motor accelerates under open-loop, but **handoff into closed-loop occurs with estimator misalignment / insufficient BEMF / overly aggressive transition**, which produces a **phase-current spike** and trips a protection such as **HW_LOCK_LIMIT**.
* Separately, you observed **~2 A draw with BASE_CURRENT at the “hardware-default”**, which is consistent with how TI expresses many limits as **% of BASE_CURRENT**; if BASE_CURRENT is correct for your shunt/gain, even “minimum” % limits can still correspond to amp-level currents. The base-current sizing guidance TI provides is effectively tied to **CSA gain, RSENSE, and the ~1.5 V internal scaling**, and then a scaling factor to create the register code. citeturn6search2

The most important concrete improvements to make tuning safe and productive for a **small 2836 ~750 kV motor on 4S** are:

* Add **firmware-side slew-rate limiting / soft-start** for speed commands (and optionally “start-above-10% then ramp down” automation), so you are not testing handoff stability with worst-case step inputs.
* Expose and/or set the **handoff-smoothing controls** called out by TI (notably **THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP, SPD_LOOP_KI**), since TI explicitly describes these as key to smoothing the **handoff current profile**. citeturn6search45turn6search44
* Expose the **lock-current deglitch** parameters (currently identified as a gap in the component’s own knowledge notes) because deglitch can decide whether brief spikes trip HW_LOCK_LIMIT.
* Add **speed feedback telemetry** using the PCB’s **FG pin**, and (if feasible on your board) add **current telemetry** (even if coarse) so you can distinguish “insufficient torque to reach handoff” from “handoff spike trips current limit”.
* Treat I²C robustness as a safety feature: TI explicitly requires **~100 µs inter-byte delay** for reliable comms on this device family; if you can’t implement true byte-gapping on your MCU peripheral, you must reduce effective byte rate (often by lowering I²C clock) or redesign transactions—otherwise you risk “ghost misconfiguration” that looks like tuning failure. citeturn7search24turn7search0

## Evidence base and artefacts reviewed

The review is based on:

* A repo snapshot you provided (**`esphome-components-main.zip`**) containing:
  * `components/mcf8329a/README.md` (usage + a suggested baseline for a 4S / 750 kV / 2836-class motor).
  * `components/mcf8329a/__init__.py` (ESPHome config schema and validation guardrails).
  * `components/mcf8329a/mcf8329a.{h,cpp}` (I²C register access, runtime control, fault logging and startup configuration writes).
  * `components/mcf8329a/mcf8329a.pdf` and extracted `mcf8329a.txt` (device reference material bundled with the component).
* Your uploaded netlist CSV (**`Netlist_ESC_High_2026-03-19.csv`**) showing the ESC power stage and how the MCF8329A is wired (notably: **RSENSE = 1 mΩ**, discrete MOSFET stage, FG/nFAULT/I²C broken out).
* High-quality external sources (TI primary documents and TI product page), especially:
  * **MCF83xx open-loop→closed-loop handoff tuning guidance** (SLLA665). citeturn6search45turn6search44
  * **MCx83xx motor pre-startup tuning guidance** (SLLA663). citeturn8search24
  * **I²C programming requirement** including the **100 µs inter-byte delay** (SLLA662 + TI forum clarification). citeturn7search24turn7search0
  * TI product page for MCF8329A feature set and constraints (supported electrical frequency, protections, FG/DACOUT, etc.). citeturn6search0

## Firmware analysis of `components/mcf8329a`

### What the firmware actually controls

At runtime, the component primarily does three things:

It forces an initial safe-ish state after communications are established by commanding **speed = 0%**, **brake ON**, and setting direction mode (default “hardware”), then applies a set of startup motor configuration registers. (This is done in the “post-comms setup” path.)

It provides control surfaces:
* `speed_percent` writes a **digital speed control word** (0…32767 representing 0–100%) into a debug/control register (override enabled), and also auto-releases brake on any non-zero command.
* `brake` writes BRAKE override bits.
* `direction` writes DIR input selection bits.
* `clear_faults` pulses the CLR_FLT bit and also clears certain MPET-related bits first, to avoid a fault relatch.

It polls telemetry and faults:
* Reads algorithm status and logs **algorithm state transitions**.
* Reads gate-driver and controller fault status, publishes a condensed fault string, and performs a “fault shutdown” (forces speed 0 once per fault episode). For “severe current faults” it latches a **software lockout** that blocks non-zero speed commands until faults are cleared.  
* Reads VM voltage (from an internal scaled register), and reads the measured/estimated motor BEMF constant from `MTR_PARAMS` for telemetry.

### Files and functions relevant to motor control/tuning/safety

This list focuses on **motor control path**, **commutation/handoff relevance**, **current limiting**, and **safety interlocks** (in your requested categories).

**`components/mcf8329a/__init__.py` (ESPHome schema + validation)**
* `validate_safety_guardrails(config)`: blocks (unless overridden) several current-limit related settings above 50%, and blocks disabling lock-mode, aiming to avoid unsafe tuning.
* `encode_max_speed_hz(value_hz)`: encodes electrical Hz into device “MAX_SPEED” code.
* `encode_base_current_amps(value_amps)`: encodes BASE_CURRENT amps into register code via the device’s scaling convention.
* Configuration keys that matter for handoff and HW_LOCK_LIMIT outcomes:
  * `startup_max_speed_hz`
  * `startup_open_loop_ilimit_percent`, `startup_open_loop_accel_hz_per_s`, `startup_open_to_closed_handoff_percent`, `startup_auto_handoff_enable`
  * `phase_current_limit_percent`, `startup_lock_ilimit_percent`, `startup_hw_lock_ilimit_percent`
  * `startup_lock_mode`, lock thresholds/enables

**`components/mcf8329a/mcf8329a.cpp` (control, logging, fault handling)**
* `MCF8329AComponent::apply_post_comms_setup_()`  
  Sets initial speed/brake/direction, clears MPET bits (optional), calls `apply_startup_motor_config_()`.
* `MCF8329AComponent::apply_startup_motor_config_()`  
  The main “tuning write” routine: reads/modifies/writes multiple configuration registers related to:
  * CSA gain and BASE_CURRENT
  * ILIMIT, HW_LOCK_ILIMIT, LOCK_ILIMIT, lock modes, lock enables and thresholds
  * startup mode & alignment time
  * open-loop limits/accel/handoff threshold
  * BEMF constant and MAX_SPEED  
  Then reads back and logs an extensive “Startup motor config” summary (very useful for auditability).
* `MCF8329AComponent::set_speed_percent(...)`  
  Converts 0–100% to 0–32767 digital speed ref, clears MPET bits if configured, releases brake automatically, and writes speed reference override. **Notably: no ramping or slew-rate limiting**.
* `MCF8329AComponent::handle_fault_shutdown_(...)`  
  On any active fault: forces speed to 0 once. If a “severe current fault” is present, latches a lockout that blocks future non-zero speed commands until `clear_faults` succeeds.
* `MCF8329AComponent::log_algorithm_state_transition_(...)`  
  Reads `ALGORITHM_STATE` and logs transitions (IDLE→ALIGN→OPEN_LOOP→CLOSED_LOOP… or into FAULT), which is essential for diagnosing where you are failing (“before CLOSED” vs “during transition”).
* `MCF8329AComponent::log_hw_lock_diagnostics_()` and `log_mpet_bemf_diagnostics_()`  
  One-shot diagnostics emitted on relevant fault types, dumping key config bits.

**`components/mcf8329a/mcf8329a.h` (register addresses and bitfields)**
Defines the registers and bit masks this component touches (fault/config/startup/closed-loop registers).

### What the firmware does not do that matters for proper tuning

Even though the component is thoughtful (guardrails, good logging), several tuning-critical capabilities are missing or incomplete relative to TI’s tuning guidance:

The firmware does not implement a **safe speed-reference profile** (soft-start/ramps). It can jump speed setpoints instantly. For a sensorless controller, abrupt reference changes can create worst-case estimator and current-loop demands, making handoff instability and current-limit trips more likely during experimentation.

The firmware does not provide **closed-loop handoff shaping knobs** that TI identifies as the drivers of the handoff current profile: **THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP, SPD_LOOP_KI**. TI explicitly ties smoother handoff to these parameters and demonstrates that THETA_ERROR_RAMP_RATE markedly changes the current profile. citeturn6search45turn6search44

The firmware does not perform (or guide) **MPET-based motor parameter extraction**. MCF8329A supports “offline motor parameters measurement with MPET” and DACOUT variable monitoring, but the component mostly *clears MPET bits to avoid MPET-related faults*, and only seeds some values to non-zero to escape a fault. citeturn6search0  
This means you are mostly hand-tuning BEMF constant and handoff settings without the full toolchain approach TI intended.

The firmware does not surface a robust **speed feedback measurement** path. Your PCB includes the MCF8329A **FG pin** (frequency proportional to motor speed), but the ESPHome component does not use it. Without speed feedback, you can’t easily reproduce TI’s recommended loop of “verify speed feedback tracks reference in open loop, then adjust OL_ILIMIT/accel/handoff”. citeturn6search45turn6search44

I²C robustness is not enforced as strongly as TI recommends. TI’s I²C app note for this family calls out a needed **≥100 µs inter-byte delay**, and TI forum clarification states it is mandatory for both reads and writes. citeturn7search24turn7search0  
If you run I²C at a “normal” 100 kHz or 400 kHz with standard hardware transactions, you may not meet this constraint, risking partial/incorrect register writes that behave like “tuning doesn’t work”.

A specific implementation gap likely relevant to **HW_LOCK_LIMIT “false trips”**: the component’s own knowledge notes identify that some **lock-current deglitch** settings exist in the MCF8329A but are **not exposed/applied** (and were applied in another related component). If your current spikes are brief (especially at handoff), deglitch settings are often decisive.

## Hardware constraints from the ESC netlist that affect safe tuning

Your uploaded netlist provides enough information to identify key constraints that directly impact tuning safety and HW_LOCK_LIMIT behaviour:

The ESC is a **discrete MOSFET three-phase inverter** driven by an MCF8329A (U1). The MCF8329A is a **three-half-bridge gate driver with integrated sensorless FOC** and a **single-shunt CSA**, designed for external MOSFETs. citeturn6search0

The current shunt is **R12 = 1 mΩ**, placed between **LSS and GND** (a classic low-side single-shunt placement). This is consistent with MCF8329A’s “single shunt” architecture. citeturn6search0

The netlist shows:
* **MCF8329A pins**: SDA/SCL, DIR, BRAKE (“BREAK” net), nFAULT, FG, DRVOFF2, AVDD/DVDD/GVDD, charge-pump caps (CPH/CPL), bootstrap caps (BSTA/BSTB/BSTC) and phase nodes OUTA/B/C.
* **External MOSFETs**: six discrete N‑MOSFETs; the listed part has very low RDS(on) and large gate charge (Qg), which increases switching stress and makes gate-drive tuning/layout important in practice.
* **Bus capacitance**: a large bulk capacitor plus many ceramics on VBAT → this reduces ripple but can also mask fast transients; regen during braking can still spike bus voltage.
* **FG and nFAULT** are pulled up to **AVDD** via resistors, which makes them easy to interface to a 3.3 V MCU without level shifting.
* An **ADS1115** (16-bit I²C ADC) is on the same I²C bus; its inputs appear routed to “TS1” and “LEAK” nets (likely temperature/monitoring), not directly to the shunt—so **your firmware cannot infer motor current from ADS1115 without additional analogue routing**.

Practical consequences:

Because the MCF8329A uses a single-shunt scheme, some braking strategies are constrained. TI’s pre-startup tuning note explicitly says **current-based brake is only available for low-side braking; if configured with high-side braking the device will default to time-based braking**. citeturn8search24  
That matters if you try to mitigate handoff spikes by relying on “smart braking”; some modes will simply not behave as you expect.

Because the board exposes **FG**, you have a clear path to add speed telemetry and RPM limits in firmware without additional analogue hardware.

Because RSENSE = 1 mΩ and CSA gain is configurable (5/10/20/40 V/V), BASE_CURRENT and all “% of base current” limits can correspond to large absolute currents unless you deliberately scale BASE_CURRENT downward for bring-up—or impose firmware-side guardrails.

## Specific shortcomings and risks tied to HW_LOCK_LIMIT and open→closed-loop transition

### The HW_LOCK_LIMIT failure mode you’re seeing

TI’s handoff tuning note shows that handoff can fail depending on the handoff threshold (example: “handoff fails with OPN_CL_HANDOFF_THR = 50%”). citeturn6search45  
More importantly, TI states that after passing to closed loop there can be theta error and the device reduces it at a rate set by **THETA_ERROR_RAMP_RATE**, entering **CLOSED_LOOP_UNALIGNED_STATE**; the speed reference during this state follows **CL_SLOW_ACC**, and the phase-current profile depends on **THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP, SPD_LOOP_KI**. citeturn6search45turn6search44

Your firmware currently tunes the open-loop accel and handoff threshold, but **does not expose those handoff-smoothing variables**, so you cannot easily “shape” the transition; if the default internal values are too aggressive for a small low-inductance outrunner, a current spike will trip HW_LOCK_LIMIT even if steady-state limits look conservative.

### BASE_CURRENT scaling and why “2 A at default” is plausible

TI’s base-current guidance (from a TI engineer response) effectively uses:

* Choose RSENSE and CSA_GAIN so that **1.5 / (RSENSE · CSA_GAIN)** is slightly above rated max peak current; then apply a scaling factor of **(32768 / 1200)** and write that to the BASE_CURRENT field. citeturn6search2

With **RSENSE = 0.001 Ω** and **CSA_GAIN = 40 V/V**, the “full-scale” current is:

* 1.5 / (0.001 · 40) = **37.5 A** (so BASE_CURRENT ≈ 37.5 A).

If your ILIMIT defaults to the minimum “5% of BASE_CURRENT”, then the first clipping point is ~1.875 A. That is right in the range you observed (~2 A), which means you may be *hitting limit constantly* during open-loop/handoff rather than having “2 A of healthy margin”. That also explains why you can “spin up” but never settle into closed loop: there isn’t enough torque headroom at the critical transition.

### I²C reliability as a hidden contributor

TI explicitly states: “Make sure at least **100 µs delay between each byte of data** for reliable communication” in the programming note for this device family. citeturn7search24  
A TI forum clarification says this requirement is **mandatory** and applies to **both reads and writes**, even in Q1 variants. citeturn7search0

If your ESP32 I²C peripheral cannot insert true inter-byte gaps, your safest option during tuning is to **lower I²C clock** enough that each byte transmission consumes ≥100 µs (often ≤50 kHz effective). If you do not, you risk intermittent mis-writes (e.g., OL_ACC or ILIMIT not being what you think), which looks exactly like “tuning is impossible”.

### Other safety gaps

There is no firmware-level RPM ceiling or acceleration ceiling tied to “4S + 750 kV”, even though that combination can reach ~10–12 krpm no-load and is mechanically dangerous in failure scenarios.

There is no firmware-integrated emergency stop input (e.g., dedicated kill switch entity that asserts DRVOFF and brake in a deterministic order) beyond setting speed to 0 + optional brake.

There is no parameter “test mode” that sequences open-loop start, observes whether the algorithm reaches CLOSED_LOOP_ALIGNED, then automatically backs off rather than continuing to bang into current limits.

## Concrete firmware changes and recommended safe defaults for a 2836 750 kV motor on 4S

### Recommended feature additions

The following changes are targeted at making handoff tuning safer and making HW_LOCK_LIMIT diagnosable rather than trial-and-error.

#### Add a speed-command ramp and soft-start profile

Add internal state:

* `target_speed_percent` (what HA/UI wants)
* `applied_speed_percent` (what you actually write to the chip)
* `ramp_rate_percent_per_s` and `ramp_rate_down_percent_per_s`
* optional `min_start_percent` and `min_start_hold_ms` (“start above 10% then ramp down” automation)

Then, in `update()` (or a faster scheduled tick), you adjust `applied_speed_percent` toward target by at most `ramp_rate * dt`, and only write the chip when the applied value changes enough to matter.

This avoids:
* step inputs that excite estimator/current-loop
* slamming into ILIMIT at handoff
* repeated HW_LOCK_LIMIT during small adjustments

#### Expose and apply TI’s handoff smoothing parameters

Per TI handoff tuning guidance, you should expose at least:

* `theta_error_ramp_rate`
* `cl_slow_acc`
* `spd_loop_kp`
* `spd_loop_ki`

Because TI explicitly states these shape IQ reference behaviour and thus current profile during handoff. citeturn6search45turn6search44

Even if you start with conservative defaults, simply being able to vary THETA_ERROR_RAMP_RATE is often the difference between stable and unstable transition.

#### Expose lock-current deglitch and relevant fault timing knobs

Given your HW_LOCK_LIMIT issue, you should add YAML options for:

* `lock_ilimit_deglitch`
* `hw_lock_ilimit_deglitch`

These values reduce sensitivity to short current spikes (especially during transition), allowing you to tune handoff without “instant failure”.

#### Add FG-based speed telemetry and RPM limiter

The PCB includes the FG pin. Use a pulse-counter/frequency measurement component on the ESP32 and publish:

* Electrical speed (Hz) and derived mechanical RPM (if poles/pole-pairs configured).
* A guardrail: if RPM exceeds a configured ceiling, force speed command → 0 and brake.

This also enables a more faithful reproduction of TI’s tuning workflow (“does speed feedback track reference?”), but using FG instead of DACOUT. citeturn6search45turn6search44

#### Enforce I²C communication constraints

Given TI’s mandatory inter-byte delay guidance, add a runtime check:

* If configured I²C bus frequency > 50 kHz (or a user-set `max_i2c_freq_khz`), emit a **warning at boot** and optionally refuse to drive the motor unless an override is set.

This is a safety feature: it reduces the probability of silent misconfiguration during a dangerous tuning process. citeturn7search24turn7search0

### Pseudocode for the most important changes

#### Speed ramp + soft-start wrapper

```cpp
// Called by HA number entity
void request_speed(float new_target_percent) {
  target_speed_percent_ = clamp(new_target_percent, 0, 100);

  // Optional: if starting from 0, enforce a minimum start percent for estimator robustness
  if (applied_speed_percent_ < 0.5f && target_speed_percent_ > 0.5f) {
    startup_boost_active_ = true;
    startup_boost_until_ms_ = millis() + min_start_hold_ms_;
    startup_boost_percent_ = min_start_percent_;   // e.g., 12%
  }
}

// Called periodically (e.g., every 20–50 ms)
void ramp_tick(float dt_s) {
  float desired = target_speed_percent_;

  if (startup_boost_active_) {
    if (millis() < startup_boost_until_ms_) {
      desired = std::max(desired, startup_boost_percent_);
    } else {
      startup_boost_active_ = false;
    }
  }

  float rate = (desired > applied_speed_percent_) ? ramp_up_pct_per_s_ : ramp_down_pct_per_s_;
  float step = rate * dt_s;

  float next = applied_speed_percent_;
  if (fabs(desired - next) <= step) next = desired;
  else next += (desired > next ? step : -step);

  // Only write if changed meaningfully
  if (fabs(next - applied_speed_percent_) >= 0.5f) {
    applied_speed_percent_ = next;
    set_speed_percent(applied_speed_percent_, "ramped");
  }
}
```

#### “Safe handoff” monitoring using algorithm state

```cpp
void update() {
  uint32_t algo_status;
  if (read_reg32(REG_ALGO_STATUS, algo_status)) {
    uint16_t state;
    if (read_reg16(REG_ALGORITHM_STATE, state)) {
      // If we're in OPEN_LOOP too long, back off and log "handoff not reached"
      if (state == MOTOR_OPEN_LOOP && millis() - open_loop_entry_ms_ > max_open_loop_ms_) {
        set_speed_percent(0, "open_loop_timeout");
        set_brake_override(true);
      }

      // If we enter CLOSED_LOOP_UNALIGNED, temporarily clamp ramp up rate
      if (state == MOTOR_CLOSED_LOOP_UNALIGNED) {
        ramp_up_pct_per_s_ = handoff_ramp_up_pct_per_s_;  // slower during handoff
      }
    }
  }

  // existing fault reads...
}
```

#### “Current-limit safety envelope” without true current telemetry

Even without current telemetry, you can treat repeated HW_LOCK_LIMIT as a proxy for “handoff spike too high”, and automatically derate:

```cpp
if (hw_lock_fault_detected) {
  hw_lock_events_++;
  if (hw_lock_events_ >= 2) {
    // Automatically reduce aggressiveness knobs (if user enabled auto-derate mode)
    // Example: reduce open-loop accel, reduce handoff threshold, slow ramps
    startup_open_loop_accel_code_ = max(startup_open_loop_accel_code_ - 1, MIN_ACCEL_CODE);
    startup_open_to_closed_handoff_threshold_ = max(startup_open_to_closed_handoff_threshold_ - 1, MIN_HANDOFF_CODE);
    ramp_up_pct_per_s_ = min(ramp_up_pct_per_s_, 5.0f);

    apply_startup_motor_config_();  // re-apply
    set_speed_percent(0, "hw_lock_derate");
  }
}
```

### Conservative YAML defaults for your 4S / 750 kV / 2836 scenario

These are designed to reduce HW_LOCK_LIMIT probability *during tuning*, not to produce maximum performance.

Key computed values:

* Electrical max speed estimate for 4S full charge:
  * ~16.8 V · 750 rpm/V ≈ 12,600 rpm no-load
  * With a **12‑pole rotor (6 pole-pairs)**: electrical Hz ≈ 12,600/60·6 ≈ **1260 Hz** (matches the repo README guidance).
* For RSENSE = 1 mΩ, CSA_GAIN = 40 V/V, a “hardware-faithful” BASE_CURRENT is around 37.5 A. citeturn6search2  
  But for safer tuning, you may temporarily set BASE_CURRENT lower so the discrete % steps map to smaller amps.

Suggested bring-up config (tuning-safe):

* I²C:
  * start at **≤50 kHz** during tuning (to respect the 100 µs inter-byte requirement). citeturn7search24turn7search0
* Startup:
  * `startup_mode: double_align`
  * `startup_align_time: 100ms`
  * `startup_brake_mode: recirculation` (avoid aggressive braking/regen during experiments)
* Scaling:
  * `startup_csa_gain_v_per_v: 40`
  * `startup_base_current_amps: 10.0` initially (tuning-safe).  
    After stable closed-loop handoff, move toward the “hardware-faithful” number (≈37.5 A) while keeping % limits low.
* Current limits:
  * `phase_current_limit_percent: 10` (maps to 1.0 A if BASE_CURRENT=10 A)
  * `startup_open_loop_limit_source: ol_ilimit`
  * `startup_open_loop_ilimit_percent: 10`
  * `startup_lock_ilimit_percent: 15`
  * `startup_hw_lock_ilimit_percent: 15`
* Handoff:
  * `startup_open_loop_accel_hz_per_s: 5`
  * `startup_open_to_closed_handoff_percent: 15`
  * Keep `startup_auto_handoff_enable: false` initially for repeatability; enable later if needed.

If you still trip current limits before reaching closed-loop, do **not** jump straight to higher currents. Instead, first apply the handoff-smoothing parameter exposure described above (THETA_ERROR_RAMP_RATE etc.) because TI explicitly frames those as the key to smoothing transition current. citeturn6search45turn6search44

### Capability comparison table

| Area | Current firmware (as reviewed) | Recommended upgrade | Why it matters for your HW_LOCK_LIMIT issue |
|---|---|---|---|
| Speed command shaping | Direct step write of digital speed control; no slew control | Add ramp/soft-start and optional minimum-start boost | Reduces handoff excitation and current spikes; makes tuning safer |
| Handoff smoothing knobs | Tunes OL_ILIMIT/OL_ACC_A1/handoff threshold; does not expose THETA_ERROR_RAMP_RATE/CL_SLOW_ACC/SPD_LOOP gains | Expose/apply those parameters | TI explicitly ties handoff current profile to these variables citeturn6search45turn6search44 |
| Speed feedback | None (no FG usage) | Measure FG; publish RPM/Hz; RPM limit | Enables data-driven tuning and safety cut-outs |
| Current telemetry | None (beyond fault bits) | Add any current proxy you can (future hardware route SOx/ADC); at minimum, log effective amp limits and fault counts | Helps distinguish “not enough torque” vs “handoff spike” |
| Fault containment | Forces speed→0 on faults; software lockout on severe faults | Add “test mode” that automatically backs down aggressiveness after repeated HW_LOCK_LIMIT | Makes tuning iterative and safer |
| I²C robustness | General retries/scan; no hard enforcement of TI inter-byte requirement | Enforce low I²C rate or implement compliant byte timing; warn/refuse unsafe bus configs | Prevents silent misconfiguration; TI states 100 µs inter-byte delay is required citeturn7search24turn7search0 |
| Lock detection tuning | Exposes limits/modes and some thresholds | Expose lock/hw-lock deglitch parameters | Reduces nuisance HW_LOCK_LIMIT from brief spikes |
| Emergency stop | Indirect (speed 0 + brake switch) | Deterministic e‑stop: assert DRVOFF + brake + speed 0 with clear state machine | Reduces risk during bench tests |

## Safe tuning and validation procedure for a 2836 750 kV on 4S

This procedure is designed to answer your specific uncertainty: “does it limit out before CLOSED, or during transition?”

### Test setup and instrumentation

Use a bench supply for early tests (even if final use is battery):

* Bench PSU with current limit and fast display (or an inline wattmeter). Start current limit low (e.g., 0.5–1.0 A) and raise gradually.
* Motor secured (no prop, no load) and physically restrained.
* Measure:
  * VM/VBAT at the ESC input.
  * Phase connection integrity.
  * Optional: scope FG and nFAULT if possible.

Ensure I²C bus compliance:

* Configure I²C clock ≤ 50 kHz during tuning to reduce the chance of violating TI’s inter-byte delay requirement. citeturn7search24turn7search0

### Step-by-step procedure

Initial configuration audit:
* Boot and ensure the firmware prints the startup configuration summary (CSA gain, BASE_CURRENT, ILIMIT and open-loop/lock parameters).
* Confirm CSA gain and BASE_CURRENT are what you intend; if you use BASE_CURRENT=10 A for tuning, understand that the controller’s per-unit scaling is altered and your % limits map to smaller amps.

Fault-free idle check:
* Clear faults.
* Confirm `fault_active = false`, `sys_enable` behaves as expected, VM reads plausibly.

Open-loop start characterisation:
* Command a small non-zero speed (with the new ramp feature, ideally).
* Watch logs for algorithm state transitions. You want to see:
  * IDLE → ALIGN → OPEN_LOOP
* Hold for 3–5 seconds. Observe input current. If you are hitting 2 A immediately at low command with conservative ILIMIT, check that:
  * BASE_CURRENT and ILIMIT mapping make sense (e.g., 5% of 37.5 A ≈ 1.9 A).
  * You are not trying to start at too low a command: TI’s own handoff discussions imply that insufficient BEMF can prevent stable estimator alignment; many systems behave better starting above a small threshold. citeturn6search45turn6search44

Handoff attempt:
* Increase command gradually until the algorithm attempts handoff.
* Use the algorithm-state logs to identify whether you ever enter:
  * `MOTOR_CLOSED_LOOP_UNALIGNED` or `MOTOR_CLOSED_LOOP_ALIGNED`
* If you never reach a closed-loop state and fault in OPEN_LOOP, you likely have **insufficient torque/current headroom** or the open-loop profile is not compatible with the motor. Actions:
  * Reduce OL acceleration before increasing current (because aggressive accel can cause slip and high current).
  * Increase OL_ILIMIT modestly if current is clearly capped and the motor can’t accelerate.
* If you reach CLOSED_LOOP_UNALIGNED and then fault (or fault right at the transition), you are in the exact regime TI targets with handoff-smoothing tuning. Actions:
  * Reduce handoff threshold (avoid very high handoff thresholds; TI shows failure at 50% in an example). citeturn6search45
  * Adjust THETA_ERROR_RAMP_RATE and CL_SLOW_ACC and speed-loop gains (requires firmware changes); TI recommends THETA_ERROR_RAMP_RATE ≈ 100b as a baseline and shows it affects damping. citeturn6search44

HW_LOCK_LIMIT-specific loop:
* If HW_LOCK_LIMIT occurs:
  * Record: command %, last algorithm state, VM, and whether it happens at handoff.
  * Lower OL acceleration first; then adjust handoff smoothing.
  * Only then adjust current limits upward, in small steps.

### Expected values and sanity checks for your hardware

With RSENSE=1 mΩ and CSA gain 40, a “hardware-like” BASE_CURRENT value is on the order of tens of amps (≈37.5 A with the TI 1.5 V scaling guidance). citeturn6search2  
That means even minimum % limits can still be in the amp range. If your goal is sub-amp bring-up, you must either:
* temporarily reduce BASE_CURRENT during tuning, or
* accept that the granularity of % steps (starting at 5%) may not support <1 A limits.

For pre-start braking/coasting behaviour, TI notes that coasting (Hi‑Z) can be useful to avoid high brake currents under certain windmilling/reverse conditions, and that brake configuration has constraints around current-based brake. citeturn8search24  
For a small outrunner on a bench, prefer less aggressive braking to reduce bus transients.

### Prioritised implementation plan with effort and risk

| Priority | Change | Estimated effort | Risk | Rationale |
|---|---|---:|---|---|
| Highest | Add speed ramp + “start-above-threshold” soft-start option | 0.5–1 day | Low | Immediately reduces tuning danger and reduces handoff spikes caused by step inputs |
| Highest | Enforce/mitigate TI I²C 100 µs inter-byte requirement (warn + recommend ≤50 kHz; optionally refuse unsafe config) | 0.5–1 day | Medium | Prevents silent misconfiguration that wastes tuning time and can be unsafe citeturn7search24turn7search0 |
| High | Expose THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP, SPD_LOOP_KI | 1–2 days | Medium | TI explicitly ties these to handoff current behaviour; expected to address your exact “handoff trips HW_LOCK_LIMIT” issue citeturn6search45turn6search44 |
| High | Expose lock/hw-lock deglitch settings | 0.5–1 day | Medium | Reduces nuisance HW_LOCK_LIMIT due to transient spikes |
| Medium | Add FG telemetry + RPM limits | 1–2 days | Low–Medium | Adds observability and a critical safety guardrail |
| Medium | Add “test mode” state machine: controlled open-loop start, detect success/failure and auto-derate knobs | 2–4 days | Medium | Turns tuning into a repeatable experiment rather than manual trial-and-error |
| Longer term | Add MPET procedure support and optionally EEPROM persistence | 1–2+ weeks | High | Requires careful sequencing; but aligns with TI’s intended workflow and can produce stable parameters citeturn6search0 |

## References and prioritised sources

Primary TI sources (highest priority):

* Texas Instruments, **MCF8329A product page** (features, architecture, protections, FG/DACOUT, supported electrical frequency and PWM). citeturn6search0  
* Texas Instruments, **SLLA665 – MCF83xx: Open Loop to Closed Loop Handoff Tuning** (handoff failure modes; importance of THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP/KI; example recommended settings). citeturn6search45turn6search44  
* Texas Instruments, **SLLA663 – Motor Pre‑Startup Tuning for MCx83xx** (coasting and braking constraints; current-based brake caveats). citeturn8search24  
* Texas Instruments, **SLLA662 – How to Program I²C for MCx83xx Device Family** (≥100 µs inter-byte delay requirement). citeturn7search24  

Secondary (useful but less “primary” than PDFs):

* TI E2E support forum (TI engineer response), **MCF8329A base current calculation**: guidance using 1.5/(RSENSE·CSA_GAIN) and scaling factor for BASE_CURRENT register. citeturn6search2  
* TI E2E forum clarification on the **mandatory** nature of the 100 µs I²C inter-byte delay. citeturn7search0  

Notes on missing/unspecified items:

* The repo snapshot did not include full PCB design files (schematic/PCB layout). The netlist enables meaningful constraints analysis (RSENSE, external MOSFET topology, pin breakout), but **layout-dependent risks** (current-loop inductance, sense routing, gate loop ringing, Kelvin sense integrity, thermal pad/plane design) cannot be fully assessed without the layout files or fabrication outputs.