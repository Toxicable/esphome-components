import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.components.ledc import output as ledc_output
from esphome.const import CONF_ID
from esphome import pins

from . import DRV8243Output

DEPENDENCIES = ["ledc"]
AUTO_LOAD = ["ledc"]

CONF_CH1 = "ch1"
CONF_CH2 = "ch2"
CONF_NSLEEP_PIN = "nsleep_pin"
CONF_NFAULT_PIN = "nfault_pin"
CONF_OUT2_PIN = "out2_pin"
CONF_FLIP_POLARITY = "flip_polarity"
CONF_MIN_LEVEL = "min_level"
CONF_EXPONENT = "exponent"


def _validate_config(config):
    has_ch2_output = CONF_CH2 in config
    has_out2_pin = CONF_OUT2_PIN in config

    if has_ch2_output and has_out2_pin:
        raise cv.Invalid("ch2 and out2_pin are mutually exclusive")
    if not has_ch2_output and not has_out2_pin:
        raise cv.Invalid("out2_pin is required when ch2 is not provided")

    return config


def _channel_schema():
    return cv.Any(ledc_output.CONFIG_SCHEMA, cv.use_id(output.FloatOutput))


CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(DRV8243Output),
            cv.Required(CONF_CH1): _channel_schema(),
            cv.Optional(CONF_CH2): _channel_schema(),
            cv.Required(CONF_NSLEEP_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_NFAULT_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_OUT2_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_FLIP_POLARITY, default=False): cv.boolean,
            cv.Optional(CONF_MIN_LEVEL, default=0.014): cv.percentage,
            cv.Optional(CONF_EXPONENT, default=1.8): cv.float_range(min=0.1, max=5.0),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _validate_config,
)


async def _build_ledc_output(ledc_config):
    cg.add_define("USE_LEDC")
    cg.add_define("USE_LEDC_OUTPUT")
    cg.add_global(cg.RawExpression('#include "esphome/components/ledc/ledc_output.h"'))
    await ledc_output.to_code(ledc_config)
    return await cg.get_variable(ledc_config[CONF_ID])


async def _resolve_channel(conf):
    if isinstance(conf, dict):
        return await _build_ledc_output(conf)
    return await cg.get_variable(conf)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)

    ch1 = await _resolve_channel(config[CONF_CH1])
    cg.add(var.set_out1_output(ch1))
    cg.add(var.set_out1_component(ch1))

    if CONF_CH2 in config:
        ch2 = await _resolve_channel(config[CONF_CH2])
        cg.add(var.set_out2_output(ch2))
        cg.add(var.set_out2_component(ch2))

    nsleep = await cg.gpio_pin_expression(config[CONF_NSLEEP_PIN])
    cg.add(var.set_nsleep_pin(nsleep))

    if CONF_NFAULT_PIN in config:
        nfault = await cg.gpio_pin_expression(config[CONF_NFAULT_PIN])
        cg.add(var.set_nfault_pin(nfault))

    if CONF_OUT2_PIN in config:
        out2 = await cg.gpio_pin_expression(config[CONF_OUT2_PIN])
        cg.add(var.set_out2_pin(out2))

    cg.add(var.set_flip_polarity(config[CONF_FLIP_POLARITY]))
    cg.add(var.set_min_level(config[CONF_MIN_LEVEL]))
    cg.add(var.set_exponent(config[CONF_EXPONENT]))
