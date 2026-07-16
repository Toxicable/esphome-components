import esphome.config_validation as cv
from esphome.components import button, number, output, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_AMPERE,
    UNIT_VOLT,
)

from ._types import *

ABSOLUTE_MAXIMUM_VOLTAGE = 75.0


def _positive(value):
    value = cv.float_(value)
    if value <= 0:
        raise cv.Invalid("value must be greater than zero")
    return value


def _hardware_voltage(value):
    value = _positive(value)
    if value > ABSOLUTE_MAXIMUM_VOLTAGE:
        raise cv.Invalid(
            f"hardware.maximum_voltage must not exceed the absolute "
            f"{ABSOLUTE_MAXIMUM_VOLTAGE:g} V component ceiling"
        )
    return value


def _positive_scale(value):
    value = cv.float_(value)
    if value <= 0:
        raise cv.Invalid("calibration scale must be greater than zero")
    return value


def _non_negative(value):
    value = cv.float_(value)
    if value < 0:
        raise cv.Invalid("value must not be negative")
    return value


def _normalized_level(value):
    value = cv.float_(value)
    if value < 0 or value >= 1:
        raise cv.Invalid("zero_level must be in the range 0 <= value < 1")
    return value


TEMPERATURE_INPUT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_REQUIRED, default=False): cv.boolean,
    }
)

HARDWARE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_DAC): cv.use_id(output.FloatOutput),
        cv.Required(CONF_HARDWARE_MAXIMUM_VOLTAGE): _hardware_voltage,
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

LINEAR_CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_SCALE, default=1.0): _positive_scale,
        cv.Optional(CONF_OFFSET, default=0.0): cv.float_,
    }
)

OUTPUT_CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_ZERO_LEVEL, default=0.0): _normalized_level,
        cv.Required(CONF_FULL_SCALE_CURRENT): _positive,
    }
)

CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_RESTORE, default=True): cv.boolean,
        cv.Optional(CONF_CURRENT, default={}): LINEAR_CALIBRATION_SCHEMA,
        cv.Optional(CONF_VOLTAGE, default={}): LINEAR_CALIBRATION_SCHEMA,
        cv.Required(CONF_OUTPUT): OUTPUT_CALIBRATION_SCHEMA,
        cv.Optional(CONF_CALIBRATION_STATUS): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CURRENT_SCALE): sensor.sensor_schema(
            accuracy_decimals=6,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_CURRENT_OFFSET): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=6,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_VOLTAGE_SCALE): sensor.sensor_schema(
            accuracy_decimals=6,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_VOLTAGE_OFFSET): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=6,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_OUTPUT_ZERO_LEVEL): sensor.sensor_schema(
            accuracy_decimals=6,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_OUTPUT_FULL_SCALE_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_RESET_CALIBRATION): button.button_schema(
            ResetCalibrationButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
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
        cv.Optional(CONF_PROPORTIONAL_GAIN, default=0.2): _positive,
        cv.Optional(CONF_INTEGRAL_GAIN, default=0.4): _non_negative,
        cv.Optional(CONF_LOG_CONTROL_SAMPLES, default=False): cv.boolean,
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

DCR_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DcrTest),
        cv.Optional(CONF_BASELINE_CURRENT, default=0.0): _non_negative,
        cv.Required(CONF_PULSE_CURRENT): _positive,
        cv.Optional(CONF_SETTLE_TIME, default="100ms"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_SAMPLE_TIME, default="500ms"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_RECOVERY_TIME, default="1s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_REPEATS, default=3): cv.int_range(min=1, max=32),
        cv.Required(CONF_START): button.button_schema(DcrStartButton),
        cv.Required(CONF_RESISTANCE): sensor.sensor_schema(
            unit_of_measurement="mΩ",
            accuracy_decimals=1,
        ),
    }
)

BATTERY_CYCLE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BatteryCycle),
        cv.Required(CONF_CHARGER): cv.use_id(ChargerInterface),
        cv.Optional(CONF_CHARGER_SAMPLE_TIMEOUT, default="3s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CHARGER_CONTROL_TIMEOUT, default="5s"):
            cv.positive_time_period_milliseconds,
        cv.Required(CONF_DISCHARGE_CURRENT): _positive,
        cv.Required(CONF_DISCHARGE_CUTOFF_VOLTAGE): _positive,
        cv.Optional(CONF_DISCHARGE_CUTOFF_HYSTERESIS, default=0.1):
            _non_negative,
        cv.Optional(CONF_DISCHARGE_CUTOFF_HOLD_TIME, default="2s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_REST_TIME, default="5s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CHARGE_START_TIMEOUT, default="30s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CHARGE_STALL_TIMEOUT, default="30s"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CHARGE_TIMEOUT, default="24h"):
            cv.positive_time_period_milliseconds,
        cv.Optional(CONF_TERMINATION_HOLD_TIME, default="2s"):
            cv.positive_time_period_milliseconds,
        cv.Required(CONF_START): button.button_schema(BatteryCycleStartButton),
        cv.Required(CONF_STOP): button.button_schema(BatteryCycleStopButton),
        cv.Optional(CONF_PHASE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_RESULT): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_DISCHARGED_CAPACITY): sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=3,
        ),
        cv.Optional(CONF_DISCHARGED_ENERGY): sensor.sensor_schema(
            unit_of_measurement="Wh",
            accuracy_decimals=3,
        ),
        cv.Optional(CONF_CHARGED_CAPACITY): sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=3,
        ),
        cv.Optional(CONF_CHARGED_ENERGY): sensor.sensor_schema(
            unit_of_measurement="Wh",
            accuracy_decimals=3,
        ),
    }
)

