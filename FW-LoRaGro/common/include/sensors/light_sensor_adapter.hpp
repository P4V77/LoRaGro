#pragma once

#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"

namespace loragro
{
    class LightSensorAdapter : public ZephyrSensorAdapter<1>
    {
    public:
        LightSensorAdapter(const struct device *dev, uint16_t lid)
            : ZephyrSensorAdapter(dev)
        {
            measurements_[0].sensor_id = lid;
            measurements_[0].sensor_type = SensorType::AMBIENT_LIGHT;
        };

        int sample() override
        {
            int ret = fetch();
            if (ret)
            {
                return ret;
            }

            sensor_value val;

            get(SENSOR_CHAN_ALL, &val);

            measurements_[0].value = val;
            measurements_[0].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            return 0;
        }

        const char *getName() const override { return "Light Intensity Sensor"; };
    };
}