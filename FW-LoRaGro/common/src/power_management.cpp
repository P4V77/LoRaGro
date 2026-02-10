#include "power_management.hpp"

LOG_MODULE_REGISTER(power_manager, LOG_LEVEL_DBG);

namespace loragro
{
    int PowerManagement::handle_sleep()
    {
        if (battery_sense_id_ < 0)
        {
            LOG_WRN("Battery sensor not available, using normal sleep");
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_min));
            return 0;
        }

        const auto meas = sample_mgr_.sample_one(battery_sense_id_);

        if (meas.value.val1 <= 0)
        {
            LOG_WRN("Battery measurement invalid, fallback to normal sleep");
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_min));
            return 0;
        }

        if (meas.value.val1 < dev_cfg_.battery_cutoff_mv)
        {
            while (true)
            {
                k_sleep(K_HOURS(dev_cfg_.critically_low_battery_timeout_hours));
                auto battery_meas = sample_mgr_.sample_one(battery_sense_id_);
                if (battery_meas.value.val1 >= dev_cfg_.battery_cutoff_mv)
                {
                    LOG_INF("Battery recovered: %d mV", battery_meas.value.val1);
                    break;
                }
                else
                {
                    LOG_WRN("Battery low: %d mV", battery_meas.value.val1);
                    LOG_WRN("Deep Sleep for: %d hours", dev_cfg_.critically_low_battery_timeout_hours);
                }
            }
        }
        else if (meas.value.val1 < dev_cfg_.battery_critical_mv)
        {
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_min_low_battery));
        }
        else
        {
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_min));
        }
        return 0;
    }
}