import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ['cc1101']
MULTI_CONF = True

davis_vantage_ns = cg.esphome_ns.namespace('davis_vantage')
DavisVantage = davis_vantage_ns.class_('DavisVantage', cg.Component)

CONF_DAVIS_VANTAGE_ID = 'davis_vantage_id'
CONF_CC1101_ID = 'cc1101_id'
CONF_UNIT_ID = 'unit_id'
CONF_REGION = 'region'
CONF_METRIC = 'metric'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DavisVantage),
    cv.Required(CONF_CC1101_ID): cv.use_id(cg.Component),
    cv.Optional(CONF_UNIT_ID, default=0): cv.int_range(min=0, max=7),
    cv.Optional(CONF_REGION, default='NA'): cv.string,
    cv.Optional(CONF_METRIC, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cc1101_var = await cg.get_variable(config[CONF_CC1101_ID])
    cg.add(var.set_cc1101(cc1101_var))
    cg.add(var.set_unit_id(config[CONF_UNIT_ID]))
    cg.add(var.set_region(config[CONF_REGION]))
    cg.add(var.set_metric(config[CONF_METRIC]))
