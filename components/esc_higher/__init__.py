import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, i2c, sensor
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["button", "sensor"]

esc_higher_ns = cg.esphome_ns.namespace("esc_higher")
ESCHigherComponent = esc_higher_ns.class_(
    "ESCHigherComponent", cg.PollingComponent, i2c.I2CDevice
)
ESCHigherStartButton = esc_higher_ns.class_("ESCHigherStartButton", button.Button)
ESCHigherStopButton = esc_higher_ns.class_("ESCHigherStopButton", button.Button)
ESCHigherClearFaultsButton = esc_higher_ns.class_(
    "ESCHigherClearFaultsButton", button.Button
)
ESCHigherEstopButton = esc_higher_ns.class_("ESCHigherEstopButton", button.Button)
ESCHigherSetSpeedRampButton = esc_higher_ns.class_(
    "ESCHigherSetSpeedRampButton", button.Button
)

CONF_PROTO_MAJOR = "proto_major"
CONF_PROTO_MINOR = "proto_minor"
CONF_FW_MAJOR = "fw_major"
CONF_FW_MINOR = "fw_minor"
CONF_HW_ID = "hw_id"
CONF_MAX_BLOCK_LEN = "max_block_len"
CONF_CAPABILITIES = "capabilities"
CONF_SEQ = "seq"
CONF_ESC_STATE = "esc_state"
CONF_MC_STATE = "mc_state"
CONF_LAST_CMD_SEQ = "last_cmd_seq"
CONF_LAST_CMD_ERROR = "last_cmd_error"
CONF_CURRENT_FAULTS = "current_faults"
CONF_OCCURRED_FAULTS = "occurred_faults"
CONF_STATUS_FLAGS = "status_flags"
CONF_WATCHDOG_MS_LEFT = "watchdog_ms_left"
CONF_VBUS_MV = "vbus_mv"
CONF_IBUS_MA = "ibus_ma"
CONF_SPEED_DHZ = "speed_dhz"
CONF_DUTY_CENTI_PCT = "duty_centi_pct"
CONF_TEMP_MC = "temp_mc"
CONF_UPTIME_S = "uptime_s"
CONF_START_MOTOR = "start_motor"
CONF_STOP_MOTOR = "stop_motor"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_ESTOP = "estop"
CONF_SET_SPEED_RAMP = "set_speed_ramp"
CONF_SPEED_RAMP_TARGET_DHZ = "speed_ramp_target_dhz"
CONF_SPEED_RAMP_TIME_MS = "speed_ramp_time_ms"


def _raw_sensor_schema():
    return sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESCHigherComponent),
            cv.Optional(CONF_PROTO_MAJOR): _raw_sensor_schema(),
            cv.Optional(CONF_PROTO_MINOR): _raw_sensor_schema(),
            cv.Optional(CONF_FW_MAJOR): _raw_sensor_schema(),
            cv.Optional(CONF_FW_MINOR): _raw_sensor_schema(),
            cv.Optional(CONF_HW_ID): _raw_sensor_schema(),
            cv.Optional(CONF_MAX_BLOCK_LEN): _raw_sensor_schema(),
            cv.Optional(CONF_CAPABILITIES): _raw_sensor_schema(),
            cv.Optional(CONF_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_ESC_STATE): _raw_sensor_schema(),
            cv.Optional(CONF_MC_STATE): _raw_sensor_schema(),
            cv.Optional(CONF_LAST_CMD_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_LAST_CMD_ERROR): _raw_sensor_schema(),
            cv.Optional(CONF_CURRENT_FAULTS): _raw_sensor_schema(),
            cv.Optional(CONF_OCCURRED_FAULTS): _raw_sensor_schema(),
            cv.Optional(CONF_STATUS_FLAGS): _raw_sensor_schema(),
            cv.Optional(CONF_WATCHDOG_MS_LEFT): _raw_sensor_schema(),
            cv.Optional(CONF_VBUS_MV): _raw_sensor_schema(),
            cv.Optional(CONF_IBUS_MA): _raw_sensor_schema(),
            cv.Optional(CONF_SPEED_DHZ): _raw_sensor_schema(),
            cv.Optional(CONF_DUTY_CENTI_PCT): _raw_sensor_schema(),
            cv.Optional(CONF_TEMP_MC): _raw_sensor_schema(),
            cv.Optional(CONF_UPTIME_S): _raw_sensor_schema(),
            cv.Optional(CONF_START_MOTOR): button.button_schema(
                ESCHigherStartButton, icon="mdi:play"
            ),
            cv.Optional(CONF_STOP_MOTOR): button.button_schema(
                ESCHigherStopButton, icon="mdi:stop"
            ),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(
                ESCHigherClearFaultsButton, icon="mdi:alert-remove"
            ),
            cv.Optional(CONF_ESTOP): button.button_schema(
                ESCHigherEstopButton, icon="mdi:alert-octagon"
            ),
            cv.Optional(CONF_SET_SPEED_RAMP): button.button_schema(
                ESCHigherSetSpeedRampButton, icon="mdi:ramp-right"
            ),
            cv.Optional(CONF_SPEED_RAMP_TARGET_DHZ, default=1000): cv.int_,
            cv.Optional(CONF_SPEED_RAMP_TIME_MS, default=1000): cv.int_range(
                min=0, max=2147483647
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x34))
)


