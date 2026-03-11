import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import ENTITY_CATEGORY_CONFIG, UNIT_PERCENT

from . import MCF8316DManualComponent, mcf8316d_manual_ns

CONF_MCF8316D_MANUAL_ID = "mcf8316d_manual_id"

CONF_SPEED = "speed"
CONF_MOTOR_RES = "motor_res"
CONF_MOTOR_IND = "motor_ind"
CONF_MOTOR_BEMF_CONST = "motor_bemf_const"
CONF_SPEED_LOOP_KP = "speed_loop_kp"
CONF_SPEED_LOOP_KI = "speed_loop_ki"
CONF_MAX_SPEED = "max_speed"

MCF8316DNumber = mcf8316d_manual_ns.class_("MCF8316DNumber", number.Number)
MotorTuneParameter = mcf8316d_manual_ns.enum("MotorTuneParameter")

PARAMETER_MAP = {
    CONF_MOTOR_RES: MotorTuneParameter.MOTOR_RES,
    CONF_MOTOR_IND: MotorTuneParameter.MOTOR_IND,
    CONF_MOTOR_BEMF_CONST: MotorTuneParameter.MOTOR_BEMF_CONST,
    CONF_SPEED_LOOP_KP: MotorTuneParameter.SPEED_LOOP_KP,
    CONF_SPEED_LOOP_KI: MotorTuneParameter.SPEED_LOOP_KI,
    CONF_MAX_SPEED: MotorTuneParameter.MAX_SPEED,
}


def _number_schema(unit=None, *, entity_category=ENTITY_CATEGORY_CONFIG):
    kwargs = {"entity_category": entity_category}
    if unit is not None:
        kwargs["unit_of_measurement"] = unit
    return number.number_schema(MCF8316DNumber, **kwargs)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8316D_MANUAL_ID): cv.use_id(MCF8316DManualComponent),
        cv.Required(CONF_SPEED): _number_schema(UNIT_PERCENT, entity_category=None),
        cv.Optional(CONF_MOTOR_RES): _number_schema(),
        cv.Optional(CONF_MOTOR_IND): _number_schema(),
        cv.Optional(CONF_MOTOR_BEMF_CONST): _number_schema(),
        cv.Optional(CONF_SPEED_LOOP_KP): _number_schema(),
        cv.Optional(CONF_SPEED_LOOP_KI): _number_schema(),
        cv.Optional(CONF_MAX_SPEED): _number_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8316D_MANUAL_ID])

    speed = await number.new_number(
        config[CONF_SPEED],
        min_value=0,
        max_value=100,
        step=1,
    )
    cg.add(speed.set_parent(parent))
    cg.add(speed.set_is_speed_command(True))
    cg.add(parent.set_speed_number(speed))

    for key, enum_value in PARAMETER_MAP.items():
        if key not in config:
            continue
        tune = await number.new_number(
            config[key],
            min_value=0,
            max_value=0x3FFF if key == CONF_MAX_SPEED else 0x3FF,
            step=1,
        )
        cg.add(tune.set_parent(parent))
        cg.add(tune.set_is_speed_command(False))
        cg.add(tune.set_tune_parameter(enum_value))
