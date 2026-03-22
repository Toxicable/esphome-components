# Datasheet Index: `open-loop-to-closed-loop-handoff-guide.pdf`

## Generated Artifacts
- Canonical source: `open-loop-to-closed-loop-handoff-guide.pdf`
- Compact source: `open-loop-to-closed-loop-handoff-guide.compact.txt`
- Compact line map: `open-loop-to-closed-loop-handoff-guide.compact.map.tsv`

## Normalization
- Source text is extracted from the PDF in memory during processing.
- Compact output removes only known page boilerplate/footer lines and collapses repeated blank lines.
- Canonical lines are line numbers in the extracted text stream used for indexing, not a stored transcript.
- Canonical lines: 271; compact lines: 248

## Removed Boilerplate Summary
- Removed lines: 23
- copyright: 12
- revision_tag: 6
- website: 5

## Section Headings (Canonical Line Numbers)
- L42: 1 Introduction
- L56: 1.1 Open Loop and Handoff Stage
- L74: 2 MCF83xx - Open Loop to Closed Loop Handoff Tuning SLLA665 – APRIL 2025
- L77: 2 EEPROM Configurations Affecting Open Loop to Closed Loop Handoff
- L80: 1 OL_ILIMIT Open Loop Current Limit
- L81: 2 OL_ACC_A1 Open loop Acceleration coefficient A1 (in Hz/sec)
- L82: 3 OL_ACC_A2 Open loop Acceleration coefficients A2 (in Hz/sec^2)
- L83: 4 OPN_CL_HANDOFF_THR Open loop to closed loop handoff threshold (% of MAX_SPEED).
- L85: 5 AUTO_HANDOFF_MIN_BEMF Minimum BEMF required before handoff. Handoff speed depends on
- L87: 6 THETA_ERROR_RAMP_RATE Ramp rate for reducing theta error (difference between open loop
- L89: 8 CL_SLOW_ACC Closed loop acceleration rate while theta error is non-zero.
- L98: 3 Open Loop to Closed Loop Handoff Tuning
- L107: 3.1 Experimental Way to Tune Open Loop and Handoff Stage
- L108: 3.1.1 Step-1: Initial Configuration
- L128: 4 MCF83xx - Open Loop to Closed Loop Handoff Tuning SLLA665 – APRIL 2025
- L131: 3.1.2 Step-2: Tuning of Parameters In Case Handoff is not Proper
- L132: 3.1.2.1 Tuning of OL_ILIMIT
- L145: 3.1.2.2 Tuning of Open Loop Acceleration Co-Efficient A1
- L157: 3.1.2.3 Tuning of Handoff Thresholds
- L164: 6 MCF83xx - Open Loop to Closed Loop Handoff Tuning SLLA665 – APRIL 2025
- L168: 3.1.3 Step-3: Tuning of Handoff Configurations
- L197: 8 MCF83xx - Open Loop to Closed Loop Handoff Tuning SLLA665 – APRIL 2025
- L200: 4 Optimum Handoff
- L214: 4.1 Open Loop Time With the Recommended Settings
- L220: 4.2 Open Loop Time by Following Optimum Handoff Steps
- L235: 10 MCF83xx - Open Loop to Closed Loop Handoff Tuning SLLA665 – APRIL 2025
- L238: 5 Summary
- L244: 6 References

## Register Offsets (Canonical Line Numbers)
- No register offsets detected with default pattern.

## Quick Token References (Canonical Line Numbers)
- Selection mode: `auto`; profile: `none`; tokens: 11
- `OL_ILIMIT`:
  - L60: by OL_ILIMIT. The motor speed is increased in open loop with peak value of the motor phase current limited to
  - L61: OL_ILIMIT. The function of the open-loop operation is to drive the motor to a speed at which the motor generates
  - L80: 1 OL_ILIMIT Open Loop Current Limit
  - L91: TI recommends OL_ILIMIT_CONFIG to be set to 0b, if the bit-field is available for configuration in the
  - L109: 1. Set open loop OL_ILIMIT to 0.5×(rated motor peak phase current)A.
  - L123: Figure 3-1. Proper Handoff - OL_ILIMIT = 0.5, OL_ACC_A1 = 25Hz/sec, OPN_CL_HANDOFF_THR = 20%
  - L132: 3.1.2.1 Tuning of OL_ILIMIT
  - L133: Sometimes setting OL_ILIMIT to (0.5×Rated current)A does not result in a proper handoff because of large
  - L134: inertia of some motors. In these scenarios increase the OL_ILIMIT in steps till the rated peak current of the
  - L138: Figure 3-2. Handoff Fails With OL_ILIMIT = 0.25A
  - L139: Figure 3-3. Handoff is Proper With OL_ILIMIT = 0.5A
  - L146: If the speed feedback is not able to reach open loop velocity reference even after increasing OL_ILIMIT, then
- `OL_ACC_A1`:
  - L59: and A2 are configured through OL_ACC_A1 and OL_ACC_A2 respectively. The current limit in open loop is set
  - L81: 2 OL_ACC_A1 Open loop Acceleration coefficient A1 (in Hz/sec)
  - L112: 3. Set OL_ACC_A1 equal to (MAX_SPEED/10)Hz/s and OL_ACC_A2 to zero.
  - L123: Figure 3-1. Proper Handoff - OL_ILIMIT = 0.5, OL_ACC_A1 = 25Hz/sec, OPN_CL_HANDOFF_THR = 20%
  - L147: the open loop A1 needs to be reduced. Further, the recommended setting is to keep OL_ACC_A1 equal to
  - L150: Higher OL_ACC_A1 increases the speed reference quickly, and the motor cannot track the speed reference with
  - L156: coefficient OL_ACC_A1 until handoff is proper.
  - L161: OL_ACC_A1. So, the handoff fails in this scenario. In that case, set handoff threshold to a lower value.
  - L206: 3. With OL_ACC_A2 = 0 (or the lowest setting available). Set OL_ACC_A1 to next higher value of
  - L207: OL_ACC_A1 = (MAX_SPEED/10)Hz/s. Verify whether handoff is proper or not. If the handoff is not proper,
  - L208: revert to the previous setting else set OL_ACC_A1 to the next higher value and continue this procedure.
  - L209: 4. Now set OL_ACC_A1 at half of last successful handoff setting, and now try increasing the value of
- `OPN_CL_HANDOFF_THR`:
  - L68: 1b. Users also have an option to manually set the handoff speed by configuring OPN_CL_HANDOFF_THR and
  - L83: 4 OPN_CL_HANDOFF_THR Open loop to closed loop handoff threshold (% of MAX_SPEED).
  - L110: 2. Set maximum speed of motor in MAX_SPEED configuration and then set OPN_CL_HANDOFF_THR to
  - L123: Figure 3-1. Proper Handoff - OL_ILIMIT = 0.5, OL_ACC_A1 = 25Hz/sec, OPN_CL_HANDOFF_THR = 20%
  - L158: The recommendation is to not set OPN_CL_HANDOFF_THR to a very high value, as the set OL_ILIMIT on
  - L159: occasion is not sufficient to reach the set speed. For MOTOR1, if OPN_CL_HANDOFF_THR is set to 50%, the
  - L167: Figure 3-5. Handoff Fails with OPN_CL_HANDOFF_THR = 50%
  - L211: 5. Set OPN_CL_HANDOFF_THR to the next value higher than 20%. Verify whether handoff is proper or not. If
  - L212: the handoff is not proper, revert to the previous setting else set OPN_CL_HANDOFF_THR to the next higher
  - L217: OPN_CL_HANDOFF_THR = 20%
  - L224: OL_ILIMIT = 2A, OL_ACC_A1 = 50Hz/s, OL_ACC_A2 = 25Hz/s^2, OPN_CL_HANDOFF_THR = 18%
- `THETA_ERROR_RAMP_RATE`:
  - L70: transition with ramp rate configured by THETA_ERROR_RAMP_RATE, for a smooth transition to closed loop
  - L87: 6 THETA_ERROR_RAMP_RATE Ramp rate for reducing theta error (difference between open loop
  - L171: of THETA_ERROR_RAMP_RATE. This state is referred as CLOSED_LOOP_UNALIGNED_STATE. The speed
  - L175: THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP and SPD_LOOP_KI.
  - L176: The following are few captures to show how the THETA_ERROR_RAMP_RATE affects the
  - L180: For THETA_ERROR_RAMP_RATE = 111b, the IQ_REF_CLOSED_LOOP is varying at faster rate, and we
  - L182: Figure 3-6. THETA_ERROR_RAMP_RATE = 111b
  - L188: For THETA_ERROR_RAMP_RATE = 000b, the IQ_REF_CLOSED_LOOP is varying at slower rate, and we
  - L190: Figure 3-7. THETA_ERROR_RAMP_RATE = 000b
  - L192: Recommended setting for THETA_ERROR_RAMP_RATE is 100b. Increase this setting for a faster
- `OL_ACC_A2`:
  - L59: and A2 are configured through OL_ACC_A1 and OL_ACC_A2 respectively. The current limit in open loop is set
  - L82: 3 OL_ACC_A2 Open loop Acceleration coefficients A2 (in Hz/sec^2)
  - L112: 3. Set OL_ACC_A1 equal to (MAX_SPEED/10)Hz/s and OL_ACC_A2 to zero.
  - L206: 3. With OL_ACC_A2 = 0 (or the lowest setting available). Set OL_ACC_A1 to next higher value of
  - L210: OL_ACC_A2, until we get a successful handoff.
  - L216: OL_ILIMIT = 1A, OL_ACC_A1 = 25Hz/s (as there is no 30Hz/s configuration), OL_ACC_A2 = 0Hz/s^2,
  - L224: OL_ILIMIT = 2A, OL_ACC_A1 = 50Hz/s, OL_ACC_A2 = 25Hz/s^2, OPN_CL_HANDOFF_THR = 18%
- `MAX_SPEED`:
  - L83: 4 OPN_CL_HANDOFF_THR Open loop to closed loop handoff threshold (% of MAX_SPEED).
  - L110: 2. Set maximum speed of motor in MAX_SPEED configuration and then set OPN_CL_HANDOFF_THR to
  - L112: 3. Set OL_ACC_A1 equal to (MAX_SPEED/10)Hz/s and OL_ACC_A2 to zero.
  - L148: (MAX_SPEED/10)Hz/s.
  - L207: OL_ACC_A1 = (MAX_SPEED/10)Hz/s. Verify whether handoff is proper or not. If the handoff is not proper,
- `IQ_REF_CLOSED_LOOP`:
  - L174: depends on the how IQ_REF_CLOSED_LOOP varies post open loop state, which in turn depends on
  - L177: IQ_REF_CLOSED_LOOP variations during handoff stage and thus current profile for a given CL_SLOW_ACC,
  - L178: SPD_LOOP_KP and SPD_LOOP_KI. IQ_REF_CLOSED_LOOP is brought out on the DACOUT1 pin of the
  - L180: For THETA_ERROR_RAMP_RATE = 111b, the IQ_REF_CLOSED_LOOP is varying at faster rate, and we
  - L188: For THETA_ERROR_RAMP_RATE = 000b, the IQ_REF_CLOSED_LOOP is varying at slower rate, and we
  - L193: IQ_REF_CLOSED_LOOP variation at handoff point and decrease this setting to slow down the
  - L194: IQ_REF_CLOSED_LOOP variation at handoff point.
- `CL_SLOW_ACC`:
  - L89: 8 CL_SLOW_ACC Closed loop acceleration rate while theta error is non-zero.
  - L172: reference in this state follows CL_SLOW_ACC (suggested value is half of CL_ACC) .
  - L175: THETA_ERROR_RAMP_RATE, CL_SLOW_ACC, SPD_LOOP_KP and SPD_LOOP_KI.
  - L177: IQ_REF_CLOSED_LOOP variations during handoff stage and thus current profile for a given CL_SLOW_ACC,
- `AUTO_HANDOFF_EN`:
  - L67: automatically determined based on the measured back-EMF and motor speed if AUTO_HANDOFF_EN is set to
  - L69: setting AUTO_HANDOFF_EN to 0b. The theta error (Ɵgen - Ɵest) at handoff point is decreased linearly after
  - L84: Speed at which handoff happens if AUTO_HANDOFF_EN = 0h.
  - L86: motor back emf, if AUTO_HANDOFF_EN = 1h
- `SPEED_FDBK`:
  - L113: 4. Use the DACOUT1 and DACOUT2 pins to plot SPEED_FDBK and SPEED_REF_OPEN_LOOP variables on
  - L116: phase current (OUTA), SPEED_FDBK, and SPEED_REF_OPEN_LOOP variables. Observe if SPEED_FDBK is
  - L203: temperature), the variables SPEED_REF_OPEN_LOOP and SPEED_FDBK on DAC needs to match for
- `SPEED_REF_OPEN_LOOP`:
  - L113: 4. Use the DACOUT1 and DACOUT2 pins to plot SPEED_FDBK and SPEED_REF_OPEN_LOOP variables on
  - L116: phase current (OUTA), SPEED_FDBK, and SPEED_REF_OPEN_LOOP variables. Observe if SPEED_FDBK is
  - L117: tracking SPEED_REF_OPEN_LOOP by the end of open loop, for at least 30% of open loop time. This makes
  - L203: temperature), the variables SPEED_REF_OPEN_LOOP and SPEED_FDBK on DAC needs to match for

Regenerate with: `./tools/datasheet_prepare.py components/mcf8329a/datasheets/open-loop-to-closed-loop-handoff-guide.pdf`
