import esphome.codegen as cg
from esphome.components import output

AUTO_LOAD = ["ledc"]

drv8243_ns = cg.esphome_ns.namespace("drv8243")

DRV8243Output = drv8243_ns.class_(
    "DRV8243Output",
    cg.Component,
    output.FloatOutput,
)
