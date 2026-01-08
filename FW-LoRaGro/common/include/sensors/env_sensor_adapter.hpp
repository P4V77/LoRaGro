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
            : ZephyrSensorAdapter(dev),
              t_id_(tid), h_id_(hid), p_id_(pid) {};

        int sample() override
        {
            int ret = fetch();
            if (ret)
            {
                return ret;
            }

            sensor_value val;

            get(SENSOR_CHAN_AMBIENT_TEMP, &val);
            measurements_[0].sensor_id = t_id_;
            measurements_[0].sensor_type = SensorType::TEMPERATURE;
            measurements_[0].value = sensor_value_to_float(&val);
            measurements_[0].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            get(SENSOR_CHAN_HUMIDITY, &val);
            measurements_[1].sensor_id = h_id_;
            measurements_[1].sensor_type = SensorType::HUMIDITY;
            measurements_[1].value = sensor_value_to_float(&val);
            measurements_[1].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            get(SENSOR_CHAN_PRESS, &val);
            measurements_[2].sensor_id = p_id_;
            measurements_[2].sensor_type = SensorType::PRESSURE;
            measurements_[2].value = sensor_value_to_float(&val);
            measurements_[2].timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            return 0;
        }

        const char *getName() const override { return "Enviromental Sensor"; };

    protected:
        uint16_t t_id_, h_id_, p_id_;
    };
};