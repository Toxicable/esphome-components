import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, number, select, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor", "switch", "number", "select", "button", "text_sensor"]

mcf8329a_ns = cg.esphome_ns.namespace("mcf8329a")
MCF8329AComponent = mcf8329a_ns.class_("MCF8329AComponent", cg.PollingComponent, i2c.I2CDevice)
MCF8329ABrakeSwitch = mcf8329a_ns.class_("MCF8329ABrakeSwitch", switch_.Switch)
MCF8329ADirectionSelect = mcf8329a_ns.class_("MCF8329ADirectionSelect", select.Select)
MCF8329ASpeedNumber = mcf8329a_ns.class_("MCF8329ASpeedNumber", number.Number)
MCF8329AClearFaultsButton = mcf8329a_ns.class_("MCF8329AClearFaultsButton", button.Button)
MCF8329AWatchdogTickleButton = mcf8329a_ns.class_("MCF8329AWatchdogTickleButton", button.Button)
MCF8329ATuneInitialParamsButton = mcf8329a_ns.class_("MCF8329ATuneInitialParamsButton", button.Button)
MCF8329ARunMPETButton = mcf8329a_ns.class_("MCF8329ARunMPETButton", button.Button)

CONF_AUTO_TICKLE_WATCHDOG = "auto_tickle_watchdog"
CONF_CLEAR_MPET_ON_STARTUP = "clear_mpet_on_startup"
CONF_ALLOW_UNSAFE_CURRENT_LIMITS = "allow_unsafe_current_limits"
CONF_MOTOR_BEMF_CONST = "motor_bemf_const"
CONF_BRAKE_MODE_CFG = "brake_mode"
CONF_BRAKE_TIME_CFG = "brake_time"
CONF_MODE_CFG = "mode"
CONF_ALIGN_TIME = "align_time"
CONF_DIRECTION_MODE_CFG = "direction_mode"
CONF_CSA_GAIN_V_PER_V = "csa_gain_v_per_v"
CONF_BASE_CURRENT_AMPS = "base_current_amps"
CONF_PHASE_CURRENT_LIMIT_PERCENT = "phase_current_limit_percent"
CONF_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT = "align_or_slow_current_limit_percent"
CONF_LOCK_MODE_CFG = "lock_mode"
CONF_LOCK_ILIMIT_PERCENT = "lock_ilimit_percent"
CONF_HW_LOCK_ILIMIT_PERCENT = "hw_lock_ilimit_percent"
CONF_LOCK_RETRY_TIME = "lock_retry_time"
CONF_ABN_SPEED_LOCK_ENABLE = "abn_speed_lock_enable"
CONF_ABN_BEMF_LOCK_ENABLE = "abn_bemf_lock_enable"
CONF_NO_MOTOR_LOCK_ENABLE = "no_motor_lock_enable"
CONF_LOCK_ABN_SPEED_THRESHOLD_PERCENT = "lock_abn_speed_threshold_percent"
CONF_ABNORMAL_BEMF_THRESHOLD_PERCENT = "abnormal_bemf_threshold_percent"
CONF_NO_MOTOR_THRESHOLD_PERCENT = "no_motor_threshold_percent"
CONF_MAX_SPEED_HZ = "max_speed_hz"
CONF_OPEN_LOOP_ILIMIT_PERCENT = "open_loop_ilimit_percent"
CONF_OPEN_LOOP_LIMIT_SOURCE = "open_loop_limit_source"
CONF_OPEN_LOOP_ACCEL_HZ_PER_S = "open_loop_accel_hz_per_s"
CONF_OPEN_LOOP_ACCEL2_HZ_PER_S2 = "open_loop_accel2_hz_per_s2"
CONF_AUTO_HANDOFF_ENABLE = "auto_handoff_enable"
CONF_OPEN_TO_CLOSED_HANDOFF_PERCENT = "open_to_closed_handoff_percent"
CONF_THETA_ERROR_RAMP_RATE = "theta_error_ramp_rate"
CONF_CL_SLOW_ACC_HZ_PER_S = "cl_slow_acc_hz_per_s"
CONF_MPET_USE_DEDICATED_PARAMS = "mpet_use_dedicated_params"
CONF_MPET_OPEN_LOOP_CURR_REF_PERCENT = "mpet_open_loop_curr_ref_percent"
CONF_MPET_OPEN_LOOP_SPEED_REF_PERCENT = "mpet_open_loop_speed_ref_percent"
CONF_MPET_OPEN_LOOP_SLEW_HZ_PER_S = "mpet_open_loop_slew_hz_per_s"
CONF_MPET_TIMEOUT_MS = "mpet_timeout_ms"
CONF_LOCK_ILIMIT_DEGLITCH_MS = "lock_ilimit_deglitch_ms"
CONF_HW_LOCK_ILIMIT_DEGLITCH_US = "hw_lock_ilimit_deglitch_us"
CONF_SPEED_LOOP_KP_CODE = "speed_loop_kp_code"
CONF_SPEED_LOOP_KI_CODE = "speed_loop_ki_code"
CONF_SPEED_RAMP_UP_PERCENT_PER_S = "speed_ramp_up_percent_per_s"
CONF_SPEED_RAMP_DOWN_PERCENT_PER_S = "speed_ramp_down_percent_per_s"
CONF_START_BOOST_PERCENT = "start_boost_percent"
CONF_START_BOOST_HOLD_MS = "start_boost_hold_ms"

