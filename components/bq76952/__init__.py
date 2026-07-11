import esphome.codegen as cg
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

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["button", "sensor", "switch", "text_sensor"]

bq76952_ns = cg.esphome_ns.namespace("bq76952")
BQ76952Component = bq76952_ns.class_(
    "BQ76952Component", cg.PollingComponent, i2c.I2CDevice)
BQ76952OutputEnabledSwitch = bq76952_ns.class_(
    "BQ76952OutputEnabledSwitch", switch_.Switch)
BQ76952AutonomousFetSwitch = bq76952_ns.class_(
    "BQ76952AutonomousFetSwitch", switch_.Switch)
BQ76952ClearAlarmsButton = bq76952_ns.class_(
    "BQ76952ClearAlarmsButton", button.Button)
BQ76952ResetPassedChargeButton = bq76952_ns.class_(
    "BQ76952ResetPassedChargeButton", button.Button)
BQ76952ApplyConfigurationButton = bq76952_ns.class_(
    "BQ76952ApplyConfigurationButton", button.Button)
BQ76952ProgramFactoryOtpButton = bq76952_ns.class_(
    "BQ76952ProgramFactoryOtpButton", button.Button)

CONF_CELL_COUNT = "cell_count"
CONF_CELL_CHANNELS = "cell_channels"
CONF_OTP_AUTONOMOUS_FET_MODE = "otp_autonomous_fet_mode"
CONF_OTP_SLEEP_MODE = "otp_sleep_mode"
CONF_PREDISCHARGE_ENABLED = "predischarge_enabled"
CONF_SLEEP_CHARGE_ENABLED = "sleep_charge_enabled"
CONF_AUTONOMOUS_BALANCING_ENABLED = "autonomous_balancing_enabled"
CONF_SENSE_RESISTOR_MILLIOHM = "sense_resistor_milliohm"
CONF_CELL_UNDERVOLTAGE_LIMIT_MV = "cell_undervoltage_limit_mv"
CONF_CELL_UNDERVOLTAGE_DELAY_MS = "cell_undervoltage_delay_ms"
CONF_CELL_OVERVOLTAGE_LIMIT_MV = "cell_overvoltage_limit_mv"
CONF_CELL_OVERVOLTAGE_DELAY_MS = "cell_overvoltage_delay_ms"
CONF_NOMINAL_CAPACITY_AH = "nominal_capacity_ah"
CONF_SHORT_CIRCUIT_IN_DISCHARGE_THRESHOLD_MV = "short_circuit_in_discharge_threshold_mv"
CONF_SHORT_CIRCUIT_IN_DISCHARGE_DELAY_US = "short_circuit_in_discharge_delay_us"
CONF_SHORT_CIRCUIT_IN_DISCHARGE_RECOVERY_TIME_S = "short_circuit_in_discharge_recovery_time_s"
CONF_CHARGE_CURRENT_LIMIT_A = "charge_current_limit_a"
CONF_DISCHARGE_CURRENT_LIMIT_A = "discharge_current_limit_a"
CONF_DISCHARGE_CURRENT_LIMIT_2_A = "discharge_current_limit_2_a"
CONF_DISCHARGE_CURRENT_LIMIT_3_A = "discharge_current_limit_3_a"
CONF_CHARGE_CURRENT_DELAY_MS = "charge_current_delay_ms"
CONF_DISCHARGE_CURRENT_DELAY_MS = "discharge_current_delay_ms"
CONF_DISCHARGE_CURRENT_DELAY_2_MS = "discharge_current_delay_2_ms"
CONF_DISCHARGE_CURRENT_DELAY_3_S = "discharge_current_delay_3_s"
CONF_CURRENT_RECOVERY_TIME_S = "current_recovery_time_s"
CONF_REG0_ENABLED = "reg0_enabled"
CONF_REG1_ENABLED = "reg1_enabled"
CONF_REG1_VOLTAGE = "reg1_voltage"
CONF_PULLUP = "pullup"
CONF_BOOT_CONFIG_APPLY_DELAY = "boot_config_apply_delay"

