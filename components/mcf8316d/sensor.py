import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import DEVICE_CLASS_VOLTAGE, STATE_CLASS_MEASUREMENT, UNIT_PERCENT, UNIT_VOLT

from . import MCF8316DComponent

CONF_MCF8316D_ID = "mcf8316d_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8316D_ID): cv.use_id(MCF8316DComponent),
        cv.Optional("vm_voltage"): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("duty_cmd_percent"): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("volt_mag_percent"): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8316D_ID])

    if "vm_voltage" in config:
        sens = await sensor.new_sensor(config["vm_voltage"])
        cg.add(parent.set_vm_voltage_sensor(sens))

    if "duty_cmd_percent" in config:
        sens = await sensor.new_sensor(config["duty_cmd_percent"])
        cg.add(parent.set_duty_cmd_percent_sensor(sens))

    if "volt_mag_percent" in config:
        sens = await sensor.new_sensor(config["volt_mag_percent"])
        cg.add(parent.set_volt_mag_percent_sensor(sens))