async def _bind_sensor(config, key, setter):
    if key in config:
        s = await sensor.new_sensor(config[key])
        cg.add(setter(s))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_speed_ramp_target_dhz(config[CONF_SPEED_RAMP_TARGET_DHZ]))
    cg.add(var.set_speed_ramp_time_ms(config[CONF_SPEED_RAMP_TIME_MS]))

    await _bind_sensor(config, CONF_PROTO_MAJOR, var.set_proto_major_sensor)
    await _bind_sensor(config, CONF_PROTO_MINOR, var.set_proto_minor_sensor)
    await _bind_sensor(config, CONF_FW_MAJOR, var.set_fw_major_sensor)
    await _bind_sensor(config, CONF_FW_MINOR, var.set_fw_minor_sensor)
    await _bind_sensor(config, CONF_HW_ID, var.set_hw_id_sensor)
    await _bind_sensor(config, CONF_MAX_BLOCK_LEN, var.set_max_block_len_sensor)
    await _bind_sensor(config, CONF_CAPABILITIES, var.set_capabilities_sensor)
    await _bind_sensor(config, CONF_SEQ, var.set_seq_sensor)
    await _bind_sensor(config, CONF_ESC_STATE, var.set_esc_state_sensor)
    await _bind_sensor(config, CONF_MC_STATE, var.set_mc_state_sensor)
    await _bind_sensor(config, CONF_LAST_CMD_SEQ, var.set_last_cmd_seq_sensor)
    await _bind_sensor(config, CONF_LAST_CMD_ERROR, var.set_last_cmd_error_sensor)
    await _bind_sensor(config, CONF_CURRENT_FAULTS, var.set_current_faults_sensor)
    await _bind_sensor(config, CONF_OCCURRED_FAULTS, var.set_occurred_faults_sensor)
    await _bind_sensor(config, CONF_STATUS_FLAGS, var.set_status_flags_sensor)
    await _bind_sensor(config, CONF_WATCHDOG_MS_LEFT, var.set_watchdog_ms_left_sensor)
    await _bind_sensor(config, CONF_VBUS_MV, var.set_vbus_mv_sensor)
    await _bind_sensor(config, CONF_IBUS_MA, var.set_ibus_ma_sensor)
    await _bind_sensor(config, CONF_SPEED_DHZ, var.set_speed_dhz_sensor)
    await _bind_sensor(config, CONF_DUTY_CENTI_PCT, var.set_duty_centi_pct_sensor)
    await _bind_sensor(config, CONF_TEMP_MC, var.set_temp_mc_sensor)
    await _bind_sensor(config, CONF_UPTIME_S, var.set_uptime_s_sensor)

    if CONF_START_MOTOR in config:
        b = await button.new_button(config[CONF_START_MOTOR])
        await cg.register_parented(b, var)
    if CONF_STOP_MOTOR in config:
        b = await button.new_button(config[CONF_STOP_MOTOR])
        await cg.register_parented(b, var)
    if CONF_CLEAR_FAULTS in config:
        b = await button.new_button(config[CONF_CLEAR_FAULTS])
        await cg.register_parented(b, var)
    if CONF_ESTOP in config:
        b = await button.new_button(config[CONF_ESTOP])
        await cg.register_parented(b, var)
    if CONF_SET_SPEED_RAMP in config:
        b = await button.new_button(config[CONF_SET_SPEED_RAMP])
        await cg.register_parented(b, var)
