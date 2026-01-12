#pragma once

#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"

namespace loragro
{
    class EnvSensorAdapter : public ZephyrSensorAdapter<3>
    {
    public:
        EnvSensorAdapter(const struct device *dev,
                         uint16_t tid, uint16_t hid, uint16_t pid)
            : ZephyrSensorAdapter(dev)
        {
            measurements_[0].sensor_id = tid;
            measurements_[0].sensor_type = SensorType::TEMPERATURE;

            measurements_[1].sensor_id = hid;
            measurements_[1].sensor_type = SensorType::HUMIDITY;

            measurements_[2].sensor_id = pid;
            measurements_[2].sensor_type = SensorType::PRESSURE;
        };

        int sample() override
        {
            int ret = fetch();
            if (ret)
            {
                return ret;
            }

            sensor_value val;

            get(SENSOR_CHAN_AMBIENT_TEMP, &val);
            measurements_[0].value = val;
            measurements_[0].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            get(SENSOR_CHAN_HUMIDITY, &val);
            measurements_[1].value = val;
            measurements_[1].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            get(SENSOR_CHAN_PRESS, &val);
            measurements_[2].value = val;
            measurements_[2].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            return 0;
        }

        const char *getName() const override { return "Enviromental Sensor"; };

    protected:
    };
};