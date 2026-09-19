import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import DavisVantage, CONF_DAVIS_VANTAGE_ID

DEPENDENCIES = ['davis_vantage']

CONF_WIND_DIRECTION = 'wind_direction'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_DAVIS_VANTAGE_ID): cv.use_id(DavisVantage),
    cv.Optional(CONF_WIND_DIRECTION): text_sensor.text_sensor_schema(
        icon="mdi:compass-outline"
    ),
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_DAVIS_VANTAGE_ID])
    if CONF_WIND_DIRECTION in config:
        sens = await text_sensor.new_text_sensor(config[CONF_WIND_DIRECTION])
        cg.add(hub.set_wind_dir_sensor(sens))