CONF_BRAKE = "brake"
CONF_DIRECTION = "direction"
CONF_SPEED_PERCENT = "speed_percent"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_WATCHDOG_TICKLE = "watchdog_tickle"
CONF_TUNE_INITIAL_PARAMS = "tune_initial_params"
CONF_RUN_MPET = "run_mpet"
CONF_FAULT_ACTIVE = "fault_active"
CONF_SYS_ENABLE = "sys_enable"
CONF_CURRENT_FAULT = "current_fault"
CONF_VM_VOLTAGE = "vm_voltage"
CONF_DUTY_CMD_PERCENT = "duty_cmd_percent"
CONF_VOLT_MAG_PERCENT = "volt_mag_percent"
CONF_MOTOR_BEMF_CONSTANT = "motor_bemf_constant"
CONF_SPEED_FDBK_HZ = "speed_fdbk_hz"
CONF_SPEED_REF_OPEN_LOOP_HZ = "speed_ref_open_loop_hz"
CONF_FG_SPEED_FDBK_HZ = "fg_speed_fdbk_hz"

BRAKE_MODE_OPTIONS = {
    "hiz": 0,
    "recirculation": 1,
    "low_side_brake": 2,
    "active_spin_down": 4,
}

BRAKE_TIME_OPTIONS = {
    "1ms": 0,
    "5ms": 5,
    "10ms": 6,
    "50ms": 7,
    "100ms": 8,
    "250ms": 9,
    "500ms": 10,
    "1000ms": 11,
    "2500ms": 12,
    "5000ms": 13,
    "10000ms": 14,
    "15000ms": 15,
}

MODE_OPTIONS = {
    "align": 0,
    "double_align": 1,
    "ipd": 2,
    "slow_first_cycle": 3,
}

ALIGN_TIME_OPTIONS = {
    "10ms": 0,
    "50ms": 1,
    "100ms": 2,
    "200ms": 3,
    "300ms": 4,
    "400ms": 5,
    "500ms": 6,
    "750ms": 7,
    "1000ms": 8,
    "1500ms": 9,
    "2000ms": 10,
    "3000ms": 11,
    "4000ms": 12,
    "5000ms": 13,
    "7500ms": 14,
    "10000ms": 15,
}

DIRECTION_MODE_OPTIONS = {
    "hardware": "hardware",
    "cw": "cw",
    "ccw": "ccw",
}

CSA_GAIN_V_PER_V_TO_CODE = {
    5: 0,
    10: 1,
    20: 2,
    40: 3,
}

OPEN_LOOP_LIMIT_SOURCE_OPTIONS = {
    "ol_ilimit": 0,
    "ilimit": 1,
}

DIRECTION_OPTIONS = ["hardware", "cw", "ccw"]

LOCK_MODE_OPTIONS = {
    "latched": 0,
    "retry": 4,
    "disabled": 9,
}

LOCK_RETRY_TIME_OPTIONS = {
    "300ms": 0,
    "500ms": 1,
    "1s": 2,
    "2s": 3,
    "3s": 4,
    "4s": 5,
    "5s": 6,
    "6s": 7,
    "7s": 8,
    "8s": 9,
    "9s": 10,
    "10s": 11,
    "11s": 12,
    "12s": 13,
    "13s": 14,
    "14s": 15,
}

LOCK_ILIMIT_PERCENT_TO_CODE = {
    5: 0,
    10: 1,
    15: 2,
    20: 3,
    25: 4,
    30: 5,
    40: 6,
    50: 7,
    60: 8,
    65: 9,
    70: 10,
    75: 11,
    80: 12,
    85: 13,
    90: 14,
    95: 15,
}

LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE = {
    130: 0,
    140: 1,
    150: 2,
    160: 3,
    170: 4,
    180: 5,
    190: 6,
    200: 7,
}

ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE = {
    40.0: 0,
    45.0: 1,
    50.0: 2,
    55.0: 3,
    60.0: 4,
    65.0: 5,
    67.5: 6,
    70.0: 7,
}

NO_MOTOR_THRESHOLD_PERCENT_TO_CODE = {
    1.0: 0,
    2.0: 1,
    3.0: 2,
    4.0: 3,
    5.0: 4,
    7.5: 5,
    10.0: 6,
    20.0: 7,
}

OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE = {
    0.01: 0,
    0.05: 1,
    1.0: 2,
    2.5: 3,
    5.0: 4,
    10.0: 5,
    25.0: 6,
    50.0: 7,
    75.0: 8,
    100.0: 9,
    250.0: 10,
    500.0: 11,
    750.0: 12,
    1000.0: 13,
    5000.0: 14,
    10000.0: 15,
}