CONF_BAT_VOLTAGE = "bat_voltage"
CONF_PACK_VOLTAGE = "pack_voltage"
CONF_LD_VOLTAGE = "ld_voltage"
CONF_LARGEST_INTERCELL_VOLTAGE = "largest_intercell_voltage"
CONF_CURRENT = "current"
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_DIE_TEMPERATURE = "die_temperature"
CONF_TS1_TEMPERATURE = "ts1_temperature"
CONF_TS2_TEMPERATURE = "ts2_temperature"
CONF_TS3_TEMPERATURE = "ts3_temperature"

CONF_BMS_STATE = "bms_state"
CONF_FAULT = "fault"
CONF_FET_STATUS_FLAGS = "fet_status_flags"

CONF_OUTPUT_ENABLED_CONTROL = "output_enabled_control"
CONF_AUTONOMOUS_FET_CONTROL = "autonomous_fet_control"
CONF_CLEAR_ALARMS = "clear_alarms"
CONF_RESET_PASSED_CHARGE = "reset_passed_charge"
CONF_PROGRAM_FACTORY_OTP = "program_factory_otp"

CELL_VOLTAGE_KEYS = [f"cell{index}_voltage" for index in range(1, 17)]

AUTONOMOUS_FET_MODE_OPTIONS = {
    "preserve": 0,
    "enable": 1,
    "disable": 2,
}

SLEEP_MODE_OPTIONS = {
    "preserve": 0,
    "enable": 1,
    "disable": 2,
}

REG1_VOLTAGE_OPTIONS = {
    "1.8v": 0x00,
    "2.5v": 0x08,
    "3.0v": 0x0A,
    "3.3v": 0x0C,
    "5.0v": 0x0E,
}

TS_PULLUP_OPTIONS = {
    "18k": False,
    "180k": True,
}

SCD_THRESHOLD_OPTIONS = {
    10: 0,
    20: 1,
    40: 2,
    60: 3,
    80: 4,
    100: 5,
    125: 6,
    150: 7,
    175: 8,
    200: 9,
    250: 10,
    300: 11,
    350: 12,
    400: 13,
    450: 14,
    500: 15,
}


def _validate_scd_delay_us(value):
    if value == 0:
        return value
    if 15 <= value <= 450 and value % 15 == 0:
        return value
    raise cv.Invalid(
        "scd_delay_us must be 0 or a multiple of 15 from 15 to 450")


VOLTAGE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)


def _validate_config(config):
    cell_count = config[CONF_CELL_COUNT]
    if CONF_CELL_CHANNELS in config:
        channels = config[CONF_CELL_CHANNELS]
        if len(channels) != cell_count:
            raise cv.Invalid(
                f"cell_channels must contain exactly {cell_count} entries"
            )
        if len(set(channels)) != len(channels):
            raise cv.Invalid("cell_channels entries must be unique")

    for index, key in enumerate(CELL_VOLTAGE_KEYS, start=1):
        if key in config and index > cell_count:
            raise cv.Invalid(f"{key} requires cell_count >= {index}")

    for key in (CONF_TS1_TEMPERATURE, CONF_TS2_TEMPERATURE, CONF_TS3_TEMPERATURE):
        if key in config and isinstance(config[key], dict):
            continue

    return config


