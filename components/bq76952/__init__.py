DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["button", "component_common", "sensor", "switch", "text_sensor"]

from ._schema import CONFIG_SCHEMA
from ._codegen import to_code

__all__ = ["CONFIG_SCHEMA", "to_code"]
