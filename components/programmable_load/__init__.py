import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, number, output, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_AMPERE,
)

AUTO_LOAD = ["button", "number", "sensor", "text_sensor"]

programmable_load_ns = cg.esphome_ns.namespace("programmable_load")

ProgrammableLoadComponent = programmable_load_ns.class_(
    "ProgrammableLoadComponent", cg.Component
)
ProgrammableLoadManualCurrentNumber = programmable_load_ns.class_(
    "ProgrammableLoadManualCurrentNumber", number.Number
)
ProgrammableLoadClearFaultButton = programmable_load_ns.class_(
    "ProgrammableLoadClearFaultButton", button.Button
)
ProgrammableLoadDcrTest = programmable_load_ns.class_(
    "ProgrammableLoadDcrTest"
)
ProgrammableLoadDcrStartButton = programmable_load_ns.class_(
    "ProgrammableLoadDcrStartButton", button.Button
)

CONF_OUTPUT = "output"
CONF_DAC = "dac"
CONF_DAC_FULL_SCALE_CURRENT = "dac_full_scale_current"

CONF_MEASUREMENTS = "measurements"
CONF_CURRENT = "current"
CONF_VOLTAGE = "voltage"
CONF_TEMPERATURES = "temperatures"
CONF_SENSOR = "sensor"
CONF_REQUIRED = "required"
CONF_SAMPLE_TIMEOUT = "sample_timeout"

CONF_LIMITS = "limits"
CONF_MAXIMUM_CURRENT = "maximum_current"
CONF_MINIMUM_VOLTAGE = "minimum_voltage"
CONF_MAXIMUM_VOLTAGE = "maximum_voltage"
CONF_MAXIMUM_POWER = "maximum_power"
CONF_MAXIMUM_TEMPERATURE = "maximum_temperature"

CONF_CONTROL = "control"
CONF_PERIOD = "period"
CONF_DEADBAND = "deadband"
CONF_RISE_RATE = "rise_rate"
CONF_FALL_RATE = "fall_rate"

CONF_COOLING = "cooling"
CONF_FAN_OUTPUT = "fan_output"
CONF_FAN_START_TEMPERATURE = "fan_start_temperature"
CONF_FAN_FULL_TEMPERATURE = "fan_full_temperature"

CONF_FAULT_POLICY = "fault_policy"
CONF_AUTO_CLEAR = "auto_clear"
CONF_CLEAR_DELAY = "clear_delay"

CONF_MANUAL_CURRENT = "manual_current"
CONF_STATE = "state"
CONF_FAULT = "fault"
CONF_CLEAR_FAULT = "clear_fault"

CONF_DCR_TEST = "dcr_test"
CONF_BASELINE_CURRENT = "baseline_current"
CONF_PULSE_CURRENT = "pulse_current"
CONF_SETTLE_TIME = "settle_time"
CONF_SAMPLE_TIME = "sample_time"
CONF_RECOVERY_TIME = "recovery_time"
CONF_REPEATS = "repeats"
CONF_START = "start"
CONF_RESISTANCE = "resistance"


def _positive(value):
    value = cv.float_(value)
    if value <= 0:
        raise cv.Invalid("value must be greater than zero")
    return value


def _non_negative(value):
    value = cv.float_(value)
    if value < 0:
        raise cv.Invalid("value must not be negative")
    return value


TEMPERATURE_INPUT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_REQUIRED, default=False): cv.boolean,
    }
)

OUTPUT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_DAC): cv.use_id(output.FloatOutput),
        cv.Required(CONF_DAC_FULL_SCALE_CURRENT): _positive,
    }
)

MEASUREMENTS_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CURRENT): cv.use_id(sensor.Sensor),
        cv.Required(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_TEMPERATURES, default=[]): cv.ensure_list(
            TEMPERATURE_INPUT_SCHEMA
        ),
        cv.Optional(CONF_SAMPLE_TIMEOUT, default="250ms"):
            cv.positive_time_period_milliseconds,
    }
)


