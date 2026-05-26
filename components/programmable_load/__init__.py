import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, number, output, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_AMPERE,
    UNIT_MILLIVOLT,
)
AUTO_LOAD = ["binary_sensor", "number", "sensor", "text_sensor"]
programmable_load_ns = cg.esphome_ns.namespace("programmable_load")
ProgrammableLoadComponent = programmable_load_ns.class_(
    "ProgrammableLoadComponent", cg.Component
)
ProgrammableLoadSetpointNumber = programmable_load_ns.class_(
    "ProgrammableLoadSetpointNumber", number.Number
)

# --- Configuration keys ---
CONF_DAC_OUTPUT = "dac_output"
CONF_FAN_OUTPUT = "fan_output"
CONF_CURRENT_SENSOR = "current_sensor"
CONF_VOLTAGE_SENSOR = "voltage_sensor"
CONF_TEMPERATURE_SENSORS = "temperature_sensors"
CONF_NTC_PRESENT_SENSORS = "ntc_present_sensors"

CONF_MAX_CURRENT_A = "max_current_a"
CONF_VOLTAGE_MIN_V = "voltage_min_v"
CONF_MAX_TEMP_C = "max_temp_c"
CONF_CONTROL_PERIOD_MS = "control_period_ms"

CONF_DEADBAND_A = "deadband_a"
CONF_MAX_UNCONFIRMED_RISE_A = "max_unconfirmed_rise_a"
CONF_MAX_UNCONFIRMED_FALL_A = "max_unconfirmed_fall_a"
CONF_RAMP_FAST_A_PER_S = "ramp_fast_a_per_s"
CONF_RAMP_MEDIUM_A_PER_S = "ramp_medium_a_per_s"

CONF_FAN_START_TEMP_C = "fan_start_temp_c"
CONF_FAN_FULL_TEMP_C = "fan_full_temp_c"

CONF_SETPPOINT = "setpoint"
CONF_DCR = "dcr"
CONF_VOLTAGE_DROP = "voltage_drop"
CONF_CURRENT_DELTA = "current_delta"
CONF_RAMP_STATE = "ramp_state"
CONF_FAULT_NTC_MISSING = "fault_ntc_missing"
CONF_FAULT_NO_VOLTAGE = "fault_no_voltage"
CONF_FAULT_OVER_TEMP = "fault_over_temp"

# --- Validation ---
def _validate_positive_float(value):
    value = cv.float_(value)
    if value <= 0:
        raise cv.invalid(f"Value must be positive, got {value}")
    return value


def _validate_non_negative_float(value):
    value = cv.float_(value)
    if value < 0:
        raise cv.invalid(f"Value must be non-negative, got {value}")
    return value


def _validate_config(config):
    if config[CONF_FAN_FULL_TEMP_C] <= config[CONF_FAN_START_TEMP_C]:
        raise cv.Invalid("fan_full_temp_c must be greater than fan_start_temp_c")

    ntc_present_sensors = config[CONF_NTC_PRESENT_SENSORS]
    if len(ntc_present_sensors) > 1:
        raise cv.Invalid(
            "Only the first NTC present sensor is used for safety; extra entries are ignored"
        )

    return config


# --- Main config schema ---
CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(ProgrammableLoadComponent),

        # Required external references.
        cv.Required(CONF_DAC_OUTPUT): cv.use_id(output.FloatOutput),
        cv.Required(CONF_FAN_OUTPUT): cv.use_id(output.FloatOutput),
        cv.Required(CONF_CURRENT_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_VOLTAGE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_TEMPERATURE_SENSORS): cv.All(
            cv.ensure_list(cv.use_id(sensor.Sensor)),
            cv.Length(min=1),
        ),
        cv.Optional(CONF_NTC_PRESENT_SENSORS, default=[]): cv.ensure_list(
            cv.use_id(binary_sensor.BinarySensor)
        ),

        # Tunables with defaults.
        cv.Optional(CONF_MAX_CURRENT_A, default=40.0): _validate_positive_float,
        cv.Optional(CONF_VOLTAGE_MIN_V, default=1.0): _validate_positive_float,
        cv.Optional(CONF_MAX_TEMP_C, default=100.0): _validate_positive_float,
        cv.Optional(CONF_CONTROL_PERIOD_MS, default=50): cv.All(
            cv.int_, cv.Range(min=10, max=1000)
        ),

        cv.Optional(CONF_DEADBAND_A, default=0.010): _validate_non_negative_float,
        cv.Optional(CONF_MAX_UNCONFIRMED_RISE_A, default=1.0): _validate_positive_float,
        cv.Optional(CONF_MAX_UNCONFIRMED_FALL_A, default=2.0): _validate_positive_float,
        cv.Optional(CONF_RAMP_FAST_A_PER_S, default=8.0): _validate_positive_float,
        cv.Optional(CONF_RAMP_MEDIUM_A_PER_S, default=4.0): _validate_positive_float,

        cv.Optional(CONF_FAN_START_TEMP_C, default=35.0): cv.float_,
        cv.Optional(CONF_FAN_FULL_TEMP_C, default=65.0): cv.float_,

        # Generated entities (optional).
        cv.Optional(CONF_SETPPOINT): number.number_schema(
            ProgrammableLoadSetpointNumber,
            unit_of_measurement=UNIT_AMPERE,
            device_class=DEVICE_CLASS_CURRENT,
        ),
        cv.Optional(CONF_DCR): sensor.sensor_schema(
            unit_of_measurement="mΩ",
            accuracy_decimals=1,
        ),
        cv.Optional(CONF_VOLTAGE_DROP): sensor.sensor_schema(
            unit_of_measurement=UNIT_MILLIVOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
        ),
        cv.Optional(CONF_CURRENT_DELTA): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_CURRENT,
        ),
        cv.Optional(CONF_RAMP_STATE): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FAULT_NTC_MISSING): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FAULT_NO_VOLTAGE): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FAULT_OVER_TEMP): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }).extend(cv.COMPONENT_SCHEMA),
    _validate_config,
)


