import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, output
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

mcp4726_ns = cg.esphome_ns.namespace("mcp4726")
MCP4726Output = mcp4726_ns.class_(
    "MCP4726Output", cg.Component, output.FloatOutput, i2c.I2CDevice
)

CONF_VREF = "vref"
CONF_GAIN = "gain"
CONF_POWER_DOWN = "power_down"
CONF_ZERO_ON_BOOT = "zero_on_boot"

# MCP47x6 VREF1:VREF0 config bits.
VREFS = {
    "vdd": 0b00,                 # 0x = VDD, use 00
    "vref_unbuffered": 0b10,     # 10 = VREF pin, unbuffered
    "vref_buffered": 0b11,       # 11 = VREF pin, buffered
}

GAINS = {
    "1x": 0,
    "2x": 1,
}

POWER_DOWNS = {
    "normal": 0b00,
    "1k_to_ground": 0b01,
    "125k_to_ground": 0b10,
    "640k_to_ground": 0b11
}


def _validate(config):
    if config[CONF_VREF] == VREFS["vdd"] and config[CONF_GAIN] == GAINS["2x"]:
        raise cv.Invalid("gain: 2x is only valid when vref is vref_unbuffered or vref_buffered")
    return config


CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.Required(CONF_ID): cv.declare_id(MCP4726Output),
            cv.Optional(CONF_VREF, default="vref_buffered"): cv.enum(VREFS, lower=True),
            cv.Optional(CONF_GAIN, default="1x"): cv.enum(GAINS, lower=True),
            cv.Optional(CONF_POWER_DOWN, default="normal"): cv.enum(POWER_DOWNS, lower=True),
            cv.Optional(CONF_ZERO_ON_BOOT, default=True): cv.boolean,
        }
    ).extend(i2c.i2c_device_schema(0x60)),
    _validate,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_vref(config[CONF_VREF]))
    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_power_down(config[CONF_POWER_DOWN]))
    cg.add(var.set_zero_on_boot(config[CONF_ZERO_ON_BOOT]))