PROCEDURES_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_DCR): DCR_SCHEMA,
        cv.Optional(CONF_BATTERY_CYCLE): BATTERY_CYCLE_SCHEMA,
    }
)


def _validate_config(config):
    hardware_maximum_voltage = config[CONF_HARDWARE][
        CONF_HARDWARE_MAXIMUM_VOLTAGE
    ]
    operating_maximum_voltage = config[CONF_LIMITS][CONF_MAXIMUM_VOLTAGE]
    if operating_maximum_voltage > hardware_maximum_voltage:
        raise cv.Invalid(
            "limits.maximum_voltage must not exceed "
            "hardware.maximum_voltage"
        )

    maximum_current = config[CONF_LIMITS][CONF_MAXIMUM_CURRENT]
    output_full_scale = config[CONF_CALIBRATION][CONF_OUTPUT][
        CONF_FULL_SCALE_CURRENT
    ]
    if maximum_current > output_full_scale:
        raise cv.Invalid(
            "limits.maximum_current must not exceed "
            "calibration.output.full_scale_current"
        )

    procedures = config[CONF_PROCEDURES]
    dcr = procedures.get(CONF_DCR)
    if dcr is not None:
        if dcr[CONF_BASELINE_CURRENT] > maximum_current:
            raise cv.Invalid(
                "procedures.dcr.baseline_current must not exceed "
                "limits.maximum_current"
            )
        if dcr[CONF_PULSE_CURRENT] > maximum_current:
            raise cv.Invalid(
                "procedures.dcr.pulse_current must not exceed "
                "limits.maximum_current"
            )
        if dcr[CONF_PULSE_CURRENT] == dcr[CONF_BASELINE_CURRENT]:
            raise cv.Invalid(
                "procedures.dcr.pulse_current must differ from "
                "baseline_current"
            )

    cycle = procedures.get(CONF_BATTERY_CYCLE)
    if cycle is not None:
        if cycle[CONF_DISCHARGE_CURRENT] > maximum_current:
            raise cv.Invalid(
                "procedures.battery_cycle.discharge_current must not exceed "
                "limits.maximum_current"
            )
        cutoff = cycle[CONF_DISCHARGE_CUTOFF_VOLTAGE]
        minimum = config[CONF_LIMITS][CONF_MINIMUM_VOLTAGE]
        maximum = config[CONF_LIMITS][CONF_MAXIMUM_VOLTAGE]
        if cutoff <= minimum:
            raise cv.Invalid(
                "procedures.battery_cycle.discharge_cutoff_voltage must be "
                "greater than limits.minimum_voltage so the cycle can stop "
                "normally before the load undervoltage fault"
            )
        if cutoff >= maximum:
            raise cv.Invalid(
                "procedures.battery_cycle.discharge_cutoff_voltage must be "
                "less than limits.maximum_voltage"
            )

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ProgrammableLoadComponent),
            cv.Required(CONF_HARDWARE): HARDWARE_SCHEMA,
            cv.Required(CONF_MEASUREMENTS): MEASUREMENTS_SCHEMA,
            cv.Required(CONF_CALIBRATION): CALIBRATION_SCHEMA,
            cv.Required(CONF_LIMITS): LIMITS_SCHEMA,
            cv.Optional(CONF_CONTROL, default={}): CONTROL_SCHEMA,
            cv.Required(CONF_COOLING): COOLING_SCHEMA,
            cv.Optional(CONF_FAULT_POLICY, default={}): FAULT_POLICY_SCHEMA,
            cv.Required(CONF_MANUAL_CURRENT): number.number_schema(
                ManualCurrentNumber,
                unit_of_measurement=UNIT_AMPERE,
                device_class=DEVICE_CLASS_CURRENT,
            ),
            cv.Required(CONF_STATE): text_sensor.text_sensor_schema(),
            cv.Required(CONF_FAULT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CLEAR_FAULT): button.button_schema(ClearFaultButton),
            cv.Optional(CONF_PROCEDURES, default={}): PROCEDURES_SCHEMA,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _validate_config,
)