async def to_code(config):
    # Create the main component.
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Wire external references.
    dac = await cg.get_variable(config[CONF_DAC_OUTPUT])
    cg.add(var.set_dac_output(dac))

    fan = await cg.get_variable(config[CONF_FAN_OUTPUT])
    cg.add(var.set_fan_output(fan))

    cur = await cg.get_variable(config[CONF_CURRENT_SENSOR])
    cg.add(var.set_current_sensor(cur))

    volt = await cg.get_variable(config[CONF_VOLTAGE_SENSOR])
    cg.add(var.set_voltage_sensor(volt))

    for ts_id in config[CONF_TEMPERATURE_SENSORS]:
        ts = await cg.get_variable(ts_id)
        cg.add(var.add_temperature_sensor(ts))

    for bs_id in config.get(CONF_NTC_PRESENT_SENSORS, []):
        bs = await cg.get_variable(bs_id)
        cg.add(var.add_ntc_present_sensor(bs))

    # Apply tunables.
    cg.add(var.set_max_current_a(config[CONF_MAX_CURRENT_A]))
    cg.add(var.set_voltage_min_v(config[CONF_VOLTAGE_MIN_V]))
    cg.add(var.set_max_temp_c(config[CONF_MAX_TEMP_C]))
    cg.add(var.set_control_period_ms(config[CONF_CONTROL_PERIOD_MS]))

    cg.add(var.set_deadband_a(config[CONF_DEADBAND_A]))
    cg.add(var.set_max_unconfirmed_rise_a(config[CONF_MAX_UNCONFIRMED_RISE_A]))
    cg.add(var.set_max_unconfirmed_fall_a(config[CONF_MAX_UNCONFIRMED_FALL_A]))
    cg.add(var.set_ramp_fast_a_per_s(config[CONF_RAMP_FAST_A_PER_S]))
    cg.add(var.set_ramp_medium_a_per_s(config[CONF_RAMP_MEDIUM_A_PER_S]))

    cg.add(var.set_fan_start_temp_c(config[CONF_FAN_START_TEMP_C]))
    cg.add(var.set_fan_full_temp_c(config[CONF_FAN_FULL_TEMP_C]))

    # Generate setpoint number.
    if CONF_SETPPOINT in config:
        num = await number.new_number(
            config[CONF_SETPPOINT],
            min_value=0.0,
            max_value=config[CONF_MAX_CURRENT_A],
            step=0.1,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_setpoint_number(num))

    # Generate DCR sensor.
    if CONF_DCR in config:
        sens = await sensor.new_sensor(config[CONF_DCR])
        cg.add(var.set_dcr_sensor(sens))

    # Generate voltage drop sensor.
    if CONF_VOLTAGE_DROP in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE_DROP])
        cg.add(var.set_voltage_drop_sensor(sens))

    # Generate current delta sensor.
    if CONF_CURRENT_DELTA in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_DELTA])
        cg.add(var.set_current_delta_sensor(sens))

    if CONF_RAMP_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_RAMP_STATE])
        cg.add(var.set_ramp_state_sensor(ts))

    # Generate fault binary sensors.
    if CONF_FAULT_NTC_MISSING in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FAULT_NTC_MISSING])
        cg.add(var.set_fault_ntc_missing_sensor(bs))

    if CONF_FAULT_NO_VOLTAGE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FAULT_NO_VOLTAGE])
        cg.add(var.set_fault_no_voltage_sensor(bs))

    if CONF_FAULT_OVER_TEMP in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FAULT_OVER_TEMP])
        cg.add(var.set_fault_over_temp_sensor(bs))
