# Datasheet Index: `tuning-guide.pdf`

## Generated Artifacts
- Canonical source: `tuning-guide.pdf`
- Compact source: `tuning-guide.compact.txt`
- Compact line map: `tuning-guide.compact.map.tsv`

## Normalization
- Source text is extracted from the PDF in memory during processing.
- Compact output removes only known page boilerplate/footer lines and collapses repeated blank lines.
- Canonical lines are line numbers in the extracted text stream used for indexing, not a stored transcript.
- Canonical lines: 864; compact lines: 842

## Removed Boilerplate Summary
- Removed lines: 22
- copyright: 22

## Section Headings (Canonical Line Numbers)
- L44: 1 Revision History
- L51: 2 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L54: 2 Introduction
- L83: 2.1 Hardware and GUI Setup
- L90: 2.1.1 Jumper Configuration
- L99: 2.1.2 External Connections
- L104: 4 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L108: 2.1.3 Connecting to the GUI
- L109: 2.1.3.1 Connect to computer
- L112: 2.1.3.2 Connect to the GUI
- L117: 2.1.3.3 Verify Hardware Connection
- L126: 3 Essential Controls
- L130: 3.1 Recommended Default Values
- L159: 3.2 Device and Pin Configuration
- L160: 3.2.1 Speed Input Mode
- L167: 6 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L177: 3.3 Control Configuration - Motor Parameters
- L178: 3.3.1 Maximum Motor Electrical Speed (Hz)
- L202: 3.4 Control configuration - Closed Loop
- L203: 3.4.1 Current Limit for Torque PI Loop
- L209: 3.5 Testing for Successful Startup into Closed Loop
- L247: 8 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L250: 3.6 Fault Handling
- L253: 3.6.1 MPET IPD Fault [MPET_IPD_Fault]
- L304: 3.6.2 MPET BEMF Fault [MPET_BEMF_Fault]
- L362: 3.6.3 Abnormal BEMF Fault [ABN_BEMF]
- L374: 3.6.4 Lock Current Limit [LOCK_LIMIT]
- L377: 10 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L383: 3.6.5 Hardware lock Current Limit [HW_LOCK_LIMIT]
- L394: 3.6.6 No Motor Fault [NO_MTR]
- L403: 4 Basic Controls
- L408: 4.1 Device and Pin Configuration
- L409: 4.1.1 Power Saver or Sleep Mode for Battery Operated Applications
- L418: 4.1.2 Direction and Brake Pin Override
- L433: 4.2 System Level Configuration
- L434: 4.2.1 Tracking Motor Speed Feedback in Real Time
- L449: 12 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L455: 4.2.2 Improving Acoustic Performance
- L481: 4.2.3 Protecting the Power supply
- L508: 4.2.4 Monitoring Power Supply Voltage Fluctuations for Normal Motor Operation
- L526: 4.3 Control Configurations
- L527: 4.3.1 Motor Parameter Estimation to Minimize Motor Parameter Variation Effects
- L533: 14 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L564: 4.3.2 Initial Speed Detection of the Motor for Reliable Motor Resynchronization
- L589: 4.3.3 Unidirectional Motor Drive Detecting Backward Spin
- L615: 16 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L628: 4.3.4 Preventing Back Spin of Rotor During Startup
- L686: 4.3.5 Faster Startup Timing
- L714: 18 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L730: 4.3.6 Gradual and Smooth Start up Motion
- L738: 4.3.7 Improving Speed Regulation
- L771: 4.3.8 Stopping Motor Quickly
- L780: 4.3.9 Preventing Supply Voltage Overshoot During Motor Stop.
- L795: 4.3.10 Protecting Against Rotor Lock or Stall Condition
- L803: 20 MCF8316A Tuning Guide SLLU335A – AUGUST 2021 – REVISED JANUARY 2022
- L806: 4.3.11 Maximizing Thermal Efficiency and Increasing Thermal Performance
- L816: 4.3.12 Mitigating Electromagnetic Interference (EMI)
- L821: 4.3.13 Faster deceleration

## Register Offsets (Canonical Line Numbers)
- No register offsets detected with default pattern.

