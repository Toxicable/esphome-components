import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

esc_higher_ns = cg.esphome_ns.namespace("esc_higher")
ESCHigherComponent = esc_higher_ns.class_(
    "ESCHigherComponent", cg.PollingComponent, i2c.I2CDevice
)

CONF_TEMPERATURE_C = "temperature_c"
CONF_STATUS = "status"
CONF_FAULT = "fault"
CONF_MOTOR_STATE = "motor_state"
CONF_CURRENT_FAULT = "current_fault"
CONF_OCCURRED_FAULT = "occurred_fault"
CONF_MEASURED_SPEED_RPM = "measured_speed_rpm"
CONF_SPEED_REFERENCE_RPM = "speed_reference_rpm"
CONF_CONTROL_MODE = "control_mode"
CONF_COMMAND_STATE = "command_state"
CONF_IA = "ia"
CONF_IB = "ib"
CONF_PHASE_CURRENT_AMPLITUDE = "phase_current_amplitude"
CONF_IQ = "iq"
CONF_ID_CURRENT = "id_current"
CONF_IQ_REF = "iq_ref"
CONF_VQ = "vq"
CONF_VD = "vd"
CONF_PHASE_VOLTAGE_AMPLITUDE = "phase_voltage_amplitude"
CONF_BUS_VOLTAGE = "bus_voltage"
CONF_ELECTRICAL_ANGLE = "electrical_angle"
CONF_VALPHA = "valpha"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESCHigherComponent),
            cv.Optional(CONF_TEMPERATURE_C): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_STATUS): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FAULT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MOTOR_STATE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT_FAULT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_OCCURRED_FAULT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MEASURED_SPEED_RPM): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SPEED_REFERENCE_RPM): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CONTROL_MODE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_COMMAND_STATE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IA): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IB): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PHASE_CURRENT_AMPLITUDE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IQ): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ID_CURRENT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IQ_REF): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VQ): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VD): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PHASE_VOLTAGE_AMPLITUDE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BUS_VOLTAGE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ELECTRICAL_ANGLE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VALPHA): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x43))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_TEMPERATURE_C in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_C])
        cg.add(var.set_temperature_c_sensor(sens))

    if CONF_STATUS in config:
        sens = await sensor.new_sensor(config[CONF_STATUS])
        cg.add(var.set_status_sensor(sens))

    if CONF_FAULT in config:
        sens = await sensor.new_sensor(config[CONF_FAULT])
        cg.add(var.set_fault_sensor(sens))

    if CONF_MOTOR_STATE in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR_STATE])
        cg.add(var.set_motor_state_sensor(sens))

    if CONF_CURRENT_FAULT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_FAULT])
        cg.add(var.set_current_fault_sensor(sens))

    if CONF_OCCURRED_FAULT in config:
        sens = await sensor.new_sensor(config[CONF_OCCURRED_FAULT])
        cg.add(var.set_occurred_fault_sensor(sens))

    if CONF_MEASURED_SPEED_RPM in config:
        sens = await sensor.new_sensor(config[CONF_MEASURED_SPEED_RPM])
        cg.add(var.set_measured_speed_rpm_sensor(sens))

    if CONF_SPEED_REFERENCE_RPM in config:
        sens = await sensor.new_sensor(config[CONF_SPEED_REFERENCE_RPM])
        cg.add(var.set_speed_reference_rpm_sensor(sens))

    if CONF_CONTROL_MODE in config:
        sens = await sensor.new_sensor(config[CONF_CONTROL_MODE])
        cg.add(var.set_control_mode_sensor(sens))

    if CONF_COMMAND_STATE in config:
        sens = await sensor.new_sensor(config[CONF_COMMAND_STATE])
        cg.add(var.set_command_state_sensor(sens))

    if CONF_IA in config:
        sens = await sensor.new_sensor(config[CONF_IA])
        cg.add(var.set_ia_sensor(sens))

    if CONF_IB in config:
        sens = await sensor.new_sensor(config[CONF_IB])
        cg.add(var.set_ib_sensor(sens))

    if CONF_PHASE_CURRENT_AMPLITUDE in config:
        sens = await sensor.new_sensor(config[CONF_PHASE_CURRENT_AMPLITUDE])
        cg.add(var.set_phase_current_amplitude_sensor(sens))

    if CONF_IQ in config:
        sens = await sensor.new_sensor(config[CONF_IQ])
        cg.add(var.set_iq_sensor(sens))

    if CONF_ID_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_ID_CURRENT])
        cg.add(var.set_id_sensor(sens))

    if CONF_IQ_REF in config:
        sens = await sensor.new_sensor(config[CONF_IQ_REF])
        cg.add(var.set_iq_ref_sensor(sens))

    if CONF_VQ in config:
        sens = await sensor.new_sensor(config[CONF_VQ])
        cg.add(var.set_vq_sensor(sens))

    if CONF_VD in config:
        sens = await sensor.new_sensor(config[CONF_VD])
        cg.add(var.set_vd_sensor(sens))

    if CONF_PHASE_VOLTAGE_AMPLITUDE in config:
        sens = await sensor.new_sensor(config[CONF_PHASE_VOLTAGE_AMPLITUDE])
        cg.add(var.set_phase_voltage_amplitude_sensor(sens))

    if CONF_BUS_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BUS_VOLTAGE])
        cg.add(var.set_bus_voltage_sensor(sens))

    if CONF_ELECTRICAL_ANGLE in config:
        sens = await sensor.new_sensor(config[CONF_ELECTRICAL_ANGLE])
        cg.add(var.set_electrical_angle_sensor(sens))

    if CONF_VALPHA in config:
        sens = await sensor.new_sensor(config[CONF_VALPHA])
        cg.add(var.set_valpha_sensor(sens))
