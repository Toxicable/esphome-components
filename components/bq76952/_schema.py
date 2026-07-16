import esphome.config_validation as cv
from esphome.components import button, i2c, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from ._types import (
    BQ76952BalancingConfig,
    BQ76952CellChemistry,
    BQ76952CellVoltageProtectionConfig,
    BQ76952ClearAlarmsButton,
    BQ76952Component,
    BQ76952Config,
    BQ76952CurrentGainPolicy,
    BQ76952CurrentProtectionConfig,
    BQ76952FetConfig,
    BQ76952OutputEnabledSwitch,
    BQ76952PrechargeConfig,
    BQ76952PredischargeConfig,
    BQ76952ProgramFactoryOtpButton,
    BQ76952ProtectionConfig,
    BQ76952RegulatorConfig,
    BQ76952ShortCircuitProtectionConfig,
    BQ76952SocConfig,
    BQ76952SustainedCurrentProtectionConfig,
    BQ76952TemperatureProtectionConfig,
    BQ76952ThermistorConfig,
    BQ76952ThermistorMode,
)

CONF_CELL_COUNT = "cell_count"
CONF_CELL_CHEMISTRY = "cell_chemistry"
CONF_SENSE_RESISTOR_MILLIOHM = "sense_resistor_milliohm"
CONF_I2C_CRC_ENABLED = "i2c_crc_enabled"
CONF_CURRENT_GAIN_POLICY = "current_gain_policy"

CONF_REGULATORS = "regulators"
CONF_REG0_ENABLED = "reg0_enabled"
CONF_REG1_ENABLED = "reg1_enabled"
CONF_REG1_VOLTAGE = "reg1_voltage"
CONF_REG2_ENABLED = "reg2_enabled"
CONF_REG2_VOLTAGE = "reg2_voltage"

CONF_THERMISTORS = "thermistors"
CONF_TS1 = "ts1"
CONF_TS2 = "ts2"
CONF_TS3 = "ts3"

CONF_FET = "fet"
CONF_AUTONOMOUS = "autonomous"
CONF_SLEEP_CHARGE_ENABLED = "sleep_charge_enabled"
CONF_PRECHARGE = "precharge"
CONF_PREDISCHARGE = "predischarge"
CONF_ENABLED = "enabled"
CONF_START_CELL_VOLTAGE_MV = "start_cell_voltage_mv"
CONF_STOP_CELL_VOLTAGE_MV = "stop_cell_voltage_mv"
CONF_TIMEOUT_MS = "timeout_ms"
CONF_STOP_DELTA_MV = "stop_delta_mv"

CONF_BALANCING = "balancing"
CONF_MINIMUM_CELL_VOLTAGE_MV = "minimum_cell_voltage_mv"
CONF_START_DELTA_MV = "start_delta_mv"
CONF_MINIMUM_TEMPERATURE_C = "minimum_temperature_c"
CONF_MAXIMUM_TEMPERATURE_C = "maximum_temperature_c"
CONF_MAXIMUM_BALANCED_CELLS = "maximum_balanced_cells"

