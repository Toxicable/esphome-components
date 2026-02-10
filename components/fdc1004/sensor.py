import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT

DEPENDENCIES = ["i2c"]

fdc1004_ns = cg.esphome_ns.namespace("fdc1004")
FDC1004Component = fdc1004_ns.class_("FDC1004Component", cg.PollingComponent, i2c.I2CDevice)

CONF_SAMPLE_RATE = "sample_rate"
CONF_CAPDAC = "capdac"
CONF_CIN1 = "cin1"
CONF_CIN2 = "cin2"
CONF_CIN3 = "cin3"
CONF_CIN4 = "cin4"

CHANNEL_KEYS = (CONF_CIN1, CONF_CIN2, CONF_CIN3, CONF_CIN4)


def _channel_schema():
    return sensor.sensor_schema(
        unit_of_measurement="pF",
        accuracy_decimals=4,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend(
        {
            cv.Optional(CONF_CAPDAC, default=0): cv.int_range(min=0, max=31),
        }
    )


def _validate_sample_rate(value):
    if isinstance(value, str):
        normalized = value.strip().lower()
        aliases = {
            "100sps": 100,
            "200sps": 200,
            "400sps": 400,
        }
        if normalized in aliases:
            return aliases[normalized]

    value = cv.int_(value)
    if value not in (100, 200, 400):
        raise cv.Invalid("sample_rate must be 100, 200, or 400 S/s")
    return value


def _validate_config(config):
    if not any(key in config for key in CHANNEL_KEYS):
        raise cv.Invalid("At least one channel sensor (cin1, cin2, cin3, cin4) must be configured")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(FDC1004Component),
            cv.Optional(CONF_SAMPLE_RATE, default=100): _validate_sample_rate,
            cv.Optional(CONF_CIN1): _channel_schema(),
            cv.Optional(CONF_CIN2): _channel_schema(),
            cv.Optional(CONF_CIN3): _channel_schema(),
            cv.Optional(CONF_CIN4): _channel_schema(),
        }
    )
    .extend(cv.polling_component_schema("200ms"))
    .extend(i2c.i2c_device_schema(default_address=0x50)),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))

    for channel_index, channel_key in enumerate(CHANNEL_KEYS):
        if channel_key not in config:
            continue

        channel_config = config[channel_key]
        sens = await sensor.new_sensor(channel_config)
        cg.add(var.set_channel_sensor(channel_index, sens))
        cg.add(var.set_channel_capdac(channel_index, channel_config[CONF_CAPDAC]))
