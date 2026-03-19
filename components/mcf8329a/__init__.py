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

CONF_INTER_BYTE_DELAY_US = "inter_byte_delay_us"
CONF_AUTO_TICKLE_WATCHDOG = "auto_tickle_watchdog"
CONF_CLEAR_MPET_ON_STARTUP = "clear_mpet_on_startup"
CONF_ALLOW_UNSAFE_CURRENT_LIMITS = "allow_unsafe_current_limits"
CONF_STARTUP_MOTOR_BEMF_CONST = "startup_motor_bemf_const"
CONF_STARTUP_BRAKE_MODE = "startup_brake_mode"
CONF_STARTUP_BRAKE_TIME = "startup_brake_time"
CONF_STARTUP_MODE = "startup_mode"
CONF_STARTUP_ALIGN_TIME = "startup_align_time"
CONF_STARTUP_DIRECTION_MODE = "startup_direction_mode"
CONF_STARTUP_CSA_GAIN_V_PER_V = "startup_csa_gain_v_per_v"
CONF_STARTUP_BASE_CURRENT_AMPS = "startup_base_current_amps"
CONF_PHASE_CURRENT_LIMIT_PERCENT = "phase_current_limit_percent"
CONF_STARTUP_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT = "startup_align_or_slow_current_limit_percent"
CONF_STARTUP_LOCK_MODE = "startup_lock_mode"
CONF_STARTUP_LOCK_ILIMIT_PERCENT = "startup_lock_ilimit_percent"
CONF_STARTUP_HW_LOCK_ILIMIT_PERCENT = "startup_hw_lock_ilimit_percent"
CONF_STARTUP_LOCK_RETRY_TIME = "startup_lock_retry_time"
CONF_STARTUP_ABN_SPEED_LOCK_ENABLE = "startup_abn_speed_lock_enable"
CONF_STARTUP_ABN_BEMF_LOCK_ENABLE = "startup_abn_bemf_lock_enable"
CONF_STARTUP_NO_MOTOR_LOCK_ENABLE = "startup_no_motor_lock_enable"
CONF_STARTUP_LOCK_ABN_SPEED_THRESHOLD_PERCENT = "startup_lock_abn_speed_threshold_percent"
CONF_STARTUP_ABNORMAL_BEMF_THRESHOLD_PERCENT = "startup_abnormal_bemf_threshold_percent"
CONF_STARTUP_NO_MOTOR_THRESHOLD_PERCENT = "startup_no_motor_threshold_percent"
CONF_STARTUP_MAX_SPEED_HZ = "startup_max_speed_hz"
CONF_STARTUP_OPEN_LOOP_ILIMIT_PERCENT = "startup_open_loop_ilimit_percent"
CONF_STARTUP_OPEN_LOOP_LIMIT_SOURCE = "startup_open_loop_limit_source"
CONF_STARTUP_OPEN_LOOP_ACCEL_HZ_PER_S = "startup_open_loop_accel_hz_per_s"
CONF_STARTUP_AUTO_HANDOFF_ENABLE = "startup_auto_handoff_enable"
CONF_STARTUP_OPEN_TO_CLOSED_HANDOFF_PERCENT = "startup_open_to_closed_handoff_percent"

CONF_BRAKE = "brake"
CONF_DIRECTION = "direction"
CONF_SPEED_PERCENT = "speed_percent"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_WATCHDOG_TICKLE = "watchdog_tickle"
CONF_FAULT_ACTIVE = "fault_active"
CONF_SYS_ENABLE = "sys_enable"
CONF_CURRENT_FAULT = "current_fault"
CONF_VM_VOLTAGE = "vm_voltage"
CONF_DUTY_CMD_PERCENT = "duty_cmd_percent"
CONF_VOLT_MAG_PERCENT = "volt_mag_percent"
CONF_MOTOR_BEMF_CONSTANT = "motor_bemf_constant"

STARTUP_BRAKE_MODE_OPTIONS = {
    "hiz": 0,
    "recirculation": 1,
    "low_side_brake": 2,
    "active_spin_down": 4,
}