OPEN_LOOP_ACCEL2_HZ_PER_S2_TO_CODE = {
    0.0: 0,
    0.05: 1,
    1.0: 2,
    2.5: 3,
    5.0: 4,
    10.0: 5,
    25.0: 6,
    50.0: 7,
    75.0: 8,
    100.0: 9,
    250.0: 10,
    500.0: 11,
    750.0: 12,
    1000.0: 13,
    5000.0: 14,
    10000.0: 15,
}

OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE = {
    1.0: 0,
    2.0: 1,
    3.0: 2,
    4.0: 3,
    5.0: 4,
    6.0: 5,
    7.0: 6,
    8.0: 7,
    9.0: 8,
    10.0: 9,
    11.0: 10,
    12.0: 11,
    13.0: 12,
    14.0: 13,
    15.0: 14,
    16.0: 15,
    17.0: 16,
    18.0: 17,
    19.0: 18,
    20.0: 19,
    22.5: 20,
    25.0: 21,
    27.5: 22,
    30.0: 23,
    32.5: 24,
    35.0: 25,
    37.5: 26,
    40.0: 27,
    42.5: 28,
    45.0: 29,
    47.5: 30,
    50.0: 31,
}

THETA_ERROR_RAMP_RATE_TO_CODE = {
    0.01: 0,
    0.05: 1,
    0.1: 2,
    0.15: 3,
    0.2: 4,
    0.5: 5,
    1.0: 6,
    2.0: 7,
}

CL_SLOW_ACC_HZ_PER_S_TO_CODE = {
    0.1: 0,
    1.0: 1,
    2.0: 2,
    3.0: 3,
    5.0: 4,
    10.0: 5,
    20.0: 6,
    30.0: 7,
    40.0: 8,
    50.0: 9,
    100.0: 10,
    200.0: 11,
    500.0: 12,
    750.0: 13,
    1000.0: 14,
    2000.0: 15,
}

MPET_OPEN_LOOP_CURR_REF_PERCENT_TO_CODE = {
    10: 0,
    20: 1,
    30: 2,
    40: 3,
    50: 4,
    60: 5,
    70: 6,
    80: 7,
}

MPET_OPEN_LOOP_SPEED_REF_PERCENT_TO_CODE = {
    15: 0,
    25: 1,
    35: 2,
    50: 3,
}

MPET_OPEN_LOOP_SLEW_HZ_PER_S_TO_CODE = {
    0.1: 0,
    0.5: 1,
    1.0: 2,
    2.0: 3,
    3.0: 4,
    5.0: 5,
    10.0: 6,
    20.0: 7,
}

LOCK_ILIMIT_DEGLITCH_MS_TO_CODE = {
    0.0: 0,
    0.1: 1,
    0.2: 2,
    0.5: 3,
    1.0: 4,
    2.5: 5,
    5.0: 6,
    7.5: 7,
    10.0: 8,
    25.0: 9,
    50.0: 10,
    75.0: 11,
    100.0: 12,
    200.0: 13,
    500.0: 14,
    1000.0: 15,
}

HW_LOCK_ILIMIT_DEGLITCH_US_TO_CODE = {
    0: 0,
    1: 1,
    2: 2,
    3: 3,
    4: 4,
    5: 5,
    6: 6,
    7: 7,
}

LEGACY_STARTUP_KEY_MAP = {
    "startup_motor_bemf_const": CONF_MOTOR_BEMF_CONST,
    "startup_brake_mode": CONF_BRAKE_MODE_CFG,
    "startup_brake_time": CONF_BRAKE_TIME_CFG,
    "startup_mode": CONF_MODE_CFG,
    "startup_align_time": CONF_ALIGN_TIME,
    "startup_direction_mode": CONF_DIRECTION_MODE_CFG,
    "startup_csa_gain_v_per_v": CONF_CSA_GAIN_V_PER_V,
    "startup_base_current_amps": CONF_BASE_CURRENT_AMPS,
    "startup_align_or_slow_current_limit_percent": CONF_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT,
    "startup_lock_mode": CONF_LOCK_MODE_CFG,
    "startup_lock_ilimit_percent": CONF_LOCK_ILIMIT_PERCENT,
    "startup_hw_lock_ilimit_percent": CONF_HW_LOCK_ILIMIT_PERCENT,
    "startup_lock_retry_time": CONF_LOCK_RETRY_TIME,
    "startup_abn_speed_lock_enable": CONF_ABN_SPEED_LOCK_ENABLE,
    "startup_abn_bemf_lock_enable": CONF_ABN_BEMF_LOCK_ENABLE,
    "startup_no_motor_lock_enable": CONF_NO_MOTOR_LOCK_ENABLE,
    "startup_lock_abn_speed_threshold_percent": CONF_LOCK_ABN_SPEED_THRESHOLD_PERCENT,
    "startup_abnormal_bemf_threshold_percent": CONF_ABNORMAL_BEMF_THRESHOLD_PERCENT,
    "startup_no_motor_threshold_percent": CONF_NO_MOTOR_THRESHOLD_PERCENT,
    "startup_max_speed_hz": CONF_MAX_SPEED_HZ,
    "startup_open_loop_ilimit_percent": CONF_OPEN_LOOP_ILIMIT_PERCENT,
    "startup_open_loop_limit_source": CONF_OPEN_LOOP_LIMIT_SOURCE,
    "startup_open_loop_accel_hz_per_s": CONF_OPEN_LOOP_ACCEL_HZ_PER_S,
    "startup_auto_handoff_enable": CONF_AUTO_HANDOFF_ENABLE,
    "startup_open_to_closed_handoff_percent": CONF_OPEN_TO_CLOSED_HANDOFF_PERCENT,
}


