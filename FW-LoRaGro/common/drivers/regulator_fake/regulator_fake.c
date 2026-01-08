#define DT_DRV_COMPAT p4v_fake_regulator

#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>

LOG_MODULE_REGISTER(regulator_fake, LOG_LEVEL_DBG); // ← Changed to DBG

struct regulator_fake_data
{
    struct regulator_common_data common;
};

struct regulator_fake_config
{
    struct regulator_common_config common;
};

static int fake_enable(const struct device *dev)
{
    // LOG_DBG("Fake regulator ENABLE called");  // ← Added
    return 0;
}

static int fake_disable(const struct device *dev)
{
    // LOG_DBG("Fake regulator DISABLE called");  // ← Added
    return 0;
}

static unsigned int fake_count_voltages(const struct device *dev)
{
    return 1;
}

static int fake_list_voltage(const struct device *dev, unsigned int idx,
                             int32_t *volt_uv)
{
    if (idx != 0)
    {
        return -EINVAL;
    }
    *volt_uv = 3300000;
    return 0;
}

static int fake_set_voltage(const struct device *dev, int32_t min_uv, int32_t max_uv)
{
    LOG_DBG("set_voltage: %d - %d uV", min_uv, max_uv);
    return 0;
}

static int fake_get_voltage(const struct device *dev, int32_t *volt_uv)
{
    *volt_uv = 3300000;
    LOG_DBG("get_voltage: %d uV", *volt_uv);
    return 0;
}

static const struct regulator_driver_api fake_api = {
    .enable = fake_enable,
    .disable = fake_disable,
    .count_voltages = fake_count_voltages,
    .list_voltage = fake_list_voltage,
    .set_voltage = fake_set_voltage, // ← Add
    .get_voltage = fake_get_voltage, // ← Add
};

static int regulator_fake_init(const struct device *dev)
{
    LOG_INF("=== REGULATOR FAKE INIT START ==="); // ← Added
    LOG_INF("Device: %s", dev->name);             // ← Added

    regulator_common_data_init(dev);

    int ret = regulator_common_init(dev, false);

    LOG_INF("regulator_common_init returned: %d", ret); // ← Added
    LOG_INF("=== REGULATOR FAKE INIT END ===");         // ← Added

    return ret;
}

#define REGULATOR_FAKE_DEFINE(inst)                                \
    static struct regulator_fake_data data_##inst;                 \
                                                                   \
    static const struct regulator_fake_config config_##inst = {    \
        .common = REGULATOR_DT_INST_COMMON_CONFIG_INIT(inst),      \
    };                                                             \
                                                                   \
    DEVICE_DT_INST_DEFINE(inst,                                    \
                          regulator_fake_init,                     \
                          NULL,                                    \
                          &data_##inst,                            \
                          &config_##inst,                          \
                          POST_KERNEL,                             \
                          CONFIG_REGULATOR_P4V_FAKE_INIT_PRIORITY, \
                          &fake_api);

DT_INST_FOREACH_STATUS_OKAY(REGULATOR_FAKE_DEFINE)