STARTUP_BRAKE_TIME_OPTIONS = {
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

STARTUP_MODE_OPTIONS = {
    "align": 0,
    "double_align": 1,
    "ipd": 2,
    "slow_first_cycle": 3,
}

STARTUP_ALIGN_TIME_OPTIONS = {
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

STARTUP_DIRECTION_MODE_OPTIONS = {
    "hardware": "hardware",
    "cw": "cw",
    "ccw": "ccw",
}

STARTUP_CSA_GAIN_V_PER_V_TO_CODE = {
    5: 0,
    10: 1,
    20: 2,
    40: 3,
}

STARTUP_OPEN_LOOP_LIMIT_SOURCE_OPTIONS = {
    "ol_ilimit": 0,
    "ilimit": 1,
}

DIRECTION_OPTIONS = ["hardware", "cw", "ccw"]

STARTUP_LOCK_MODE_OPTIONS = {
    "latched": 0,
    "retry": 4,
    "disabled": 9,
}

STARTUP_LOCK_RETRY_TIME_OPTIONS = {
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


def validate_lock_ilimit_percent(value):
    value = cv.int_(value)
    if value not in LOCK_ILIMIT_PERCENT_TO_CODE:
        raise cv.Invalid(
            "lock current-limit percent must be one of: "
            + ", ".join(str(v) for v in LOCK_ILIMIT_PERCENT_TO_CODE)
        )
    return value


def validate_lock_abn_speed_threshold_percent(value):
    value = cv.int_(value)
    if value not in LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE:
        raise cv.Invalid(
            "abnormal speed threshold percent must be one of: "
            + ", ".join(str(v) for v in LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE)
        )
    return value


def validate_abnormal_bemf_threshold_percent(value):
    value = cv.float_(value)
    for allowed in ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE:
        if abs(value - allowed) < 1e-6:
            return allowed
    raise cv.Invalid(
        "abnormal BEMF threshold percent must be one of: "
        + ", ".join(str(v) for v in ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE)
    )


def validate_no_motor_threshold_percent(value):
    value = cv.float_(value)
    for allowed in NO_MOTOR_THRESHOLD_PERCENT_TO_CODE:
        if abs(value - allowed) < 1e-6:
            return allowed
    raise cv.Invalid(
        "no-motor threshold percent must be one of: "
        + ", ".join(str(v) for v in NO_MOTOR_THRESHOLD_PERCENT_TO_CODE)
    )


def validate_startup_max_speed_hz(value):
    value = cv.int_(value)
    if value < 1 or value > 3295:
        raise cv.Invalid("startup max speed must be in range 1..3295 Hz")
    return value


def validate_startup_csa_gain_v_per_v(value):
    value = cv.int_(value)
    if value not in STARTUP_CSA_GAIN_V_PER_V_TO_CODE:
        raise cv.Invalid(
            "startup CSA gain must be one of: "
            + ", ".join(str(v) for v in STARTUP_CSA_GAIN_V_PER_V_TO_CODE)
        )
    return value


def validate_startup_base_current_amps(value):
    value = cv.float_(value)
    if value <= 0.0 or value > 1200.0:
        raise cv.Invalid("startup base current must be in range (0, 1200] amps")
    return value


def validate_open_loop_accel_hz_per_s(value):
    value = cv.float_(value)
    for allowed in OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE:
        if abs(value - allowed) < 1e-6:
            return allowed
    raise cv.Invalid(
        "open-loop accel must be one of: "
        + ", ".join(str(v) for v in OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE)
    )


def validate_open_to_closed_handoff_percent(value):
    value = cv.float_(value)
    for allowed in OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE:
        if abs(value - allowed) < 1e-6:
            return allowed
    raise cv.Invalid(
        "open-to-closed handoff percent must be one of: "
        + ", ".join(str(v) for v in OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE)
    )


def validate_safety_guardrails(config):
    if config.get(CONF_ALLOW_UNSAFE_CURRENT_LIMITS, False):
        return config

    max_safe_percent = 50
    guarded_keys = (
        CONF_PHASE_CURRENT_LIMIT_PERCENT,
        CONF_STARTUP_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT,
        CONF_STARTUP_OPEN_LOOP_ILIMIT_PERCENT,
        CONF_STARTUP_LOCK_ILIMIT_PERCENT,
        CONF_STARTUP_HW_LOCK_ILIMIT_PERCENT,
    )
    for key in guarded_keys:
        if key in config and config[key] > max_safe_percent:
            raise cv.Invalid(
                f"{key}={config[key]} exceeds safety guardrail ({max_safe_percent}%). "
                "Set allow_unsafe_current_limits: true to override intentionally."
            )

    if config.get(CONF_STARTUP_LOCK_MODE) == "disabled":
        raise cv.Invalid(
            "startup_lock_mode=disabled is blocked by safety guardrail. "
            "Set allow_unsafe_current_limits: true to override intentionally."
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


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8329AComponent),
            cv.Optional(CONF_INTER_BYTE_DELAY_US, default=100): cv.positive_int,
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_CLEAR_MPET_ON_STARTUP, default=True): cv.boolean,
            cv.Optional(CONF_ALLOW_UNSAFE_CURRENT_LIMITS, default=False): cv.boolean,
            cv.Required(CONF_STARTUP_MOTOR_BEMF_CONST): cv.int_range(min=1, max=255),
            cv.Required(CONF_STARTUP_BRAKE_MODE): cv.enum(STARTUP_BRAKE_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_BRAKE_TIME): cv.enum(STARTUP_BRAKE_TIME_OPTIONS, lower=True),
            cv.Required(CONF_STARTUP_MODE): cv.enum(STARTUP_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_ALIGN_TIME): cv.enum(STARTUP_ALIGN_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_DIRECTION_MODE): cv.enum(STARTUP_DIRECTION_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_CSA_GAIN_V_PER_V): validate_startup_csa_gain_v_per_v,
            cv.Optional(CONF_STARTUP_BASE_CURRENT_AMPS): validate_startup_base_current_amps,
            cv.Optional(CONF_PHASE_CURRENT_LIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_STARTUP_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_STARTUP_LOCK_MODE): cv.enum(STARTUP_LOCK_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_LOCK_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_STARTUP_HW_LOCK_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_STARTUP_LOCK_RETRY_TIME): cv.enum(STARTUP_LOCK_RETRY_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_ABN_SPEED_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_STARTUP_ABN_BEMF_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_STARTUP_NO_MOTOR_LOCK_ENABLE): cv.boolean,
            cv.Optional(CONF_STARTUP_LOCK_ABN_SPEED_THRESHOLD_PERCENT): validate_lock_abn_speed_threshold_percent,
            cv.Optional(CONF_STARTUP_ABNORMAL_BEMF_THRESHOLD_PERCENT): validate_abnormal_bemf_threshold_percent,
            cv.Optional(CONF_STARTUP_NO_MOTOR_THRESHOLD_PERCENT): validate_no_motor_threshold_percent,
            cv.Required(CONF_STARTUP_MAX_SPEED_HZ): validate_startup_max_speed_hz,
            cv.Optional(CONF_STARTUP_OPEN_LOOP_ILIMIT_PERCENT): validate_lock_ilimit_percent,
            cv.Optional(CONF_STARTUP_OPEN_LOOP_LIMIT_SOURCE): cv.enum(
                STARTUP_OPEN_LOOP_LIMIT_SOURCE_OPTIONS, lower=True
            ),
            cv.Optional(CONF_STARTUP_OPEN_LOOP_ACCEL_HZ_PER_S): validate_open_loop_accel_hz_per_s,
            cv.Optional(CONF_STARTUP_AUTO_HANDOFF_ENABLE): cv.boolean,
            cv.Optional(CONF_STARTUP_OPEN_TO_CLOSED_HANDOFF_PERCENT): validate_open_to_closed_handoff_percent,
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
        }
    )
    .extend(cv.polling_component_schema("250ms"))
    .extend(i2c.i2c_device_schema(default_address=0x01)),
    validate_safety_guardrails,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_inter_byte_delay_us(config[CONF_INTER_BYTE_DELAY_US]))
    cg.add(var.set_auto_tickle_watchdog(config[CONF_AUTO_TICKLE_WATCHDOG]))
    cg.add(var.set_clear_mpet_on_startup(config[CONF_CLEAR_MPET_ON_STARTUP]))

    cg.add(var.set_startup_motor_bemf_const(config[CONF_STARTUP_MOTOR_BEMF_CONST]))
    cg.add(var.set_startup_brake_mode(config[CONF_STARTUP_BRAKE_MODE]))
    if CONF_STARTUP_BRAKE_TIME in config:
        cg.add(var.set_startup_brake_time(config[CONF_STARTUP_BRAKE_TIME]))
    cg.add(var.set_startup_mode(config[CONF_STARTUP_MODE]))
    if CONF_STARTUP_ALIGN_TIME in config:
        cg.add(var.set_startup_align_time(config[CONF_STARTUP_ALIGN_TIME]))
    if CONF_STARTUP_DIRECTION_MODE in config:
        cg.add(var.set_startup_direction_mode(config[CONF_STARTUP_DIRECTION_MODE]))
    if CONF_STARTUP_CSA_GAIN_V_PER_V in config:
        cg.add(var.set_startup_csa_gain(STARTUP_CSA_GAIN_V_PER_V_TO_CODE[config[CONF_STARTUP_CSA_GAIN_V_PER_V]]))
    if CONF_STARTUP_BASE_CURRENT_AMPS in config:
        cg.add(var.set_startup_base_current_code(encode_base_current_amps(config[CONF_STARTUP_BASE_CURRENT_AMPS])))
    if CONF_PHASE_CURRENT_LIMIT_PERCENT in config:
        cg.add(var.set_startup_ilimit(LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_PHASE_CURRENT_LIMIT_PERCENT]]))
    if CONF_STARTUP_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT in config:
        cg.add(
            var.set_startup_align_or_slow_current_ilimit(
                LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_STARTUP_ALIGN_OR_SLOW_CURRENT_LIMIT_PERCENT]]
            )
        )
    if CONF_STARTUP_LOCK_MODE in config:
        cg.add(var.set_startup_lock_mode(config[CONF_STARTUP_LOCK_MODE]))
    if CONF_STARTUP_LOCK_ILIMIT_PERCENT in config:
        cg.add(var.set_startup_lock_ilimit(LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_STARTUP_LOCK_ILIMIT_PERCENT]]))
    if CONF_STARTUP_HW_LOCK_ILIMIT_PERCENT in config:
        cg.add(var.set_startup_hw_lock_ilimit(LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_STARTUP_HW_LOCK_ILIMIT_PERCENT]]))
    if CONF_STARTUP_LOCK_RETRY_TIME in config:
        cg.add(var.set_startup_lock_retry_time(config[CONF_STARTUP_LOCK_RETRY_TIME]))
    if CONF_STARTUP_ABN_SPEED_LOCK_ENABLE in config:
        cg.add(var.set_startup_abn_speed_lock_enable(config[CONF_STARTUP_ABN_SPEED_LOCK_ENABLE]))
    if CONF_STARTUP_ABN_BEMF_LOCK_ENABLE in config:
        cg.add(var.set_startup_abn_bemf_lock_enable(config[CONF_STARTUP_ABN_BEMF_LOCK_ENABLE]))
    if CONF_STARTUP_NO_MOTOR_LOCK_ENABLE in config:
        cg.add(var.set_startup_no_motor_lock_enable(config[CONF_STARTUP_NO_MOTOR_LOCK_ENABLE]))
    if CONF_STARTUP_LOCK_ABN_SPEED_THRESHOLD_PERCENT in config:
        cg.add(
            var.set_startup_lock_abn_speed_threshold(
                LOCK_ABN_SPEED_THRESHOLD_PERCENT_TO_CODE[config[CONF_STARTUP_LOCK_ABN_SPEED_THRESHOLD_PERCENT]]
            )
        )
    if CONF_STARTUP_ABNORMAL_BEMF_THRESHOLD_PERCENT in config:
        cg.add(
            var.set_startup_abnormal_bemf_threshold(
                ABNORMAL_BEMF_THRESHOLD_PERCENT_TO_CODE[config[CONF_STARTUP_ABNORMAL_BEMF_THRESHOLD_PERCENT]]
            )
        )
    if CONF_STARTUP_NO_MOTOR_THRESHOLD_PERCENT in config:
        cg.add(
            var.set_startup_no_motor_threshold(
                NO_MOTOR_THRESHOLD_PERCENT_TO_CODE[config[CONF_STARTUP_NO_MOTOR_THRESHOLD_PERCENT]]
            )
        )
    cg.add(var.set_startup_max_speed_code(encode_max_speed_hz(config[CONF_STARTUP_MAX_SPEED_HZ])))
    if CONF_STARTUP_OPEN_LOOP_ILIMIT_PERCENT in config:
        cg.add(var.set_startup_open_loop_ilimit(LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_STARTUP_OPEN_LOOP_ILIMIT_PERCENT]]))
    if CONF_STARTUP_OPEN_LOOP_LIMIT_SOURCE in config:
        cg.add(var.set_startup_open_loop_limit_source(config[CONF_STARTUP_OPEN_LOOP_LIMIT_SOURCE] == 1))
    if CONF_STARTUP_OPEN_LOOP_ACCEL_HZ_PER_S in config:
        cg.add(var.set_startup_open_loop_accel(OPEN_LOOP_ACCEL_HZ_PER_S_TO_CODE[config[CONF_STARTUP_OPEN_LOOP_ACCEL_HZ_PER_S]]))
    if CONF_STARTUP_AUTO_HANDOFF_ENABLE in config:
        cg.add(var.set_startup_auto_handoff_enable(config[CONF_STARTUP_AUTO_HANDOFF_ENABLE]))
    if CONF_STARTUP_OPEN_TO_CLOSED_HANDOFF_PERCENT in config:
        cg.add(
            var.set_startup_open_to_closed_handoff_threshold(
                OPEN_TO_CLOSED_HANDOFF_PERCENT_TO_CODE[config[CONF_STARTUP_OPEN_TO_CLOSED_HANDOFF_PERCENT]]
            )
        )

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

    if CONF_CLEAR_FAULTS in config:
        clear_faults = await button.new_button(config[CONF_CLEAR_FAULTS])
        cg.add(clear_faults.set_parent(var))

    if CONF_WATCHDOG_TICKLE in config:
        watchdog_tickle = await button.new_button(config[CONF_WATCHDOG_TICKLE])
        cg.add(watchdog_tickle.set_parent(var))

    if CONF_FAULT_ACTIVE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_FAULT_ACTIVE])
        cg.add(var.set_fault_active_binary_sensor(sens))

    if CONF_SYS_ENABLE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_SYS_ENABLE])
        cg.add(var.set_sys_enable_binary_sensor(sens))

    if CONF_CURRENT_FAULT in config:
        sens = await text_sensor.new_text_sensor(config[CONF_CURRENT_FAULT])
        cg.add(var.set_current_fault_text_sensor(sens))

    if CONF_VM_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VM_VOLTAGE])
        cg.add(var.set_vm_voltage_sensor(sens))

    if CONF_DUTY_CMD_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_DUTY_CMD_PERCENT])
        cg.add(var.set_duty_cmd_percent_sensor(sens))

    if CONF_VOLT_MAG_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_VOLT_MAG_PERCENT])
        cg.add(var.set_volt_mag_percent_sensor(sens))

    if CONF_MOTOR_BEMF_CONSTANT in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR_BEMF_CONSTANT])
        cg.add(var.set_motor_bemf_constant_sensor(sens))