def validate_int_from_map(value, value_map, label):
    value = cv.int_(value)
    if value not in value_map:
        raise cv.Invalid(
            f"{label} must be one of: " + ", ".join(str(v) for v in value_map)
        )
    return value


def validate_float_from_map(value, value_map, label):
    value = cv.float_(value)
    for allowed in value_map:
        if abs(value - allowed) < 1e-6:
            return allowed
    raise cv.Invalid(
        f"{label} must be one of: " + ", ".join(str(v) for v in value_map)
    )


def validate_max_speed_hz(value):
    value = cv.int_(value)
    if value < 1 or value > 3295:
        raise cv.Invalid("max speed must be in range 1..3295 Hz")
    return value


def validate_csa_gain_v_per_v(value):
    return validate_int_from_map(value, CSA_GAIN_V_PER_V_TO_CODE, "CSA gain")


def validate_base_current_amps(value):
    value = cv.float_(value)
    if value <= 0.0 or value > 1200.0:
        raise cv.Invalid("base current must be in range (0, 1200] amps")
    return value


def validate_open_loop_accel_hz_per_s(value):
    return validate_float_from_map(
        value,
        OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE,
        "open-loop accel",
    )


def validate_open_loop_accel2_hz_per_s2(value):
    return validate_float_from_map(
        value,
        OPEN_LOOP_ACCEL2_HZ_PER_S2_TO_CODE,
        "open-loop accel2",
    )


def validate_open_to_closed_handoff_percent(value):
    return validate_float_from_map(
        value,
        OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE,
        "open-to-closed handoff percent",
    )


def validate_theta_error_ramp_rate(value):
    return validate_float_from_map(
        value,
        THETA_ERROR_RAMP_RATE_TO_CODE,
        "theta_error_ramp_rate",
    )


def validate_cl_slow_acc_hz_per_s(value):
    return validate_float_from_map(
        value,
        CL_SLOW_ACC_HZ_PER_S_TO_CODE,
        "cl_slow_acc_hz_per_s",
    )


def validate_mpet_open_loop_curr_ref_percent(value):
    return validate_int_from_map(
        value,
        MPET_OPEN_LOOP_CURR_REF_PERCENT_TO_CODE,
        "mpet_open_loop_curr_ref_percent",
    )


def validate_mpet_open_loop_speed_ref_percent(value):
    return validate_int_from_map(
        value,
        MPET_OPEN_LOOP_SPEED_REF_PERCENT_TO_CODE,
        "mpet_open_loop_speed_ref_percent",
    )


def validate_mpet_open_loop_slew_hz_per_s(value):
    return validate_float_from_map(
        value,
        MPET_OPEN_LOOP_SLEW_HZ_PER_S_TO_CODE,
        "mpet_open_loop_slew_hz_per_s",
    )


def validate_lock_ilimit_deglitch_ms(value):
    return validate_float_from_map(
        value,
        LOCK_ILIMIT_DEGLITCH_MS_TO_CODE,
        "lock_ilimit_deglitch_ms",
    )


def validate_hw_lock_ilimit_deglitch_us(value):
    return validate_int_from_map(
        value,
        HW_LOCK_ILIMIT_DEGLITCH_US_TO_CODE,
        "hw_lock_ilimit_deglitch_us",
    )


def validate_lock_ilimit_percent(value):
    return validate_int_from_map(
        value, LOCK_ILIMIT_PERCENT_TO_CODE, "lock current-limit percent"
    )


def validate_lock_abn_speed_threshold_percent(value):
    return validate_int_from_map(
        value,
        LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE,
        "abnormal speed threshold percent",
    )


def validate_abnormal_bemf_threshold_percent(value):
    return validate_float_from_map(
        value,
        ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE,
        "abnormal BEMF threshold percent",
    )


def validate_no_motor_threshold_percent(value):
    return validate_float_from_map(
        value,
        NO_MOTOR_THRESHOLD_PERCENT_TO_CODE,
        "no-motor threshold percent",
    )


def validate_no_legacy_startup_keys(config):
    for old_key, new_key in LEGACY_STARTUP_KEY_MAP.items():
        if old_key in config:
            raise cv.Invalid(f"`{old_key}` has been removed. Use `{new_key}` instead.")
    return config