def _validate_limits(config):
    if config[CONF_MAXIMUM_VOLTAGE] <= config[CONF_MINIMUM_VOLTAGE]:
        raise cv.Invalid(
            "maximum_voltage must be greater than minimum_voltage"
        )
    return config


LIMITS_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_MAXIMUM_CURRENT): _positive,
            cv.Required(CONF_MINIMUM_VOLTAGE): _non_negative,
            cv.Required(CONF_MAXIMUM_VOLTAGE): _positive,
            cv.Required(CONF_MAXIMUM_POWER): _positive,
            cv.Required(CONF_MAXIMUM_TEMPERATURE): cv.float_,
        }
    ),
    _validate_limits,
)

CONTROL_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_PERIOD, default="50ms"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_DEADBAND, default=0.01): _non_negative,
        cv.Optional(CONF_RISE_RATE, default=2.0): _positive,
        cv.Optional(CONF_FALL_RATE, default=4.0): _positive,
    }
)


def _validate_cooling(config):
    if (
        config[CONF_FAN_FULL_TEMPERATURE]
        <= config[CONF_FAN_START_TEMPERATURE]
    ):
        raise cv.Invalid(
            "fan_full_temperature must be greater than "
            "fan_start_temperature"
        )
    return config


COOLING_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_FAN_OUTPUT): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_FAN_START_TEMPERATURE, default=35.0): cv.float_,
            cv.Optional(CONF_FAN_FULL_TEMPERATURE, default=70.0): cv.float_,
        }
    ),
    _validate_cooling,
)

FAULT_POLICY_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_AUTO_CLEAR, default=False): cv.boolean,
        cv.Optional(CONF_CLEAR_DELAY, default="2s"):
            cv.positive_time_period_milliseconds,
    }
)