## Quick Token References (Canonical Line Numbers)
- Selection mode: `auto`; profile: `none`; tokens: 8
- `LOCK_LIMIT`:
  - L374: 3.6.4 Lock Current Limit [LOCK_LIMIT]
  - L383: 3.6.5 Hardware lock Current Limit [HW_LOCK_LIMIT]
  - L626: 5. If the device triggers Lock current limit [LOCK_LIMIT], then increase the open loop current limit
  - L697: If the device triggers Lock current limit [LOCK_LIMIT], it is recommended to increase [LOCK_ILIMIT] upto the
  - L706: might trigger LOCK_LIMIT. If this happens, reduce A1 and A2 until LOCK_LIMIT no longer triggers.
  - L718: LOCK_LIMIT fault handling:
  - L721: Increasing closed loop acceleration rate [CL_ACC] might trigger LOCK_LIMIT. If this happens, reduce
  - L799: MCF8316A provides options to either latch LOCK_LIMIT fault or auto retry for
- `SPD_LOOP_KI`:
  - L388: Step 1: Program Speed loop Kp [SPD_LOOP_KP], Speed loop Ki [SPD_LOOP_KI], current loop Kp
  - L740: [SPD_LOOP_KP] and [SPD_LOOP_KI]. Kp coefficient of speed loop [SPD_LOOP_KP] controls the settling time
  - L741: and speed overshoots. Ki coefficient of Speed loop [SPD_LOOP_KI] controls speed overshoot and ensures
  - L745: [SPD_LOOP_KI] to zero.
  - L765: Step 9: Speed loop Ki [SPD_LOOP_KI] is calculated using equationEquation 7.
- `SPD_LOOP_KP`:
  - L388: Step 1: Program Speed loop Kp [SPD_LOOP_KP], Speed loop Ki [SPD_LOOP_KI], current loop Kp
  - L740: [SPD_LOOP_KP] and [SPD_LOOP_KI]. Kp coefficient of speed loop [SPD_LOOP_KP] controls the settling time
  - L744: Auto Tuning: MCF8316A auto calculates Speed loop PI controller gains by setting [SPD_LOOP_KP] and
  - L757: Step 8: Speed loop Kp [SPD_LOOP_KP] is calculated using Equation 6.
- `ABN_BEMF`:
  - L362: 3.6.3 Abnormal BEMF Fault [ABN_BEMF]
  - L708: If the device triggers Abnormal BEMF [ABN_BEMF] fault, then it is recommended to increase the
- `CL_ACC`:
  - L712: Step 9: Increase Closed loop acceleration rate [CL_ACC]
  - L719: Closed loop acceleration rate [CL_ACC] can be increased until closed loop current reaches Lock
  - L721: Increasing closed loop acceleration rate [CL_ACC] might trigger LOCK_LIMIT. If this happens, reduce
  - L722: closed loop acceleration rate [CL_ACC] until no longer triggers.
- `LOCK_ILIMIT`:
  - L381: in the datasheet. If the load torque is still within the stall torque, we recommend to increase the Lock_ILIMIT in
  - L472: [HW_LOCK_ILIMIT]. If this fault gets triggered, increase the dead time.
  - L697: If the device triggers Lock current limit [LOCK_LIMIT], it is recommended to increase [LOCK_ILIMIT] upto the
  - L704: [LOCK_ILIMIT]. Open loop current can be measured using oscilloscope. Increasing Open loop
  - L720: detection current threshold [LOCK_ILIMIT]. Closed loop current can be measured using oscilloscope.
  - L797: Increase lock detection current threshold [LOCK_ILIMIT]
  - L801: LOCK_ILIMIT_MODE. We have defaulted the auto retry time and lock retry.
- `MTR_STARTUP`:
  - L637: Step 1: If IPD is chosen as startup method, select IPD in the Motor startup option [MTR_STARTUP] in “Control
  - L661: Step 1: Select Slow first cycle in the Motor startup option [MTR_STARTUP] in “Control Configuration – Motor
  - L691: Step 1: Select IPD [MTR_STARTUP] as the motor startup method.
  - L724: Step 1: Select Slow first cycle as the motor startup method in [MTR_STARTUP].
- `MTR_STOP`:
  - L773: [MTR_STOP] to either High side braking or Low side braking.
  - L774: Step 1: Configure Motor stop options [MTR_STOP] to either High side braking or Low side braking.
  - L787: Step 1: Configure Motor stop options [MTR_STOP] to Recirculation Mode.
  - L791: Step 1: Configure Motor stop options [MTR_STOP] to Active spin down

Regenerate with: `./tools/datasheet_prepare.py components/mcf8329a/datasheets/tuning-guide.pdf`
