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

    loragro::Measurement SampleManager::sample_one(uint8_t logical_id)
    {
        for (size_t s = 0; s < sensor_count_; ++s)
        {
            SensorBase *sensor = sensors_[s];

            const Measurement *m = sensor->measurements();
            size_t count = sensor->count();

            for (size_t i = 0; i < count; ++i)
            {
                if (m[i].sensor_id == logical_id)
                {
                    int ret = sensor->sample();
                    if (ret)
                    {
                        LOG_ERR("Sampling failed for sensor %s", sensor->getName());
                        return Measurement{};
                    }

                    // return updated measurement
                    return sensor->measurements()[i];
                }
            }
        }

        LOG_ERR("Logical sensor ID %u not found", logical_id);
        return Measurement{};
    }

    const BatchView SampleManager::get_batch()
    {
        return BatchView{
            .data = batch_.data(),
            .count = batch_size_};
    }
}