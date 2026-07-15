import esphome.codegen as cg
from esphome.components import button, i2c, sensor, switch as switch_, text_sensor

from . import _schema as schema
from . import _types as types


def _struct(type_, config, fields):
    return cg.StructInitializer(
        type_, *((field, config[key]) for field, key in fields)
    )


def _build_cpp_config(config):
    regulators = _struct(
        types.BQ76952RegulatorConfig,
        config[schema.CONF_REGULATORS],
        (
            ("reg0_enabled", schema.CONF_REG0_ENABLED),
            ("reg1_enabled", schema.CONF_REG1_ENABLED),
            ("reg1_voltage_code", schema.CONF_REG1_VOLTAGE),
            ("reg2_enabled", schema.CONF_REG2_ENABLED),
            ("reg2_voltage_code", schema.CONF_REG2_VOLTAGE),
        ),
    )
    thermistors = _struct(
        types.BQ76952ThermistorConfig,
        config[schema.CONF_THERMISTORS],
        (("ts1", schema.CONF_TS1), ("ts2", schema.CONF_TS2), ("ts3", schema.CONF_TS3)),
    )
    precharge = _struct(
        types.BQ76952PrechargeConfig,
        config[schema.CONF_FET][schema.CONF_PRECHARGE],
        (
            ("enabled", schema.CONF_ENABLED),
            ("start_cell_voltage_mv", schema.CONF_START_CELL_VOLTAGE_MV),
            ("stop_cell_voltage_mv", schema.CONF_STOP_CELL_VOLTAGE_MV),
        ),
    )
    predischarge = _struct(
        types.BQ76952PredischargeConfig,
        config[schema.CONF_FET][schema.CONF_PREDISCHARGE],
        (
            ("enabled", schema.CONF_ENABLED),
            ("timeout_ms", schema.CONF_TIMEOUT_MS),
            ("stop_delta_mv", schema.CONF_STOP_DELTA_MV),
        ),
    )
    fet = cg.StructInitializer(
        types.BQ76952FetConfig,
        ("autonomous", config[schema.CONF_FET][schema.CONF_AUTONOMOUS]),
        ("sleep_charge_enabled", config[schema.CONF_FET][schema.CONF_SLEEP_CHARGE_ENABLED]),
        ("precharge", precharge),
        ("predischarge", predischarge),
    )
    balancing = _struct(
        types.BQ76952BalancingConfig,
        config[schema.CONF_BALANCING],
        (
            ("minimum_cell_voltage_mv", schema.CONF_MINIMUM_CELL_VOLTAGE_MV),
            ("start_delta_mv", schema.CONF_START_DELTA_MV),
            ("stop_delta_mv", schema.CONF_STOP_DELTA_MV),
            ("minimum_temperature_c", schema.CONF_MINIMUM_TEMPERATURE_C),
            ("maximum_temperature_c", schema.CONF_MAXIMUM_TEMPERATURE_C),
            ("maximum_balanced_cells", schema.CONF_MAXIMUM_BALANCED_CELLS),
        ),
    )

    def cell_voltage(name):
        return _struct(
            types.BQ76952CellVoltageProtectionConfig,
            config[schema.CONF_PROTECTIONS][name],
            (
                ("threshold_mv", schema.CONF_THRESHOLD_MV),
                ("delay_ms", schema.CONF_DELAY_MS),
                ("recovery_hysteresis_mv", schema.CONF_RECOVERY_HYSTERESIS_MV),
            ),
        )

    def current(name):
        return _struct(
            types.BQ76952CurrentProtectionConfig,
            config[schema.CONF_PROTECTIONS][name],
            (("threshold_a", schema.CONF_THRESHOLD_A), ("delay_ms", schema.CONF_DELAY_MS)),
        )

    sustained = _struct(
        types.BQ76952SustainedCurrentProtectionConfig,
        config[schema.CONF_PROTECTIONS][schema.CONF_DISCHARGE_SUSTAINED_OVERCURRENT],
        (("threshold_a", schema.CONF_THRESHOLD_A), ("delay_s", schema.CONF_DELAY_S)),
    )
    short_circuit = _struct(
        types.BQ76952ShortCircuitProtectionConfig,
        config[schema.CONF_PROTECTIONS][schema.CONF_DISCHARGE_SHORT_CIRCUIT],
        (
            ("threshold_mv", schema.CONF_THRESHOLD_MV),
            ("delay_us", schema.CONF_DELAY_US),
            ("recovery_time_s", schema.CONF_RECOVERY_TIME_S),
        ),
    )
    temperature = _struct(
        types.BQ76952TemperatureProtectionConfig,
        config[schema.CONF_PROTECTIONS][schema.CONF_TEMPERATURE],
        (
            ("charge_minimum_c", schema.CONF_CHARGE_MINIMUM_C),
            ("charge_maximum_c", schema.CONF_CHARGE_MAXIMUM_C),
            ("discharge_minimum_c", schema.CONF_DISCHARGE_MINIMUM_C),
            ("discharge_maximum_c", schema.CONF_DISCHARGE_MAXIMUM_C),
            ("recovery_hysteresis_c", schema.CONF_RECOVERY_HYSTERESIS_C),
        ),
    )
    protections = cg.StructInitializer(
        types.BQ76952ProtectionConfig,
        ("cell_undervoltage", cell_voltage(schema.CONF_CELL_UNDERVOLTAGE)),
        ("cell_overvoltage", cell_voltage(schema.CONF_CELL_OVERVOLTAGE)),
        ("charge_overcurrent", current(schema.CONF_CHARGE_OVERCURRENT)),
        ("discharge_overcurrent", current(schema.CONF_DISCHARGE_OVERCURRENT)),
        (
            "discharge_severe_overcurrent",
            current(schema.CONF_DISCHARGE_SEVERE_OVERCURRENT),
        ),
        ("discharge_sustained_overcurrent", sustained),
        ("discharge_short_circuit", short_circuit),
        ("temperature", temperature),
        (
            "current_recovery_time_s",
            config[schema.CONF_PROTECTIONS][schema.CONF_CURRENT_RECOVERY_TIME_S],
        ),
    )
    soc = _struct(
        types.BQ76952SocConfig,
        config[schema.CONF_SOC],
        (
            ("empty_cell_voltage_mv", schema.CONF_EMPTY_CELL_VOLTAGE_MV),
            ("full_cell_voltage_mv", schema.CONF_FULL_CELL_VOLTAGE_MV),
        ),
    )

    return cg.StructInitializer(
        types.BQ76952Config,
        ("cell_count", config[schema.CONF_CELL_COUNT]),
        ("cell_chemistry", config[schema.CONF_CELL_CHEMISTRY]),
        ("sense_resistor_milliohm", config[schema.CONF_SENSE_RESISTOR_MILLIOHM]),
        ("i2c_crc_enabled", config[schema.CONF_I2C_CRC_ENABLED]),
        ("current_gain_policy", config[schema.CONF_CURRENT_GAIN_POLICY]),
        ("regulators", regulators),
        ("thermistors", thermistors),
        ("fet", fet),
        ("balancing", balancing),
        ("protections", protections),
        ("soc", soc),
    )