DCR_TEST_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ProgrammableLoadDcrTest),
        cv.Optional(CONF_BASELINE_CURRENT, default=0.0): _non_negative,
        cv.Required(CONF_PULSE_CURRENT): _positive,
        cv.Optional(CONF_SETTLE_TIME, default="100ms"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_SAMPLE_TIME, default="500ms"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_RECOVERY_TIME, default="1s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_REPEATS, default=3): cv.int_range(min=1, max=32),
        cv.Required(CONF_START): button.button_schema(
            ProgrammableLoadDcrStartButton
        ),
        cv.Required(CONF_RESISTANCE): sensor.sensor_schema(
            unit_of_measurement="mΩ",
            accuracy_decimals=1,
        ),
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ProgrammableLoadComponent),
        cv.Required(CONF_OUTPUT): OUTPUT_SCHEMA,
        cv.Required(CONF_MEASUREMENTS): MEASUREMENTS_SCHEMA,
        cv.Required(CONF_LIMITS): LIMITS_SCHEMA,
        cv.Optional(CONF_CONTROL, default={}): CONTROL_SCHEMA,
        cv.Required(CONF_COOLING): COOLING_SCHEMA,
        cv.Optional(CONF_FAULT_POLICY, default={}): FAULT_POLICY_SCHEMA,
        cv.Required(CONF_MANUAL_CURRENT): number.number_schema(
            ProgrammableLoadManualCurrentNumber,
            unit_of_measurement=UNIT_AMPERE,
            device_class=DEVICE_CLASS_CURRENT,
        ),
        cv.Required(CONF_STATE): text_sensor.text_sensor_schema(),
        cv.Required(CONF_FAULT): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CLEAR_FAULT): button.button_schema(
            ProgrammableLoadClearFaultButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_DCR_TEST): DCR_TEST_SCHEMA,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    output_config = config[CONF_OUTPUT]
    dac = await cg.get_variable(output_config[CONF_DAC])
    cg.add(var.set_dac_output(dac))
    cg.add(
        var.set_dac_full_scale_current(
            output_config[CONF_DAC_FULL_SCALE_CURRENT]
        )
    )

    measurements = config[CONF_MEASUREMENTS]
    current = await cg.get_variable(measurements[CONF_CURRENT])
    voltage = await cg.get_variable(measurements[CONF_VOLTAGE])
    cg.add(var.set_current_sensor(current))
    cg.add(var.set_voltage_sensor(voltage))
    cg.add(var.set_sample_timeout_ms(
        measurements[CONF_SAMPLE_TIMEOUT].total_milliseconds
    ))
    for temperature_config in measurements[CONF_TEMPERATURES]:
        temperature = await cg.get_variable(temperature_config[CONF_SENSOR])
        cg.add(
            var.add_temperature_sensor(
                temperature,
                temperature_config[CONF_REQUIRED],
            )
        )

    limits = config[CONF_LIMITS]
    cg.add(var.set_maximum_current(limits[CONF_MAXIMUM_CURRENT]))
    cg.add(var.set_minimum_voltage(limits[CONF_MINIMUM_VOLTAGE]))
    cg.add(var.set_maximum_voltage(limits[CONF_MAXIMUM_VOLTAGE]))
    cg.add(var.set_maximum_power(limits[CONF_MAXIMUM_POWER]))
    cg.add(var.set_maximum_temperature(limits[CONF_MAXIMUM_TEMPERATURE]))

    control = config[CONF_CONTROL]
    cg.add(var.set_control_period_ms(control[CONF_PERIOD].total_milliseconds))
    cg.add(var.set_deadband(control[CONF_DEADBAND]))
    cg.add(var.set_rise_rate(control[CONF_RISE_RATE]))
    cg.add(var.set_fall_rate(control[CONF_FALL_RATE]))

    cooling = config[CONF_COOLING]
    fan = await cg.get_variable(cooling[CONF_FAN_OUTPUT])
    cg.add(var.set_fan_output(fan))
    cg.add(
        var.set_fan_temperature_range(
            cooling[CONF_FAN_START_TEMPERATURE],
            cooling[CONF_FAN_FULL_TEMPERATURE],
        )
    )

    fault_policy = config[CONF_FAULT_POLICY]
    cg.add(var.set_fault_auto_clear(fault_policy[CONF_AUTO_CLEAR]))
    cg.add(
        var.set_fault_clear_delay_ms(
            fault_policy[CONF_CLEAR_DELAY].total_milliseconds
        )
    )

    manual_current = await number.new_number(
        config[CONF_MANUAL_CURRENT],
        min_value=0.0,
        max_value=limits[CONF_MAXIMUM_CURRENT],
        step=0.1,
    )
    cg.add(manual_current.set_parent(var))
    cg.add(var.set_manual_current_number(manual_current))

    state = await text_sensor.new_text_sensor(config[CONF_STATE])
    fault = await text_sensor.new_text_sensor(config[CONF_FAULT])
    cg.add(var.set_state_sensor(state))
    cg.add(var.set_fault_sensor(fault))

    if CONF_CLEAR_FAULT in config:
        clear_fault = await button.new_button(config[CONF_CLEAR_FAULT])
        cg.add(clear_fault.set_parent(var))

    if CONF_DCR_TEST in config:
        dcr_config = config[CONF_DCR_TEST]
        dcr = cg.new_Pvariable(dcr_config[CONF_ID])
        cg.add(dcr.set_parent(var))
        cg.add(dcr.set_baseline_current(dcr_config[CONF_BASELINE_CURRENT]))
        cg.add(dcr.set_pulse_current(dcr_config[CONF_PULSE_CURRENT]))
        cg.add(
            dcr.set_timing(
                dcr_config[CONF_SETTLE_TIME].total_milliseconds,
                dcr_config[CONF_SAMPLE_TIME].total_milliseconds,
                dcr_config[CONF_RECOVERY_TIME].total_milliseconds,
            )
        )
        cg.add(dcr.set_repeats(dcr_config[CONF_REPEATS]))

        start = await button.new_button(dcr_config[CONF_START])
        cg.add(start.set_parent(dcr))

        resistance = await sensor.new_sensor(dcr_config[CONF_RESISTANCE])
        cg.add(dcr.set_resistance_sensor(resistance))
        cg.add(var.add_procedure(dcr))