CONF_PROTECTIONS = "protections"
CONF_SOC = "soc"
CONF_EMPTY_CELL_VOLTAGE_MV = "empty_cell_voltage_mv"
CONF_FULL_CELL_VOLTAGE_MV = "full_cell_voltage_mv"
CONF_CELL_UNDERVOLTAGE = "cell_undervoltage"
CONF_CELL_OVERVOLTAGE = "cell_overvoltage"
CONF_CHARGE_OVERCURRENT = "charge_overcurrent"
CONF_DISCHARGE_OVERCURRENT = "discharge_overcurrent"
CONF_DISCHARGE_SEVERE_OVERCURRENT = "discharge_severe_overcurrent"
CONF_DISCHARGE_SUSTAINED_OVERCURRENT = "discharge_sustained_overcurrent"
CONF_DISCHARGE_SHORT_CIRCUIT = "discharge_short_circuit"
CONF_TEMPERATURE = "temperature"
CONF_THRESHOLD_MV = "threshold_mv"
CONF_THRESHOLD_A = "threshold_a"
CONF_DELAY_MS = "delay_ms"
CONF_DELAY_S = "delay_s"
CONF_DELAY_US = "delay_us"
CONF_RECOVERY_TIME_S = "recovery_time_s"
CONF_RECOVERY_HYSTERESIS_MV = "recovery_hysteresis_mv"
CONF_CURRENT_RECOVERY_TIME_S = "current_recovery_time_s"
CONF_CHARGE_MINIMUM_C = "charge_minimum_c"
CONF_CHARGE_MAXIMUM_C = "charge_maximum_c"
CONF_DISCHARGE_MINIMUM_C = "discharge_minimum_c"
CONF_DISCHARGE_MAXIMUM_C = "discharge_maximum_c"
CONF_RECOVERY_HYSTERESIS_C = "recovery_hysteresis_c"

CONF_BAT_VOLTAGE = "bat_voltage"
CONF_PACK_VOLTAGE = "pack_voltage"
CONF_LD_VOLTAGE = "ld_voltage"
CONF_LARGEST_INTERCELL_VOLTAGE = "largest_intercell_voltage"
CONF_CURRENT = "current"
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_LEARNED_CAPACITY = "learned_capacity"
CONF_DIE_TEMPERATURE = "die_temperature"
CONF_TS1_TEMPERATURE = "ts1_temperature"
CONF_TS2_TEMPERATURE = "ts2_temperature"
CONF_TS3_TEMPERATURE = "ts3_temperature"
CONF_CONNECTION_STATE = "connection_state"
CONF_STATE = "state"
CONF_FAULT = "fault"
CONF_CAPACITY_CALIBRATION_STATUS = "capacity_calibration_status"
CONF_OUTPUT_ENABLED_CONTROL = "output_enabled_control"
CONF_CLEAR_ALARMS = "clear_alarms"
CONF_MANUFACTURING = "manufacturing"
CONF_PROGRAM_FACTORY_OTP = "program_factory_otp"

CELL_VOLTAGE_KEYS = [f"cell{index}_voltage" for index in range(1, 17)]

CELL_CHEMISTRY_OPTIONS = {
    "lithium_ion": BQ76952CellChemistry.LITHIUM_ION,
}

CURRENT_GAIN_POLICY_OPTIONS = {
    "factory_calibration": BQ76952CurrentGainPolicy.FACTORY_CALIBRATION,
    "derive_from_shunt": BQ76952CurrentGainPolicy.DERIVE_FROM_SHUNT,
}

THERMISTOR_MODE_OPTIONS = {
    "disabled": BQ76952ThermistorMode.DISABLED,
    "18k": BQ76952ThermistorMode.PULLUP_18K,
    "180k": BQ76952ThermistorMode.PULLUP_180K,
}

REGULATOR_VOLTAGE_OPTIONS = {
    "1.8v": 0x00,
    "2.5v": 0x08,
    "3.0v": 0x0A,
    "3.3v": 0x0C,
    "5.0v": 0x0E,
}

SCD_THRESHOLD_OPTIONS = (
    10,
    20,
    40,
    60,
    80,
    100,
    125,
    150,
    175,
    200,
    250,
    300,
    350,
    400,
    450,
    500,
)


def _validate_scd_delay_us(value):
    if value == 0 or (15 <= value <= 450 and value % 15 == 0):
        return value
    raise cv.Invalid("delay_us must be 0 or a multiple of 15 from 15 to 450")


def _validate_10_unit_0_to_2550(value):
    if value == 0 or (10 <= value <= 2550 and value % 10 == 0):
        return value
    raise cv.Invalid("value must be 0 or a multiple of 10 from 10 to 2550")