def validate_safety_guardrails(config):
    if config.get(CONF_ALLOW_UNSAFE_CURRENT_LIMITS, False):
        return config

    max_safe_percent = 50
    guarded_keys = (
        CONF_PHASE_CURRENT_LIMIT_PERCENT,
        CONF_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT,
        CONF_OPEN_LOOP_ILIMIT_PERCENT,
        CONF_LOCK_ILIMIT_PERCENT,
        CONF_HW_LOCK_ILIMIT_PERCENT,
    )
    for key in guarded_keys:
        if key in config and config[key] > max_safe_percent:
            raise cv.Invalid(
                f"{key}={config[key]} exceeds safety guardrail ({max_safe_percent}%). "
                "Set allow_unsafe_current_limits: true to override intentionally."
            )

    if config.get(CONF_LOCK_MODE_CFG) == LOCK_MODE_OPTIONS["disabled"]:
        raise cv.Invalid(
            "lock_mode=disabled is blocked by safety guardrail. "
            "Set allow_unsafe_current_limits: true to override intentionally."
        )

    return config


def validate_tuning_prerequisites(config):
    tuning_keys = (
        CONF_TUNE_INITIAL_PARAMS,
        CONF_OPEN_LOOP_ILIMIT_PERCENT,
        CONF_OPEN_LOOP_ACCEL_HZ_PER_S,
        CONF_OPEN_LOOP_ACCEL2_HZ_PER_S2,
        CONF_AUTO_HANDOFF_ENABLE,
        CONF_OPEN_TO_CLOSED_HANDOFF_PERCENT,
        CONF_THETA_ERROR_RAMP_RATE,
        CONF_CL_SLOW_ACC_HZ_PER_S,
        CONF_MPET_USE_DEDICATED_PARAMS,
        CONF_MPET_OPEN_LOOP_CURR_REF_PERCENT,
        CONF_MPET_OPEN_LOOP_SPEED_REF_PERCENT,
        CONF_MPET_OPEN_LOOP_SLEW_HZ_PER_S,
        CONF_LOCK_ILIMIT_PERCENT,
        CONF_HW_LOCK_ILIMIT_PERCENT,
        CONF_LOCK_ILIMIT_DEGLITCH_MS,
        CONF_HW_LOCK_ILIMIT_DEGLITCH_US,
        CONF_SPEED_LOOP_KP_CODE,
        CONF_SPEED_LOOP_KI_CODE,
    )
    tuning_requested = any(key in config for key in tuning_keys)
    if not tuning_requested:
        return config

    required_hardware_keys = (
        CONF_CSA_GAIN_V_PER_V,
        CONF_BASE_CURRENT_AMPS,
        CONF_PHASE_CURRENT_LIMIT_PERCENT,
        CONF_OPEN_LOOP_LIMIT_SOURCE,
        CONF_LOCK_MODE_CFG,
    )
    missing = [key for key in required_hardware_keys if key not in config]
    if missing:
        raise cv.Invalid(
            "tuning requires hardware baseline keys first; missing: " + ", ".join(missing)
        )

    if config.get(CONF_OPEN_LOOP_LIMIT_SOURCE) == OPEN_LOOP_LIMIT_SOURCE_OPTIONS["ol_ilimit"]:
        if CONF_OPEN_LOOP_ILIMIT_PERCENT not in config and CONF_TUNE_INITIAL_PARAMS not in config:
            raise cv.Invalid(
                "open_loop_limit_source=ol_ilimit requires open_loop_ilimit_percent for tuning"
            )

    return config


def encode_max_speed_hz(value_hz):
    if value_hz <= 1600:
        code = int(round(value_hz * 6.0))
        if code < 1:
            code = 1
        if code > 9600:
            code = 9600
        return code
    code = int(round((value_hz + 800.0) * 4.0))
    if code < 9601:
        code = 9601
    if code > 16383:
        code = 16383
    return code


def encode_base_current_amps(value_amps):
    code = int(round((value_amps * 32768.0) / 1200.0))
    if code < 1:
        code = 1
    if code > 0x7FFF:
        code = 0x7FFF
    return code


def encode_open_loop_limit_source(value):
    return value == OPEN_LOOP_LIMIT_SOURCE_OPTIONS["ilimit"]


RUNTIME_SETTER_SPECS = (
    (CONF_AUTO_TICKLE_WATCHDOG, "set_auto_tickle_watchdog", None),
    (CONF_CLEAR_MPET_ON_STARTUP, "set_clear_mpet_on_startup", None),
    (CONF_SPEED_RAMP_UP_PERCENT_PER_S, "set_speed_ramp_up_percent_per_s", None),
    (CONF_SPEED_RAMP_DOWN_PERCENT_PER_S, "set_speed_ramp_down_percent_per_s", None),
    (CONF_START_BOOST_PERCENT, "set_start_boost_percent", None),
    (CONF_START_BOOST_HOLD_MS, "set_start_boost_hold_ms", None),
)

REQUIRED_MOTOR_SETTER_SPECS = (
    (CONF_MOTOR_BEMF_CONST, "set_cfg_motor_bemf_const", None),
    (CONF_BRAKE_MODE_CFG, "set_cfg_brake_mode", None),
    (CONF_MODE_CFG, "set_cfg_mode", None),
    (CONF_MAX_SPEED_HZ, "set_cfg_max_speed_code", encode_max_speed_hz),
)

