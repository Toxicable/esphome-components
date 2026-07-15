import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, number, output, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    UNIT_AMPERE,
)

AUTO_LOAD = ["button", "component_common", "number", "sensor", "text_sensor"]

component_common_ns = cg.global_ns.namespace("component_common")
ChargerInterface = component_common_ns.class_("ChargerInterface")

programmable_load_ns = cg.esphome_ns.namespace("programmable_load")

ProgrammableLoadComponent = programmable_load_ns.class_(
    "ProgrammableLoadComponent", cg.Component
)
ManualCurrentNumber = programmable_load_ns.class_(
    "ManualCurrentNumber", number.Number
)
ClearFaultButton = programmable_load_ns.class_(
    "ClearFaultButton", button.Button
)
DcrTest = programmable_load_ns.class_("DcrTest")
DcrStartButton = programmable_load_ns.class_(
    "DcrStartButton", button.Button
)
BatteryCycle = programmable_load_ns.class_("BatteryCycle")
BatteryCycleStartButton = programmable_load_ns.class_(
    "BatteryCycleStartButton", button.Button
)
BatteryCycleStopButton = programmable_load_ns.class_(
    "BatteryCycleStopButton", button.Button
)

CONF_HARDWARE = "hardware"
CONF_DAC = "dac"
CONF_HARDWARE_MAXIMUM_VOLTAGE = "maximum_voltage"

CONF_MEASUREMENTS = "measurements"
CONF_CURRENT = "current"
CONF_VOLTAGE = "voltage"
CONF_TEMPERATURES = "temperatures"
CONF_SENSOR = "sensor"
CONF_REQUIRED = "required"
CONF_SAMPLE_TIMEOUT = "sample_timeout"

CONF_CALIBRATION = "calibration"
CONF_RESTORE = "restore"
CONF_SCALE = "scale"
CONF_OFFSET = "offset"
CONF_OUTPUT = "output"
CONF_ZERO_LEVEL = "zero_level"
CONF_FULL_SCALE_CURRENT = "full_scale_current"

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
CONF_PROPORTIONAL_GAIN = "proportional_gain"
CONF_INTEGRAL_GAIN = "integral_gain"
CONF_LOG_CONTROL_SAMPLES = "log_control_samples"

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

CONF_PROCEDURES = "procedures"
CONF_DCR = "dcr"
CONF_BASELINE_CURRENT = "baseline_current"
CONF_PULSE_CURRENT = "pulse_current"
CONF_SETTLE_TIME = "settle_time"
CONF_SAMPLE_TIME = "sample_time"
CONF_RECOVERY_TIME = "recovery_time"
CONF_REPEATS = "repeats"
CONF_START = "start"
CONF_STOP = "stop"
CONF_RESISTANCE = "resistance"

CONF_BATTERY_CYCLE = "battery_cycle"
CONF_CHARGER = "charger"
CONF_CHARGER_SAMPLE_TIMEOUT = "charger_sample_timeout"
CONF_CHARGER_CONTROL_TIMEOUT = "charger_control_timeout"
CONF_DISCHARGE_CURRENT = "discharge_current"
CONF_DISCHARGE_CUTOFF_VOLTAGE = "discharge_cutoff_voltage"
CONF_DISCHARGE_CUTOFF_HYSTERESIS = "discharge_cutoff_hysteresis"
CONF_DISCHARGE_CUTOFF_HOLD_TIME = "discharge_cutoff_hold_time"
CONF_REST_TIME = "rest_time"
CONF_CHARGE_START_TIMEOUT = "charge_start_timeout"
CONF_CHARGE_STALL_TIMEOUT = "charge_stall_timeout"
CONF_CHARGE_TIMEOUT = "charge_timeout"
CONF_TERMINATION_HOLD_TIME = "termination_hold_time"
CONF_PHASE = "phase"
CONF_RESULT = "result"
CONF_DISCHARGED_CAPACITY = "discharged_capacity"
CONF_DISCHARGED_ENERGY = "discharged_energy"
CONF_CHARGED_CAPACITY = "charged_capacity"
CONF_CHARGED_ENERGY = "charged_energy"