schema = {
    cv.GenerateID(): cv.declare_id(BQ76952Component),
    cv.Optional(CONF_CELL_COUNT, default=16): cv.int_range(min=3, max=16),
    cv.Optional(CONF_CELL_CHANNELS): cv.ensure_list(cv.int_range(min=1, max=16)),
    cv.Optional(CONF_OTP_AUTONOMOUS_FET_MODE, default="preserve"): cv.enum(
        AUTONOMOUS_FET_MODE_OPTIONS, lower=True
    ),
    cv.Optional(CONF_OTP_SLEEP_MODE, default="preserve"): cv.enum(SLEEP_MODE_OPTIONS, lower=True),
    cv.Optional(CONF_PREDISCHARGE_ENABLED): cv.boolean,
    cv.Optional(CONF_SLEEP_CHARGE_ENABLED): cv.boolean,
    cv.Optional(CONF_AUTONOMOUS_BALANCING_ENABLED): cv.boolean,
    cv.Optional(CONF_SENSE_RESISTOR_MILLIOHM, default=1.0): cv.float_range(min=0.001),
    cv.Optional(CONF_CELL_UNDERVOLTAGE_LIMIT_MV): cv.int_range(min=1000, max=5000),
    cv.Optional(CONF_CELL_UNDERVOLTAGE_DELAY_MS): cv.int_range(min=1, max=7000),
    cv.Optional(CONF_CELL_OVERVOLTAGE_LIMIT_MV): cv.int_range(min=1000, max=6000),
    cv.Optional(CONF_CELL_OVERVOLTAGE_DELAY_MS): cv.int_range(min=1, max=7000),
    cv.Optional(CONF_NOMINAL_CAPACITY_AH): cv.float_range(min=0.001),
    cv.Optional(CONF_SHORT_CIRCUIT_IN_DISCHARGE_THRESHOLD_MV): cv.one_of(
        *SCD_THRESHOLD_OPTIONS.keys(), int=True
    ),
    cv.Optional(CONF_SHORT_CIRCUIT_IN_DISCHARGE_DELAY_US): _validate_scd_delay_us,
    cv.Optional(CONF_SHORT_CIRCUIT_IN_DISCHARGE_RECOVERY_TIME_S): cv.int_range(min=0, max=255),
    cv.Optional(CONF_CHARGE_CURRENT_LIMIT_A): cv.float_range(min=0.001),
    cv.Optional(CONF_DISCHARGE_CURRENT_LIMIT_A): cv.float_range(min=0.001),
    cv.Optional(CONF_DISCHARGE_CURRENT_LIMIT_2_A): cv.float_range(min=0.001),
    cv.Optional(CONF_DISCHARGE_CURRENT_LIMIT_3_A): cv.float_range(min=0.001),
    cv.Optional(CONF_CHARGE_CURRENT_DELAY_MS): cv.int_range(min=10, max=426),
    cv.Optional(CONF_DISCHARGE_CURRENT_DELAY_MS): cv.int_range(min=10, max=426),
    cv.Optional(CONF_DISCHARGE_CURRENT_DELAY_2_MS): cv.int_range(min=10, max=426),
    cv.Optional(CONF_DISCHARGE_CURRENT_DELAY_3_S): cv.int_range(min=0, max=255),
    cv.Optional(CONF_CURRENT_RECOVERY_TIME_S): cv.int_range(min=0, max=255),
    cv.Optional(CONF_REG0_ENABLED): cv.boolean,
    cv.Optional(CONF_REG1_ENABLED): cv.boolean,
    cv.Optional(CONF_REG1_VOLTAGE): cv.enum(REG1_VOLTAGE_OPTIONS, lower=True),
    cv.Optional(CONF_BOOT_CONFIG_APPLY_DELAY, default="10s"): cv.positive_time_period_milliseconds,

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
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_TS2_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_TS3_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_BMS_STATE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_FAULT): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_FET_STATUS_FLAGS): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_OUTPUT_ENABLED_CONTROL): switch_.switch_schema(
        BQ76952OutputEnabledSwitch, entity_category=ENTITY_CATEGORY_CONFIG
    ),
    cv.Optional(CONF_AUTONOMOUS_FET_CONTROL): switch_.switch_schema(
        BQ76952AutonomousFetSwitch,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_CLEAR_ALARMS): button.button_schema(
        BQ76952ClearAlarmsButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_RESET_PASSED_CHARGE): button.button_schema(
        BQ76952ResetPassedChargeButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_PROGRAM_FACTORY_OTP): button.button_schema(
        BQ76952ProgramFactoryOtpButton,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}

for key in CELL_VOLTAGE_KEYS:
    schema[cv.Optional(key)] = VOLTAGE_SENSOR_SCHEMA


