import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor", "switch", "number", "select", "button", "text_sensor"]

mcf8329a_ns = cg.esphome_ns.namespace("mcf8329a")
MCF8329AComponent = mcf8329a_ns.class_("MCF8329AComponent", cg.PollingComponent, i2c.I2CDevice)

CONF_INTER_BYTE_DELAY_US = "inter_byte_delay_us"
CONF_AUTO_TICKLE_WATCHDOG = "auto_tickle_watchdog"
CONF_CLEAR_MPET_ON_STARTUP = "clear_mpet_on_startup"
CONF_APPLY_STARTUP_CONFIG = "apply_startup_config"
CONF_STARTUP_MOTOR_BEMF_CONST = "startup_motor_bemf_const"
CONF_STARTUP_BRAKE_MODE = "startup_brake_mode"
CONF_STARTUP_BRAKE_TIME = "startup_brake_time"
CONF_STARTUP_MODE = "startup_mode"
CONF_STARTUP_ALIGN_TIME = "startup_align_time"
CONF_STARTUP_DIRECTION_MODE = "startup_direction_mode"
CONF_STARTUP_ILIMIT_PERCENT = "startup_ilimit_percent"
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

STARTUP_LOCK_MODE_OPTIONS = {
    "latched": 0,
    "retry": 4,
    "report_only": 8,
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


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8329AComponent),
            cv.Optional(CONF_INTER_BYTE_DELAY_US, default=100): cv.positive_int,
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_CLEAR_MPET_ON_STARTUP, default=True): cv.boolean,
            cv.Optional(CONF_APPLY_STARTUP_CONFIG, default=True): cv.boolean,
            cv.Optional(CONF_STARTUP_MOTOR_BEMF_CONST): cv.int_range(min=1, max=255),
            cv.Optional(CONF_STARTUP_BRAKE_MODE): cv.enum(STARTUP_BRAKE_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_BRAKE_TIME): cv.enum(STARTUP_BRAKE_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_MODE): cv.enum(STARTUP_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_ALIGN_TIME): cv.enum(STARTUP_ALIGN_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_DIRECTION_MODE): cv.enum(STARTUP_DIRECTION_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_ILIMIT_PERCENT): validate_lock_ilimit_percent,
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
            cv.Optional(CONF_STARTUP_MAX_SPEED_HZ): validate_startup_max_speed_hz,
        }
    )
    .extend(cv.polling_component_schema("250ms"))
    .extend(i2c.i2c_device_schema(default_address=0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_inter_byte_delay_us(config[CONF_INTER_BYTE_DELAY_US]))
    cg.add(var.set_auto_tickle_watchdog(config[CONF_AUTO_TICKLE_WATCHDOG]))
    cg.add(var.set_clear_mpet_on_startup(config[CONF_CLEAR_MPET_ON_STARTUP]))
    cg.add(var.set_apply_startup_config(config[CONF_APPLY_STARTUP_CONFIG]))

    if CONF_STARTUP_MOTOR_BEMF_CONST in config:
        cg.add(var.set_startup_motor_bemf_const(config[CONF_STARTUP_MOTOR_BEMF_CONST]))
    if CONF_STARTUP_BRAKE_MODE in config:
        cg.add(var.set_startup_brake_mode(config[CONF_STARTUP_BRAKE_MODE]))
    if CONF_STARTUP_BRAKE_TIME in config:
        cg.add(var.set_startup_brake_time(config[CONF_STARTUP_BRAKE_TIME]))
    if CONF_STARTUP_MODE in config:
        cg.add(var.set_startup_mode(config[CONF_STARTUP_MODE]))
    if CONF_STARTUP_ALIGN_TIME in config:
        cg.add(var.set_startup_align_time(config[CONF_STARTUP_ALIGN_TIME]))
    if CONF_STARTUP_DIRECTION_MODE in config:
        cg.add(var.set_startup_direction_mode(config[CONF_STARTUP_DIRECTION_MODE]))
    if CONF_STARTUP_ILIMIT_PERCENT in config:
        cg.add(var.set_startup_ilimit(LOCK_ILIMIT_PERCENT_TO_CODE[config[CONF_STARTUP_ILIMIT_PERCENT]]))
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
    if CONF_STARTUP_MAX_SPEED_HZ in config:
        cg.add(var.set_startup_max_speed_code(encode_max_speed_hz(config[CONF_STARTUP_MAX_SPEED_HZ])))
