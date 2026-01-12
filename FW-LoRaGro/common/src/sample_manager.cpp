#include "sample_manager.hpp"
#include <algorithm>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sample_manager, LOG_LEVEL_DBG);

namespace loragro
{

    int SampleManager::add_sensor(SensorBase *sensor)
    {
        if (sensor == nullptr)
        {
            return -EINVAL;
        }
        if (sensor_count_ >= MAX_SENSORS)
        {
            return -ENOMEM;
        }

        sensors_[sensor_count_++] = sensor;
        return 0;
    }

    int SampleManager::init_all()
    {
        for (size_t i = 0; i < sensor_count_; ++i)
        {
            if (!sensors_[i])
            {
                return -EINVAL;
            }

            int ret = sensors_[i]->init();
            if (ret)
            {
                return ret;
            }
        }
        return 0;
    }

    int SampleManager::sample_all()
    {
        for (size_t i = 0; i < sensor_count_; ++i)
        {
            if (!sensors_[i])
                return -EINVAL;

            int ret = sensors_[i]->sample();
            if (ret)
            {
                return ret;
            }

            const Measurement *m = sensors_[i]->measurements();
            size_t n = sensors_[i]->count();

            for (size_t j = 0; j < n; ++j)
            {

                int int_part = m[j].value.val1;
                int frac_2dp = m[j].value.val2 / 10000; // 10⁶ → 10² | Keeping 2 decimal for shorter more sensible LOG

                LOG_INF("Sensor %u measurement[%u]: id=%u type=%u value=%d.%02d ts=%u",
                        i, j,
                        m[j].sensor_id,
                        static_cast<uint32_t>(m[j].sensor_type),
                        int_part,
                        frac_2dp,
                        m[j].timestamp);
            }
        }
        return 0;
    }
}