OPTIONAL_MOTOR_SETTER_SPECS = (
    (CONF_BRAKE_TIME_CFG, "set_cfg_brake_time", None),
    (CONF_ALIGN_TIME, "set_cfg_align_time", None),
    (CONF_DIRECTION_MODE_CFG, "set_cfg_direction_mode", None),
    (CONF_CSA_GAIN_V_PER_V, "set_cfg_csa_gain", lambda value: CSA_GAIN_V_PER_V_TO_CODE[value]),
    (CONF_BASE_CURRENT_AMPS, "set_cfg_base_current_code", encode_base_current_amps),
    (CONF_PHASE_CURRENT_LIMIT_PERCENT, "set_cfg_ilimit", lambda value: LOCK_ILIMIT_PERCENT_TO_CODE[value]),
    (
        CONF_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT,
        "set_cfg_align_or_slow_current_ilimit",
        lambda value: LOCK_ILIMIT_PERCENT_TO_CODE[value],
    ),
    (CONF_LOCK_MODE_CFG, "set_cfg_lock_mode", None),
    (CONF_LOCK_ILIMIT_PERCENT, "set_cfg_lock_ilimit", lambda value: LOCK_ILIMIT_PERCENT_TO_CODE[value]),
    (CONF_HW_LOCK_ILIMIT_PERCENT, "set_cfg_hw_lock_ilimit", lambda value: LOCK_ILIMIT_PERCENT_TO_CODE[value]),
    (CONF_LOCK_RETRY_TIME, "set_cfg_lock_retry_time", None),
    (CONF_ABN_SPEED_LOCK_ENABLE, "set_cfg_abn_speed_lock_enable", None),
    (CONF_ABN_BEMF_LOCK_ENABLE, "set_cfg_abn_bemf_lock_enable", None),
    (CONF_NO_MOTOR_LOCK_ENABLE, "set_cfg_no_motor_lock_enable", None),
    (
        CONF_LOCK_ABN_SPEED_THRESHOLD_PERCENT,
        "set_cfg_lock_abn_speed_threshold",
        lambda value: LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE[value],
    ),
    (
        CONF_ABNORMAL_BEMF_THRESHOLD_PERCENT,
        "set_cfg_abnormal_bemf_threshold",
        lambda value: ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE[value],
    ),
    (
        CONF_NO_MOTOR_THRESHOLD_PERCENT,
        "set_cfg_no_motor_threshold",
        lambda value: NO_MOTOR_THRESHOLD_PERCENT_TO_CODE[value],
    ),
    (CONF_OPEN_LOOP_ILIMIT_PERCENT, "set_cfg_open_loop_ilimit", lambda value: LOCK_ILIMIT_PERCENT_TO_CODE[value]),
    (CONF_OPEN_LOOP_LIMIT_SOURCE, "set_cfg_open_loop_limit_source", encode_open_loop_limit_source),
    (CONF_OPEN_LOOP_ACCEL_HZ_PER_S, "set_cfg_open_loop_accel", lambda value: OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE[value]),
    (
        CONF_OPEN_LOOP_ACCEL2_HZ_PER_S2,
        "set_cfg_open_loop_accel2",
        lambda value: OPEN_LOOP_ACCEL2_HZ_PER_S2_TO_CODE[value],
    ),
    (CONF_AUTO_HANDOFF_ENABLE, "set_cfg_auto_handoff_enable", None),
    (
        CONF_OPEN_TO_CLOSED_HANDOFF_PERCENT,
        "set_cfg_open_to_closed_handoff_threshold",
        lambda value: OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE[value],
    ),
    (CONF_THETA_ERROR_RAMP_RATE, "set_cfg_theta_error_ramp_rate", lambda value: THETA_ERROR_RAMP_RATE_TO_CODE[value]),
    (CONF_CL_SLOW_ACC_HZ_PER_S, "set_cfg_cl_slow_acc", lambda value: CL_SLOW_ACC_HZ_PER_S_TO_CODE[value]),
    (CONF_MPET_USE_DEDICATED_PARAMS, "set_cfg_mpet_use_dedicated_params", None),
    (
        CONF_MPET_OPEN_LOOP_CURR_REF_PERCENT,
        "set_cfg_mpet_open_loop_curr_ref",
        lambda value: MPET_OPEN_LOOP_CURR_REF_PERCENT_TO_CODE[value],
    ),
    (
        CONF_MPET_OPEN_LOOP_SPEED_REF_PERCENT,
        "set_cfg_mpet_open_loop_speed_ref",
        lambda value: MPET_OPEN_LOOP_SPEED_REF_PERCENT_TO_CODE[value],
    ),
    (
        CONF_MPET_OPEN_LOOP_SLEW_HZ_PER_S,
        "set_cfg_mpet_open_loop_slew",
        lambda value: MPET_OPEN_LOOP_SLEW_HZ_PER_S_TO_CODE[value],
    ),
    (CONF_MPET_TIMEOUT_MS, "set_mpet_timeout_ms", None),
    (
        CONF_LOCK_ILIMIT_DEGLITCH_MS,
        "set_cfg_lock_ilimit_deglitch",
        lambda value: LOCK_ILIMIT_DEGLITCH_MS_TO_CODE[value],
    ),
    (
        CONF_HW_LOCK_ILIMIT_DEGLITCH_US,
        "set_cfg_hw_lock_ilimit_deglitch",
        lambda value: HW_LOCK_ILIMIT_DEGLITCH_US_TO_CODE[value],
    ),
    (CONF_SPEED_LOOP_KP_CODE, "set_cfg_speed_loop_kp_code", None),
    (CONF_SPEED_LOOP_KI_CODE, "set_cfg_speed_loop_ki_code", None),
)

