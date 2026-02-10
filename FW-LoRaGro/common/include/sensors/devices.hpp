static const struct device *const envi_dev =
    DEVICE_DT_GET(DT_ALIAS(envirmoental_sensor));

static const struct device *const light_dev =
    DEVICE_DT_GET(DT_ALIAS(light_sensor));

static const struct device *const co2_dev =
    DEVICE_DT_GET(DT_ALIAS(co2_sensor));

static const struct device *const soil_modbus_dev =
    DEVICE_DT_GET(DT_ALIAS(soil_sensor));

static const struct device *const adc_dev =
    DEVICE_DT_GET(DT_ALIAS(adc0));

static const struct device *const lora_dev =
    DEVICE_DT_GET(DT_ALIAS(lora0));