#define DT_DRV_COMPAT p4v_sensor_bh1750_fake

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bh1750_fake, LOG_LEVEL_INF);

struct bh1750_fake_config
{
    const struct device *vdd;
};

struct bh1750_fake_data
{
    struct sensor_value light_intensity;
};

static int bh1750_fake_sample_fetch(const struct device *dev,
                                    enum sensor_channel chan)
{
    struct bh1750_fake_data *data = dev->data;

    if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_LIGHT)
    {
        return -ENOTSUP;
    }

    static int fake_lux = 8000;

    fake_lux += 500;
    if (fake_lux > 30000)
    {
        fake_lux = 2000;
    }

    data->light_intensity.val1 = fake_lux;
    data->light_intensity.val2 = 0;

    return 0;
}

static int bh1750_fake_channel_get(const struct device *dev,
                                   enum sensor_channel chan,
                                   struct sensor_value *val)
{
    struct bh1750_fake_data *data = dev->data;

    switch (chan)
    {

    case SENSOR_CHAN_LIGHT:
        /* Return the last sampled ambient light value (lux) */
        val->val1 = data->light_intensity.val1;
        val->val2 = data->light_intensity.val2;
        return 0;

    case SENSOR_CHAN_ALL:
        /*
         * BH1750 exposes only one meaningful channel (ambient light),
         * so SENSOR_CHAN_ALL is treated the same as SENSOR_CHAN_LIGHT.
         */
        val->val1 = data->light_intensity.val1;
        val->val2 = data->light_intensity.val2;
        return 0;

    default:
        /*
         * Any other channel (temperature, pressure, etc.)
         * is not supported by the BH1750 sensor.
         */
        LOG_ERR("BH1750 fake: unsupported channel request (%d)", chan);
        return -ENOTSUP;
    }
}

static const struct sensor_driver_api bh1750_fake_api =
    {
        .sample_fetch = bh1750_fake_sample_fetch,
        .channel_get = bh1750_fake_channel_get,
};

static int bh1750_fake_init(const struct device *dev)
{
    const struct bh1750_fake_config *cfg = dev->config;

    if (!device_is_ready(cfg->vdd))
    {
        LOG_ERR("VDD regulator not ready");
        return -ENODEV;
    }

    LOG_INF("Fake BH1750 light sensor initialized");
    return 0;
}

#define SENSOR_BH1750_FAKE_DEFINE(inst)                                \
    static struct bh1750_fake_data data_##inst;                        \
    static const struct bh1750_fake_config config_##inst = {           \
        .vdd = DEVICE_DT_GET(DT_INST_PHANDLE(inst, vdd_supply))};      \
    DEVICE_DT_INST_DEFINE(inst,                                        \
                          bh1750_fake_init,                            \
                          NULL,                                        \
                          &data_##inst,                                \
                          &config_##inst,                              \
                          POST_KERNEL,                                 \
                          CONFIG_SENSOR_P4V_BH1750_FAKE_INIT_PRIORITY, \
                          &bh1750_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_BH1750_FAKE_DEFINE);