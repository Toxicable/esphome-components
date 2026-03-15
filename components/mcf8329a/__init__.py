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


def validate_lock_ilimit_percent(value):
    value = cv.int_(value)
    if value not in LOCK_ILIMIT_PERCENT_TO_CODE:
        raise cv.Invalid(
            "lock current-limit percent must be one of: "
            + ", ".join(str(v) for v in LOCK_ILIMIT_PERCENT_TO_CODE)
        )
    return value


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