BUTTON_CONFIG_KEYS = (
    CONF_CLEAR_FAULTS,
    CONF_WATCHDOG_TICKLE,
    CONF_TUNE_INITIAL_PARAMS,
    CONF_RUN_MPET,
)

BINARY_SENSOR_SETTER_SPECS = (
    (CONF_FAULT_ACTIVE, "set_fault_active_binary_sensor"),
    (CONF_SYS_ENABLE, "set_sys_enable_binary_sensor"),
)

TEXT_SENSOR_SETTER_SPECS = (
    (CONF_CURRENT_FAULT, "set_current_fault_text_sensor"),
)

SENSOR_SETTER_SPECS = (
    (CONF_VM_VOLTAGE, "set_vm_voltage_sensor"),
    (CONF_DUTY_CMD_PERCENT, "set_duty_cmd_percent_sensor"),
    (CONF_VOLT_MAG_PERCENT, "set_volt_mag_percent_sensor"),
    (CONF_MOTOR_BEMF_CONSTANT, "set_motor_bemf_constant_sensor"),
    (CONF_SPEED_FDBK_HZ, "set_speed_fdbk_hz_sensor"),
    (CONF_SPEED_REF_OPEN_LOOP_HZ, "set_speed_ref_open_loop_hz_sensor"),
    (CONF_FG_SPEED_FDBK_HZ, "set_fg_speed_fdbk_hz_sensor"),
)


def apply_codegen_setters(var, config, setter_specs, optional):
    for conf_key, setter_name, transform in setter_specs:
        if optional and conf_key not in config:
            continue
        value = config[conf_key]
        if transform is not None:
            value = transform(value)
        cg.add(getattr(var, setter_name)(value))


async def attach_optional_entities(var, config, entity_specs, factory):
    for conf_key, setter_name in entity_specs:
        if conf_key not in config:
            continue
        entity = await factory(config[conf_key])
        cg.add(getattr(var, setter_name)(entity))