async def to_code(config):
    var = cg.new_Pvariable(config[schema.CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_config(_build_cpp_config(config)))

    sensor_setters = (
        (schema.CONF_BAT_VOLTAGE, var.set_bat_voltage_sensor),
        (schema.CONF_PACK_VOLTAGE, var.set_pack_voltage_sensor),
        (schema.CONF_LD_VOLTAGE, var.set_ld_voltage_sensor),
        (
            schema.CONF_LARGEST_INTERCELL_VOLTAGE,
            var.set_largest_intercell_voltage_sensor,
        ),
        (schema.CONF_CURRENT, var.set_current_sensor),
        (schema.CONF_STATE_OF_CHARGE, var.set_state_of_charge_sensor),
        (schema.CONF_LEARNED_CAPACITY, var.set_learned_capacity_sensor),
        (schema.CONF_DIE_TEMPERATURE, var.set_die_temperature_sensor),
        (schema.CONF_TS1_TEMPERATURE, var.set_ts1_temperature_sensor),
        (schema.CONF_TS2_TEMPERATURE, var.set_ts2_temperature_sensor),
        (schema.CONF_TS3_TEMPERATURE, var.set_ts3_temperature_sensor),
    )
    for key, setter in sensor_setters:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))

    for index, key in enumerate(schema.CELL_VOLTAGE_KEYS, start=1):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(var.set_cell_voltage_sensor(index, sens))

    text_sensor_setters = (
        (schema.CONF_LIFECYCLE, var.set_lifecycle_sensor),
        (schema.CONF_STATE, var.set_state_sensor),
        (schema.CONF_FAULT, var.set_fault_sensor),
        (schema.CONF_FAULT_FLAGS, var.set_fault_flags_sensor),
        (
            schema.CONF_CAPACITY_CALIBRATION_STATUS,
            var.set_capacity_calibration_status_sensor,
        ),
    )
    for key, setter in text_sensor_setters:
        if key in config:
            text = await text_sensor.new_text_sensor(config[key])
            cg.add(setter(text))

    if schema.CONF_OUTPUT_ENABLED_CONTROL in config:
        control = await switch_.new_switch(config[schema.CONF_OUTPUT_ENABLED_CONTROL])
        await cg.register_parented(control, var)
        cg.add(var.set_output_enabled_switch(control))
    if schema.CONF_CLEAR_ALARMS in config:
        control = await button.new_button(config[schema.CONF_CLEAR_ALARMS])
        await cg.register_parented(control, var)
    if schema.CONF_MANUFACTURING in config:
        manufacturing = config[schema.CONF_MANUFACTURING]
        control = await button.new_button(manufacturing[schema.CONF_PROGRAM_FACTORY_OTP])
        await cg.register_parented(control, var)