REGULATOR_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_REG0_ENABLED): cv.boolean,
        cv.Required(CONF_REG1_ENABLED): cv.boolean,
        cv.Required(CONF_REG1_VOLTAGE): cv.enum(
            REGULATOR_VOLTAGE_OPTIONS, lower=True
        ),
        cv.Required(CONF_REG2_ENABLED): cv.boolean,
        cv.Required(CONF_REG2_VOLTAGE): cv.enum(
            REGULATOR_VOLTAGE_OPTIONS, lower=True
        ),
    }
)

THERMISTOR_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TS1): cv.enum(THERMISTOR_MODE_OPTIONS, lower=True),
        cv.Required(CONF_TS2): cv.enum(THERMISTOR_MODE_OPTIONS, lower=True),
        cv.Required(CONF_TS3): cv.enum(THERMISTOR_MODE_OPTIONS, lower=True),
    }
)

PRECHARGE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ENABLED): cv.boolean,
        cv.Required(CONF_START_CELL_VOLTAGE_MV): cv.int_range(min=0, max=5000),
        cv.Required(CONF_STOP_CELL_VOLTAGE_MV): cv.int_range(min=0, max=5000),
    }
)

PREDISCHARGE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ENABLED): cv.boolean,
        cv.Required(CONF_TIMEOUT_MS): _validate_10_unit_0_to_2550,
        cv.Required(CONF_STOP_DELTA_MV): _validate_10_unit_0_to_2550,
    }
)

FET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_AUTONOMOUS): cv.boolean,
        cv.Required(CONF_SLEEP_CHARGE_ENABLED): cv.boolean,
        cv.Required(CONF_PRECHARGE): PRECHARGE_SCHEMA,
        cv.Required(CONF_PREDISCHARGE): PREDISCHARGE_SCHEMA,
    }
)

BALANCING_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_MINIMUM_CELL_VOLTAGE_MV): cv.int_range(
            min=0, max=5000
        ),
        cv.Required(CONF_START_DELTA_MV): cv.int_range(min=0, max=1000),
        cv.Required(CONF_STOP_DELTA_MV): cv.int_range(min=0, max=1000),
        cv.Required(CONF_MINIMUM_TEMPERATURE_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_MAXIMUM_TEMPERATURE_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_MAXIMUM_BALANCED_CELLS): cv.int_range(min=1, max=16),
    }
)

CELL_VOLTAGE_PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_THRESHOLD_MV): cv.int_range(min=1000, max=6000),
        cv.Required(CONF_DELAY_MS): cv.int_range(min=1, max=7000),
        cv.Required(CONF_RECOVERY_HYSTERESIS_MV): cv.int_range(min=0, max=1000),
    }
)

CURRENT_PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_THRESHOLD_A): cv.float_range(min=0.001),
        cv.Required(CONF_DELAY_MS): cv.int_range(min=10, max=426),
    }
)

SUSTAINED_CURRENT_PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_THRESHOLD_A): cv.float_range(min=0.001),
        cv.Required(CONF_DELAY_S): cv.int_range(min=0, max=255),
    }
)

SHORT_CIRCUIT_PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_THRESHOLD_MV): cv.one_of(*SCD_THRESHOLD_OPTIONS, int=True),
        cv.Required(CONF_DELAY_US): _validate_scd_delay_us,
        cv.Required(CONF_RECOVERY_TIME_S): cv.int_range(min=0, max=255),
    }
)

TEMPERATURE_PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CHARGE_MINIMUM_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_CHARGE_MAXIMUM_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_DISCHARGE_MINIMUM_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_DISCHARGE_MAXIMUM_C): cv.int_range(min=-128, max=127),
        cv.Required(CONF_RECOVERY_HYSTERESIS_C): cv.int_range(min=0, max=255),
    }
)

PROTECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CELL_UNDERVOLTAGE): CELL_VOLTAGE_PROTECTION_SCHEMA,
        cv.Required(CONF_CELL_OVERVOLTAGE): CELL_VOLTAGE_PROTECTION_SCHEMA,
        cv.Required(CONF_CHARGE_OVERCURRENT): CURRENT_PROTECTION_SCHEMA,
        cv.Required(CONF_DISCHARGE_OVERCURRENT): CURRENT_PROTECTION_SCHEMA,
        cv.Required(CONF_DISCHARGE_SEVERE_OVERCURRENT): CURRENT_PROTECTION_SCHEMA,
        cv.Required(
            CONF_DISCHARGE_SUSTAINED_OVERCURRENT
        ): SUSTAINED_CURRENT_PROTECTION_SCHEMA,
        cv.Required(CONF_DISCHARGE_SHORT_CIRCUIT): SHORT_CIRCUIT_PROTECTION_SCHEMA,
        cv.Required(CONF_TEMPERATURE): TEMPERATURE_PROTECTION_SCHEMA,
        cv.Required(CONF_CURRENT_RECOVERY_TIME_S): cv.int_range(min=0, max=255),
    }
)

SOC_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_EMPTY_CELL_VOLTAGE_MV): cv.int_range(min=2500, max=4200),
        cv.Required(CONF_FULL_CELL_VOLTAGE_MV): cv.int_range(min=2500, max=4500),
    }
)

VOLTAGE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)


def _validate_config(config):
    cell_count = config[CONF_CELL_COUNT]
    precharge = config[CONF_FET][CONF_PRECHARGE]
    predischarge = config[CONF_FET][CONF_PREDISCHARGE]
    balancing = config[CONF_BALANCING]
    protections = config[CONF_PROTECTIONS]
    soc = config[CONF_SOC]

    if soc[CONF_EMPTY_CELL_VOLTAGE_MV] >= soc[CONF_FULL_CELL_VOLTAGE_MV]:
        raise cv.Invalid("soc.empty_cell_voltage_mv must be below soc.full_cell_voltage_mv")
    temperature = protections[CONF_TEMPERATURE]
    thermistors = config[CONF_THERMISTORS]

    if (
        precharge[CONF_ENABLED]
        and precharge[CONF_STOP_CELL_VOLTAGE_MV]
        <= precharge[CONF_START_CELL_VOLTAGE_MV]
    ):
        raise cv.Invalid(
            "precharge stop_cell_voltage_mv must exceed start_cell_voltage_mv"
        )

    if (
        predischarge[CONF_ENABLED]
        and predischarge[CONF_TIMEOUT_MS] == 0
        and predischarge[CONF_STOP_DELTA_MV] == 0
    ):
        raise cv.Invalid(
            "enabled predischarge requires a non-zero timeout_ms or stop_delta_mv"
        )

    if balancing[CONF_STOP_DELTA_MV] > balancing[CONF_START_DELTA_MV]:
        raise cv.Invalid("balancing stop_delta_mv must not exceed start_delta_mv")
    if balancing[CONF_MAXIMUM_BALANCED_CELLS] > cell_count:
        raise cv.Invalid("maximum_balanced_cells must not exceed cell_count")
    if (
        balancing[CONF_MINIMUM_TEMPERATURE_C]
        >= balancing[CONF_MAXIMUM_TEMPERATURE_C]
    ):
        raise cv.Invalid(
            "balancing minimum_temperature_c must be below maximum_temperature_c"
        )

    if temperature[CONF_CHARGE_MINIMUM_C] >= temperature[CONF_CHARGE_MAXIMUM_C]:
        raise cv.Invalid("charge_minimum_c must be below charge_maximum_c")
    if (
        temperature[CONF_DISCHARGE_MINIMUM_C]
        >= temperature[CONF_DISCHARGE_MAXIMUM_C]
    ):
        raise cv.Invalid("discharge_minimum_c must be below discharge_maximum_c")

    normal_ocd = protections[CONF_DISCHARGE_OVERCURRENT]
    severe_ocd = protections[CONF_DISCHARGE_SEVERE_OVERCURRENT]
    if severe_ocd[CONF_THRESHOLD_A] <= normal_ocd[CONF_THRESHOLD_A]:
        raise cv.Invalid(
            "discharge_severe_overcurrent threshold_a must exceed discharge_overcurrent threshold_a"
        )
    if severe_ocd[CONF_DELAY_MS] >= normal_ocd[CONF_DELAY_MS]:
        raise cv.Invalid(
            "discharge_severe_overcurrent delay_ms must be shorter than discharge_overcurrent delay_ms"
        )

    for index, key in enumerate(CELL_VOLTAGE_KEYS, start=1):
        if key in config and index > cell_count:
            raise cv.Invalid(f"{key} requires cell_count >= {index}")

    sensor_modes = (
        (CONF_TS1_TEMPERATURE, CONF_TS1),
        (CONF_TS2_TEMPERATURE, CONF_TS2),
        (CONF_TS3_TEMPERATURE, CONF_TS3),
    )
    for sensor_key, mode_key in sensor_modes:
        if (
            sensor_key in config
            and str(thermistors[mode_key]) == "disabled"
        ):
            raise cv.Invalid(
                f"{sensor_key} requires thermistors.{mode_key} to be enabled"
            )

    return config