def _positive(value):
    value = cv.float_(value)
    if value <= 0:
        raise cv.Invalid("value must be greater than zero")
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
        cv.Required(CONF_HARDWARE_MAXIMUM_VOLTAGE): _positive,
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


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    hardware = config[CONF_HARDWARE]
    dac = await cg.get_variable(hardware[CONF_DAC])
    cg.add(var.set_dac_output(dac))
    cg.add(
        var.set_hardware_maximum_voltage(
            hardware[CONF_HARDWARE_MAXIMUM_VOLTAGE]
        )
    )

    measurements = config[CONF_MEASUREMENTS]
    current = await cg.get_variable(measurements[CONF_CURRENT])
    voltage = await cg.get_variable(measurements[CONF_VOLTAGE])
    cg.add(var.set_current_sensor(current))
    cg.add(var.set_voltage_sensor(voltage))
    cg.add(
        var.set_sample_timeout_ms(
            measurements[CONF_SAMPLE_TIMEOUT].total_milliseconds
        )
    )
    for temperature_config in measurements[CONF_TEMPERATURES]:
        temperature = await cg.get_variable(temperature_config[CONF_SENSOR])
        cg.add(
            var.add_temperature_sensor(
                temperature,
                temperature_config[CONF_REQUIRED],
            )
        )

    calibration = config[CONF_CALIBRATION]
    current_calibration = calibration[CONF_CURRENT]
    voltage_calibration = calibration[CONF_VOLTAGE]
    output_calibration = calibration[CONF_OUTPUT]
    cg.add(
        var.set_current_calibration(
            current_calibration[CONF_SCALE],
            current_calibration[CONF_OFFSET],
        )
    )
    cg.add(
        var.set_voltage_calibration(
            voltage_calibration[CONF_SCALE],
            voltage_calibration[CONF_OFFSET],
        )
    )
    cg.add(
        var.set_output_calibration(
            output_calibration[CONF_ZERO_LEVEL],
            output_calibration[CONF_FULL_SCALE_CURRENT],
        )
    )
    cg.add(var.set_restore_calibration(calibration[CONF_RESTORE]))

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
    cg.add(var.set_proportional_gain(control[CONF_PROPORTIONAL_GAIN]))
    cg.add(var.set_integral_gain(control[CONF_INTEGRAL_GAIN]))
    cg.add(var.set_log_control_samples(control[CONF_LOG_CONTROL_SAMPLES]))

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

    procedures = config[CONF_PROCEDURES]

    dcr_config = procedures.get(CONF_DCR)
    if dcr_config is not None:
        dcr = cg.new_Pvariable(dcr_config[CONF_ID])
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
        cg.add(start.set_host(var))
        cg.add(start.set_procedure(dcr))

        resistance = await sensor.new_sensor(dcr_config[CONF_RESISTANCE])
        cg.add(dcr.set_resistance_sensor(resistance))

    cycle_config = procedures.get(CONF_BATTERY_CYCLE)
    if cycle_config is not None:
        charger = await cg.get_variable(cycle_config[CONF_CHARGER])
        cg.add(var.set_charger(charger))
        cg.add(
            var.set_charger_sample_timeout_ms(
                cycle_config[CONF_CHARGER_SAMPLE_TIMEOUT].total_milliseconds
            )
        )
        cg.add(
            var.set_charger_control_timeout_ms(
                cycle_config[CONF_CHARGER_CONTROL_TIMEOUT].total_milliseconds
            )
        )

        cycle = cg.new_Pvariable(cycle_config[CONF_ID])
        cg.add(cycle.set_discharge_current(cycle_config[CONF_DISCHARGE_CURRENT]))
        cg.add(
            cycle.set_discharge_cutoff_voltage(
                cycle_config[CONF_DISCHARGE_CUTOFF_VOLTAGE]
            )
        )
        cg.add(
            cycle.set_discharge_cutoff_hysteresis(
                cycle_config[CONF_DISCHARGE_CUTOFF_HYSTERESIS]
            )
        )
        cg.add(
            cycle.set_discharge_cutoff_hold_time_ms(
                cycle_config[CONF_DISCHARGE_CUTOFF_HOLD_TIME].total_milliseconds
            )
        )
        cg.add(
            cycle.set_rest_time_ms(
                cycle_config[CONF_REST_TIME].total_milliseconds
            )
        )
        cg.add(
            cycle.set_charge_start_timeout_ms(
                cycle_config[CONF_CHARGE_START_TIMEOUT].total_milliseconds
            )
        )
        cg.add(
            cycle.set_charge_stall_timeout_ms(
                cycle_config[CONF_CHARGE_STALL_TIMEOUT].total_milliseconds
            )
        )
        cg.add(
            cycle.set_charge_timeout_ms(
                cycle_config[CONF_CHARGE_TIMEOUT].total_milliseconds
            )
        )
        cg.add(
            cycle.set_termination_hold_time_ms(
                cycle_config[CONF_TERMINATION_HOLD_TIME].total_milliseconds
            )
        )

        cycle_start = await button.new_button(cycle_config[CONF_START])
        cg.add(cycle_start.set_host(var))
        cg.add(cycle_start.set_procedure(cycle))
        cycle_stop = await button.new_button(cycle_config[CONF_STOP])
        cg.add(cycle_stop.set_host(var))
        cg.add(cycle_stop.set_procedure(cycle))

        if CONF_PHASE in cycle_config:
            phase = await text_sensor.new_text_sensor(cycle_config[CONF_PHASE])
            cg.add(cycle.set_phase_sensor(phase))
        if CONF_RESULT in cycle_config:
            result = await text_sensor.new_text_sensor(cycle_config[CONF_RESULT])
            cg.add(cycle.set_result_sensor(result))
        if CONF_DISCHARGED_CAPACITY in cycle_config:
            value = await sensor.new_sensor(cycle_config[CONF_DISCHARGED_CAPACITY])
            cg.add(cycle.set_discharged_capacity_sensor(value))
        if CONF_DISCHARGED_ENERGY in cycle_config:
            value = await sensor.new_sensor(cycle_config[CONF_DISCHARGED_ENERGY])
            cg.add(cycle.set_discharged_energy_sensor(value))
        if CONF_CHARGED_CAPACITY in cycle_config:
            value = await sensor.new_sensor(cycle_config[CONF_CHARGED_CAPACITY])
            cg.add(cycle.set_charged_capacity_sensor(value))
        if CONF_CHARGED_ENERGY in cycle_config:
            value = await sensor.new_sensor(cycle_config[CONF_CHARGED_ENERGY])
            cg.add(cycle.set_charged_energy_sensor(value))