CONFIG_SCHEMA = cv.All(
    validate_no_legacy_startup_keys,
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8329AComponent),
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_CLEAR_MPET_ON_STARTUP, default=True): cv.boolean,
            cv.Optional(CONF_SPEED_RAMP_UP_PERCENT_PER_S, default=0.0): cv.float_range(min=0.0),
            cv.Optional(CONF_SPEED_RAMP_DOWN_PERCENT_PER_S, default=0.0): cv.float_range(min=0.0),
            cv.Optional(CONF_START_BOOST_PERCENT, default=0.0): cv.float_range(min=0.0, max=100.0),
            cv.Optional(CONF_START_BOOST_HOLD_MS, default=0): cv.int_range(min=0),
            cv.Optional(CONF_ALLOW_UNSAFE_CURRENT_LIMITS, default=False): cv.boolean,
            cv.Required(CONF_MOTOR_BEMF_CONST): cv.int_range(min=1, max=255),
            cv.Required(CONF_BRAKE_MODE_CFG): cv.enum(BRAKE_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_BRAKE_TIME_CFG): cv.enum(BRAKE_TIME_OPTIONS, lower=True),
            cv.Required(CONF_MODE_CFG): cv.enum(MODE_OPTIONS, lower=True),
            cv.Optional(CONF_ALIGN_TIME): cv.enum(ALIGN_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_DIRECTION_MODE_CFG): cv.enum(DIRECTION_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_CSA_GAIN_V_PER_V): validate_csa_gain_v_per_v,
            cv.Optional(CONF_BASE_CURRENT_AMPS): validate_base_current_amps,
            cv.Optional(CONF_PHASE_CURRENT_LIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_LOCK_MODE_CFG): cv.enum(LOCK_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_LOCK_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_HW_LOCK_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_LOCK_RETRY_TIME): cv.enum(LOCK_RETRY_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_ABN_SPEED_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_ABN_BEMF_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_NO_MOTOR_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_LOCK_ABN_SPEED_THRESHOLD_PERCENT): validate_lock_abn_speed_threshold_percent,
            cv.Optional(CONF_ABNORMAL_BEMF_THRESHOLD_PERCENT): validate_abnormal_bemf_threshold_percent,
            cv.Optional(CONF_NO_MOTOR_THRESHOLD_PERCENT): validate_no_motor_threshold_percent,
            cv.Required(CONF_MAX_SPEED_HZ): validate_max_speed_hz,
            cv.Optional(CONF_OPEN_LOOP_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_OPEN_LOOP_LIMIT_SOURCE): cv.enum(
                OPEN_LOOP_LIMIT_SOURCE_OPTIONS, lower=True
            ),
            cv.Optional(CONF_OPEN_LOOP_ACCEL_HZ_PER_S): validate_open_loop_accel_hz_per_s,
            cv.Optional(CONF_OPEN_LOOP_ACCEL2_HZ_PER_S2): validate_open_loop_accel2_hz_per_s2,
            cv.Optional(CONF_AUTO_HANDOFF_ENABLE): cv.boolean,
            cv.Optional(CONF_OPEN_TO_CLOSED_HANDOFF_PERCENT): validate_open_to_closed_handoff_percent,
            cv.Optional(CONF_THETA_ERROR_RAMP_RATE): validate_theta_error_ramp_rate,
            cv.Optional(CONF_CL_SLOW_ACC_HZ_PER_S): validate_cl_slow_acc_hz_per_s,
            cv.Optional(CONF_MPET_USE_DEDICATED_PARAMS): cv.boolean,
            cv.Optional(CONF_MPET_OPEN_LOOP_CURR_REF_PERCENT): validate_mpet_open_loop_curr_ref_percent,
            cv.Optional(CONF_MPET_OPEN_LOOP_SPEED_REF_PERCENT): validate_mpet_open_loop_speed_ref_percent,
            cv.Optional(CONF_MPET_OPEN_LOOP_SLEW_HZ_PER_S): validate_mpet_open_loop_slew_hz_per_s,
            cv.Optional(CONF_MPET_TIMEOUT_MS): cv.int_range(min=1000, max=600000),
            cv.Optional(CONF_LOCK_ILIMIT_DEGLITCH_MS): validate_lock_ilimit_deglitch_ms,
            cv.Optional(CONF_HW_LOCK_ILIMIT_DEGLITCH_US): validate_hw_lock_ilimit_deglitch_us,
            cv.Optional(CONF_SPEED_LOOP_KP_CODE): cv.int_range(min=0, max=1023),
            cv.Optional(CONF_SPEED_LOOP_KI_CODE): cv.int_range(min=0, max=1023),
            cv.Optional(CONF_BRAKE): switch_.switch_schema(
                MCF8329ABrakeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_DIRECTION): select.select_schema(
                MCF8329ADirectionSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_SPEED_PERCENT): number.number_schema(
                MCF8329ASpeedNumber,
                unit_of_measurement=UNIT_PERCENT,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(
                MCF8329AClearFaultsButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG_TICKLE): button.button_schema(
                MCF8329AWatchdogTickleButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_TUNE_INITIAL_PARAMS): button.button_schema(
                MCF8329ATuneInitialParamsButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_RUN_MPET): button.button_schema(
                MCF8329ARunMPETButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_FAULT_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_SYS_ENABLE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_CURRENT_FAULT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_VM_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DUTY_CMD_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLT_MAG_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MOTOR_BEMF_CONSTANT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SPEED_FDBK_HZ): sensor.sensor_schema(
                unit_of_measurement="Hz",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SPEED_REF_OPEN_LOOP_HZ): sensor.sensor_schema(
                unit_of_measurement="Hz",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FG_SPEED_FDBK_HZ): sensor.sensor_schema(
                unit_of_measurement="Hz",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("250ms"))
    .extend(i2c.i2c_device_schema(default_address=0x01)),
    validate_tuning_prerequisites,
    validate_safety_guardrails,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add_build_flag("-DMCF8329A_EMBED_IMPL")

    apply_codegen_setters(var, config, RUNTIME_SETTER_SPECS, optional=False)
    apply_codegen_setters(var, config, REQUIRED_MOTOR_SETTER_SPECS, optional=False)
    apply_codegen_setters(var, config, OPTIONAL_MOTOR_SETTER_SPECS, optional=True)

    if CONF_BRAKE in config:
        sw = await switch_.new_switch(config[CONF_BRAKE])
        cg.add(sw.set_parent(var))
        cg.add(var.set_brake_switch(sw))

    if CONF_DIRECTION in config:
        sel = await select.new_select(config[CONF_DIRECTION], options=DIRECTION_OPTIONS)
        cg.add(sel.set_parent(var))
        cg.add(var.set_direction_select(sel))

    if CONF_SPEED_PERCENT in config:
        num = await number.new_number(
            config[CONF_SPEED_PERCENT],
            min_value=0,
            max_value=100,
            step=1,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_speed_number(num))

    for conf_key in BUTTON_CONFIG_KEYS:
        if conf_key not in config:
            continue
        btn = await button.new_button(config[conf_key])
        cg.add(btn.set_parent(var))

    await attach_optional_entities(
        var,
        config,
        BINARY_SENSOR_SETTER_SPECS,
        binary_sensor.new_binary_sensor,
    )
    await attach_optional_entities(
        var,
        config,
        TEXT_SENSOR_SETTER_SPECS,
        text_sensor.new_text_sensor,
    )
    await attach_optional_entities(
        var,
        config,
        SENSOR_SETTER_SPECS,
        sensor.new_sensor,
    )
