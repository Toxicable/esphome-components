import esphome.codegen as cg
from esphome.components import button, number, sensor, text_sensor
from esphome.const import CONF_ID

from ._types import *


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

    if CONF_CALIBRATION_STATUS in calibration:
        value = await text_sensor.new_text_sensor(
            calibration[CONF_CALIBRATION_STATUS]
        )
        cg.add(var.set_calibration_status_sensor(value))
    if CONF_CURRENT_SCALE in calibration:
        value = await sensor.new_sensor(calibration[CONF_CURRENT_SCALE])
        cg.add(var.set_current_scale_sensor(value))
    if CONF_CURRENT_OFFSET in calibration:
        value = await sensor.new_sensor(calibration[CONF_CURRENT_OFFSET])
        cg.add(var.set_current_offset_sensor(value))
    if CONF_VOLTAGE_SCALE in calibration:
        value = await sensor.new_sensor(calibration[CONF_VOLTAGE_SCALE])
        cg.add(var.set_voltage_scale_sensor(value))
    if CONF_VOLTAGE_OFFSET in calibration:
        value = await sensor.new_sensor(calibration[CONF_VOLTAGE_OFFSET])
        cg.add(var.set_voltage_offset_sensor(value))
    if CONF_OUTPUT_ZERO_LEVEL in calibration:
        value = await sensor.new_sensor(calibration[CONF_OUTPUT_ZERO_LEVEL])
        cg.add(var.set_output_zero_level_sensor(value))
    if CONF_OUTPUT_FULL_SCALE_CURRENT in calibration:
        value = await sensor.new_sensor(
            calibration[CONF_OUTPUT_FULL_SCALE_CURRENT]
        )
        cg.add(var.set_output_full_scale_current_sensor(value))
    if CONF_RESET_CALIBRATION in calibration:
        value = await button.new_button(calibration[CONF_RESET_CALIBRATION])
        cg.add(value.set_parent(var))

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
