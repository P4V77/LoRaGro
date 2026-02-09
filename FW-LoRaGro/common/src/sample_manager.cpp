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
        return sensor_count_;
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
        batch_size_ = 0; /* Need to reset every sample */

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

            /* Serializing data into batch */
            for (size_t j = 0; j < n; ++j)
            {
                if (batch_size_ >= MAX_MEASUREMENT)
                {
                    return -ENOMEM;
                }

                batch_[batch_size_++] = m[j];

                /* Scaling down */
                // int16_t int_part = static_cast<int16_t>(m[j].value.val1 / 1000);
                int16_t int_part = static_cast<int16_t>(m[j].value.val1);
                int16_t frac_3dp = static_cast<int16_t>(m[j].value.val2 / 1000);

                LOG_INF("Type(Manager) %u Measurement[%u]: ID=%u value=%d.%03d ts=%u",
                        i, j,
                        m[j].sensor_id,
                        int_part,
                        frac_3dp,
                        m[j].timestamp);
            }
        }
        return 0;
    }

    loragro::Measurement SampleManager::sample_one(uint8_t sensor_id)
    {
        if (sensor_id >= sensor_count_ || !sensors_[sensor_id])
        {
            LOG_ERR("Invalid sensor_id %u", sensor_id);
            return Measurement{}; // return default / invalid measurement
        }

        int ret = sensors_[sensor_id]->sample();
        if (ret)
        {
            LOG_ERR("Sampling sensor %u failed: %d", sensor_id, ret);
            return Measurement{}; // invalid
        }

        const Measurement *m = sensors_[sensor_id]->measurements();
        size_t n = sensors_[sensor_id]->count();
        if (n == 0)
        {
            LOG_ERR("Sensor %u returned no measurement", sensor_id);
            return Measurement{};
        }

        return m[0]; // battery sensor usually returns single measurement
    }

    const BatchView SampleManager::get_batch()
    {
        return BatchView{
            .data = batch_.data(),
            .count = batch_size_};
    }
}