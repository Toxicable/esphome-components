import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import DEVICE_CLASS_VOLTAGE, STATE_CLASS_MEASUREMENT, UNIT_PERCENT, UNIT_VOLT

from . import MCF8329AComponent

CONF_MCF8329A_ID = "mcf8329a_id"
CONF_VM_VOLTAGE = "vm_voltage"
CONF_DUTY_CMD_PERCENT = "duty_cmd_percent"
CONF_VOLT_MAG_PERCENT = "volt_mag_percent"
CONF_MOTOR_BEMF_CONSTANT = "motor_bemf_constant"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
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


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    if CONF_VM_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VM_VOLTAGE])
        cg.add(parent.set_vm_voltage_sensor(sens))

    if CONF_DUTY_CMD_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_DUTY_CMD_PERCENT])
        cg.add(parent.set_duty_cmd_percent_sensor(sens))

    if CONF_VOLT_MAG_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_VOLT_MAG_PERCENT])
        cg.add(parent.set_volt_mag_percent_sensor(sens))

    if CONF_MOTOR_BEMF_CONSTANT in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR_BEMF_CONSTANT])
        cg.add(parent.set_motor_bemf_constant_sensor(sens))