CONFIG_SCHEMA = cv.All(
    cv.Schema(schema)
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x08)),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_cell_count(config[CONF_CELL_COUNT]))
    if CONF_CELL_CHANNELS in config:
        for index, channel in enumerate(config[CONF_CELL_CHANNELS], start=1):
            cg.add(var.set_cell_channel(index, channel))
    cg.add(var.set_sense_resistor_milliohm(
        config[CONF_SENSE_RESISTOR_MILLIOHM]))
    cg.add(var.set_autonomous_fet_mode(config[CONF_OTP_AUTONOMOUS_FET_MODE]))
    cg.add(var.set_sleep_mode(config[CONF_OTP_SLEEP_MODE]))
    if CONF_PREDISCHARGE_ENABLED in config:
        cg.add(var.set_predischarge_enabled(config[CONF_PREDISCHARGE_ENABLED]))
    if CONF_SLEEP_CHARGE_ENABLED in config:
        cg.add(var.set_sleep_charge_enabled(config[CONF_SLEEP_CHARGE_ENABLED]))
    if CONF_AUTONOMOUS_BALANCING_ENABLED in config:
        cg.add(var.set_autonomous_balancing_enabled(
            config[CONF_AUTONOMOUS_BALANCING_ENABLED]))
    if CONF_CELL_UNDERVOLTAGE_LIMIT_MV in config:
        cg.add(var.set_cell_undervoltage_limit_mv(
            config[CONF_CELL_UNDERVOLTAGE_LIMIT_MV]))
    if CONF_CELL_UNDERVOLTAGE_DELAY_MS in config:
        cg.add(var.set_cell_undervoltage_delay_ms(
            config[CONF_CELL_UNDERVOLTAGE_DELAY_MS]))
    if CONF_CELL_OVERVOLTAGE_LIMIT_MV in config:
        cg.add(var.set_cell_overvoltage_limit_mv(
            config[CONF_CELL_OVERVOLTAGE_LIMIT_MV]))
    if CONF_CELL_OVERVOLTAGE_DELAY_MS in config:
        cg.add(var.set_cell_overvoltage_delay_ms(
            config[CONF_CELL_OVERVOLTAGE_DELAY_MS]))
    if CONF_SHORT_CIRCUIT_IN_DISCHARGE_THRESHOLD_MV in config:
        cg.add(
            var.set_scd_threshold_mv(
                config[CONF_SHORT_CIRCUIT_IN_DISCHARGE_THRESHOLD_MV]
            )
        )
    if CONF_SHORT_CIRCUIT_IN_DISCHARGE_DELAY_US in config:
        cg.add(
            var.set_scd_delay_us(config[CONF_SHORT_CIRCUIT_IN_DISCHARGE_DELAY_US])
        )
    if CONF_SHORT_CIRCUIT_IN_DISCHARGE_RECOVERY_TIME_S in config:
        cg.add(
            var.set_scd_recovery_time_s(
                config[CONF_SHORT_CIRCUIT_IN_DISCHARGE_RECOVERY_TIME_S]
            )
        )
    if CONF_CHARGE_CURRENT_LIMIT_A in config:
        cg.add(var.set_charge_current_limit_a(config[CONF_CHARGE_CURRENT_LIMIT_A]))
    if CONF_DISCHARGE_CURRENT_LIMIT_A in config:
        cg.add(var.set_discharge_current_limit_a(config[CONF_DISCHARGE_CURRENT_LIMIT_A]))
    if CONF_DISCHARGE_CURRENT_LIMIT_2_A in config:
        cg.add(var.set_discharge_current_limit_2_a(config[CONF_DISCHARGE_CURRENT_LIMIT_2_A]))
    if CONF_DISCHARGE_CURRENT_LIMIT_3_A in config:
        cg.add(var.set_discharge_current_limit_3_a(config[CONF_DISCHARGE_CURRENT_LIMIT_3_A]))
    if CONF_CHARGE_CURRENT_DELAY_MS in config:
        cg.add(var.set_charge_current_delay_ms(config[CONF_CHARGE_CURRENT_DELAY_MS]))
    if CONF_DISCHARGE_CURRENT_DELAY_MS in config:
        cg.add(var.set_discharge_current_delay_ms(config[CONF_DISCHARGE_CURRENT_DELAY_MS]))
    if CONF_DISCHARGE_CURRENT_DELAY_2_MS in config:
        cg.add(var.set_discharge_current_delay_2_ms(config[CONF_DISCHARGE_CURRENT_DELAY_2_MS]))
    if CONF_DISCHARGE_CURRENT_DELAY_3_S in config:
        cg.add(var.set_discharge_current_delay_3_s(config[CONF_DISCHARGE_CURRENT_DELAY_3_S]))
    if CONF_CURRENT_RECOVERY_TIME_S in config:
        cg.add(var.set_current_recovery_time_s(config[CONF_CURRENT_RECOVERY_TIME_S]))
    if CONF_REG0_ENABLED in config:
        cg.add(var.set_reg0_enabled(config[CONF_REG0_ENABLED]))
    if CONF_REG1_ENABLED in config:
        cg.add(var.set_reg1_enabled(config[CONF_REG1_ENABLED]))
    elif CONF_REG1_VOLTAGE in config:
        cg.add(var.set_reg1_enabled(True))
    if CONF_REG1_VOLTAGE in config:
        cg.add(var.set_reg1_voltage(config[CONF_REG1_VOLTAGE]))
    cg.add(var.set_boot_config_apply_delay_ms(config[CONF_BOOT_CONFIG_APPLY_DELAY].total_milliseconds))
    if CONF_NOMINAL_CAPACITY_AH in config:
        cg.add(var.set_nominal_capacity_ah(config[CONF_NOMINAL_CAPACITY_AH]))

    if CONF_BAT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BAT_VOLTAGE])
        cg.add(var.set_bat_voltage_sensor(sens))
    if CONF_PACK_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_PACK_VOLTAGE])
        cg.add(var.set_pack_voltage_sensor(sens))
    if CONF_LD_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_LD_VOLTAGE])
        cg.add(var.set_ld_voltage_sensor(sens))
    if CONF_LARGEST_INTERCELL_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_LARGEST_INTERCELL_VOLTAGE])
        cg.add(var.set_largest_intercell_voltage_sensor(sens))

    for index, key in enumerate(CELL_VOLTAGE_KEYS, start=1):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(var.set_cell_voltage_sensor(index, sens))

    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_STATE_OF_CHARGE in config:
        sens = await sensor.new_sensor(config[CONF_STATE_OF_CHARGE])
        cg.add(var.set_state_of_charge_sensor(sens))
    if CONF_DIE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_DIE_TEMPERATURE])
        cg.add(var.set_die_temperature_sensor(sens))
    if CONF_TS1_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS1_TEMPERATURE])
        cg.add(var.set_ts1_temperature_sensor(sens))
        cg.add(var.set_ts1_pullup_180k(config[CONF_TS1_TEMPERATURE][CONF_PULLUP]))
    if CONF_TS2_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS2_TEMPERATURE])
        cg.add(var.set_ts2_temperature_sensor(sens))
        cg.add(var.set_ts2_pullup_180k(config[CONF_TS2_TEMPERATURE][CONF_PULLUP]))
    if CONF_TS3_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS3_TEMPERATURE])
        cg.add(var.set_ts3_temperature_sensor(sens))
        cg.add(var.set_ts3_pullup_180k(config[CONF_TS3_TEMPERATURE][CONF_PULLUP]))

    if CONF_BMS_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_BMS_STATE])
        cg.add(var.set_bms_state_sensor(ts))
    if CONF_FAULT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_FAULT])
        cg.add(var.set_fault_sensor(ts))
    if CONF_FET_STATUS_FLAGS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_FET_STATUS_FLAGS])
        cg.add(var.set_fet_status_flags_sensor(ts))

    if CONF_OUTPUT_ENABLED_CONTROL in config:
        sw = await switch_.new_switch(config[CONF_OUTPUT_ENABLED_CONTROL])
        await cg.register_parented(sw, var)
        cg.add(var.set_output_enabled_switch(sw))

    if CONF_AUTONOMOUS_FET_CONTROL in config:
        sw = await switch_.new_switch(config[CONF_AUTONOMOUS_FET_CONTROL])
        await cg.register_parented(sw, var)
        cg.add(var.set_autonomous_fet_switch(sw))
    if CONF_CLEAR_ALARMS in config:
        btn = await button.new_button(config[CONF_CLEAR_ALARMS])
        await cg.register_parented(btn, var)
    if CONF_RESET_PASSED_CHARGE in config:
        btn = await button.new_button(config[CONF_RESET_PASSED_CHARGE])
        await cg.register_parented(btn, var)
    if CONF_PROGRAM_FACTORY_OTP in config:
        btn = await button.new_button(config[CONF_PROGRAM_FACTORY_OTP])
        await cg.register_parented(btn, var)
