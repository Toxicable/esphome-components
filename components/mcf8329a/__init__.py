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
CONF_APPLY_STARTUP_CONFIG = "apply_startup_config"
CONF_STARTUP_BRAKE_MODE = "startup_brake_mode"
CONF_STARTUP_BRAKE_TIME = "startup_brake_time"
CONF_STARTUP_MODE = "startup_mode"
CONF_STARTUP_ALIGN_TIME = "startup_align_time"
CONF_STARTUP_DIRECTION_MODE = "startup_direction_mode"

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

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8329AComponent),
            cv.Optional(CONF_INTER_BYTE_DELAY_US, default=100): cv.positive_int,
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_APPLY_STARTUP_CONFIG, default=True): cv.boolean,
            cv.Optional(CONF_STARTUP_BRAKE_MODE): cv.enum(STARTUP_BRAKE_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_BRAKE_TIME): cv.enum(STARTUP_BRAKE_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_MODE): cv.enum(STARTUP_MODE_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_ALIGN_TIME): cv.enum(STARTUP_ALIGN_TIME_OPTIONS, lower=True),
            cv.Optional(CONF_STARTUP_DIRECTION_MODE): cv.enum(STARTUP_DIRECTION_MODE_OPTIONS, lower=True),
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
    cg.add(var.set_apply_startup_config(config[CONF_APPLY_STARTUP_CONFIG]))

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
