import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.components.ledc import output as ledc_output
from esphome.const import CONF_FREQUENCY, CONF_ID, CONF_PIN
from esphome import pins

from . import DRV8243ChannelOutput, DRV8243Output

DEPENDENCIES = ["ledc"]
AUTO_LOAD = ["ledc"]

CONF_CH1 = "ch1"
CONF_CH1_ID = "ch1_id"
CONF_CH2 = "ch2"
CONF_CH2_ID = "ch2_id"
CONF_NSLEEP_PIN = "nsleep_pin"
CONF_NFAULT_PIN = "nfault_pin"
CONF_OUT2_PIN = "out2_pin"
CONF_FLIP_POLARITY = "flip_polarity"
CONF_MIN_LEVEL = "min_level"
CONF_EXPONENT = "exponent"


def _validate_config(config):
    has_ch2_output = CONF_CH2 in config
    has_out2_pin = CONF_OUT2_PIN in config
    has_ch2_id = CONF_CH2_ID in config
    has_ch1_id = CONF_CH1_ID in config

    if has_ch2_output and has_out2_pin:
        raise cv.Invalid("ch2 and out2_pin are mutually exclusive")
    if not has_ch2_output and not has_out2_pin:
        raise cv.Invalid("out2_pin is required when ch2 is not provided")
    if has_ch2_id and not has_ch2_output:
        raise cv.Invalid("ch2_id requires ch2 to be provided")
    if has_ch1_id and CONF_CH1 not in config:
        raise cv.Invalid("ch1_id requires ch1 to be provided")

    return config


def _internal_ledc_schema():
    return cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ledc_output.LEDCOutput),
            cv.Required(CONF_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_FREQUENCY, default="20kHz"): cv.frequency,
        }
    )


def _channel_schema():
    return cv.Any(_internal_ledc_schema(), cv.use_id(output.FloatOutput))


CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(DRV8243Output),
            cv.Required(CONF_CH1): _channel_schema(),
            cv.Optional(CONF_CH2): _channel_schema(),
            cv.Optional(CONF_CH1_ID): cv.declare_id(DRV8243ChannelOutput),
            cv.Optional(CONF_CH2_ID): cv.declare_id(DRV8243ChannelOutput),
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
    if CONF_CH1_ID in config:
        ch1_var = cg.new_Pvariable(config[CONF_CH1_ID])
        ch1_config = dict(config)
        ch1_config[CONF_ID] = config[CONF_CH1_ID]
        await cg.register_component(ch1_var, ch1_config)
        await output.register_output(ch1_var, ch1_config)
        cg.add(ch1_var.set_parent(var))
        cg.add(ch1_var.set_channel(1))
        cg.add(var.set_ch1_output(ch1_var))

    if CONF_CH2 in config:
        ch2 = await _resolve_channel(config[CONF_CH2])
        cg.add(var.set_out2_output(ch2))
        cg.add(var.set_out2_component(ch2))
        if CONF_CH2_ID in config:
            ch2_var = cg.new_Pvariable(config[CONF_CH2_ID])
            ch2_config = dict(config)
            ch2_config[CONF_ID] = config[CONF_CH2_ID]
            await cg.register_component(ch2_var, ch2_config)
            await output.register_output(ch2_var, ch2_config)
            cg.add(ch2_var.set_parent(var))
            cg.add(ch2_var.set_channel(2))
            cg.add(var.set_ch2_output(ch2_var))

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