MANUFACTURING_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PROGRAM_FACTORY_OTP): button.button_schema(
            BQ76952ProgramFactoryOtpButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


schema = {
    cv.GenerateID(): cv.declare_id(BQ76952Component),
    cv.Required(CONF_CELL_COUNT): cv.int_range(min=3, max=16),
    cv.Required(CONF_CELL_CHEMISTRY): cv.enum(
        CELL_CHEMISTRY_OPTIONS, lower=True
    ),
    cv.Required(CONF_SENSE_RESISTOR_MILLIOHM): cv.float_range(min=0.001),
    cv.Required(CONF_I2C_CRC_ENABLED): cv.boolean,
    cv.Required(CONF_CURRENT_GAIN_POLICY): cv.enum(
        CURRENT_GAIN_POLICY_OPTIONS, lower=True
    ),
    cv.Required(CONF_REGULATORS): REGULATOR_SCHEMA,
    cv.Required(CONF_THERMISTORS): THERMISTOR_SCHEMA,
    cv.Required(CONF_FET): FET_SCHEMA,
    cv.Required(CONF_BALANCING): BALANCING_SCHEMA,
    cv.Required(CONF_PROTECTIONS): PROTECTION_SCHEMA,
    cv.Required(CONF_SOC): SOC_SCHEMA,
    cv.Optional(CONF_BAT_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_PACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_LD_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_LARGEST_INTERCELL_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_CURRENT): sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_STATE_OF_CHARGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_BATTERY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_LEARNED_CAPACITY): sensor.sensor_schema(
        unit_of_measurement="Ah",
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_DIE_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_TS1_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_TS2_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_TS3_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_CONNECTION_STATE): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_STATE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_FAULT): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_CAPACITY_CALIBRATION_STATUS): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_OUTPUT_ENABLED_CONTROL): switch_.switch_schema(
        BQ76952OutputEnabledSwitch, entity_category=ENTITY_CATEGORY_CONFIG
    ),
    cv.Optional(CONF_CLEAR_ALARMS): button.button_schema(
        BQ76952ClearAlarmsButton, entity_category=ENTITY_CATEGORY_CONFIG
    ),
    cv.Optional(CONF_MANUFACTURING): MANUFACTURING_SCHEMA,
}

for key in CELL_VOLTAGE_KEYS:
    schema[cv.Optional(key)] = VOLTAGE_SENSOR_SCHEMA

CONFIG_SCHEMA = cv.All(
    cv.Schema(schema)
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x08)),
    _validate_config,
)
