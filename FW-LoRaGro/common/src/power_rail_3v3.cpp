#include "power_rail_3v3.hpp"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(power_rail_3v3, LOG_LEVEL_INF);

#define REGULATOR_3V3_NODE DT_NODELABEL(power_rail_3v3)


namespace loragro
{
    int PowerRail3V3::powerOn()
    {
        if (powered_)
        {
            return 0;
        }

        static const device *regulator = DEVICE_DT_GET(REGULATOR_3V3_NODE);

        if (!device_is_ready(regulator))
        {
            LOG_ERR("3V3 regulator not ready");
            return -ENODEV;
        }

        int ret = regulator_enable(regulator);
        if (ret)
        {
            LOG_ERR("Failed to enable 3V3  rail (%d)", ret);
            return ret;
        }

        k_sleep(K_MSEC(10));

        PowerRail3V3::powered_ = true;
        LOG_DBG("Sensor 3V3 ON");

        return 0;
    }
    int PowerRail3V3::powerOff()
    {
        if (!powered_)
        {
            LOG_DBG("Device already OFF");
            return 0;
        }

        static const device *regulator =
            DEVICE_DT_GET(REGULATOR_3V3_NODE);

        if (!device_is_ready(regulator))
        {
            LOG_ERR("3V3  rail regulator not ready");
            return -ENODEV;
        }

        int ret = regulator_disable(regulator);
        if (ret)
        {
            LOG_ERR("Failed to disable 3V3 rail regulator (%d)", ret);
            return ret;
        }

        PowerRail3V3::powered_ = false;
        LOG_DBG("Power rail 3V3 OFF");

        return 0;
    }
};