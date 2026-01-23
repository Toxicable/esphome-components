import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.components.ledc import output as ledc_output
from esphome.const import CONF_ID
from esphome import pins

from . import DRV8243Output

CONF_OUT1 = "out1"
CONF_LEDC = "ledc"
CONF_NSLEEP_PIN = "nsleep_pin"
CONF_NFAULT_PIN = "nfault_pin"
CONF_OUT2_PIN = "out2_pin"
CONF_FLIP_POLARITY = "flip_polarity"
CONF_MIN_LEVEL = "min_level"
CONF_EXPONENT = "exponent"

def _validate_out1_config(config):
    if CONF_OUT1 in config and CONF_LEDC in config:
        raise cv.Invalid("Specify only one of out1 or ledc")
    if CONF_OUT1 not in config and CONF_LEDC not in config:
        raise cv.Invalid("Specify either out1 or ledc")
    return config


CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(DRV8243Output),

            cv.Optional(CONF_OUT1): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_LEDC): ledc_output.CONFIG_SCHEMA,

            cv.Required(CONF_NSLEEP_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_NFAULT_PIN): pins.gpio_input_pin_schema,

            cv.Optional(CONF_OUT2_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_FLIP_POLARITY, default=False): cv.boolean,

            cv.Optional(CONF_MIN_LEVEL, default=0.014): cv.percentage,
            cv.Optional(CONF_EXPONENT, default=1.8): cv.float_range(min=0.1, max=5.0),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _validate_out1_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)

    if CONF_LEDC in config:
        ledc_config = config[CONF_LEDC]
        await ledc_output.to_code(ledc_config)
        out1 = await cg.get_variable(ledc_config[CONF_ID])
    else:
        out1 = await cg.get_variable(config[CONF_OUT1])
    cg.add(var.set_out1_output(out1))

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
