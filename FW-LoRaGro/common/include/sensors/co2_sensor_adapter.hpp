#pragma once

#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"

namespace loragro
{
    class CO2SensorAdapter : public ZephyrSensorAdapter<3> // Changed from <1> to <3>
    {
    public:
        CO2SensorAdapter(const struct device *dev,
                         const uint16_t co2_id,
                         const uint16_t temp_id,
                         const uint16_t hum_id) : ZephyrSensorAdapter(dev)
        {
            // CO2 measurement
            measurements_[0].sensor_id = co2_id;
            measurements_[0].sensor_type = SensorType::CARBON_DIOXIDE;

            // Temperature measurement
            measurements_[1].sensor_id = temp_id;
            measurements_[1].sensor_type = SensorType::TEMPERATURE;

            // Humidity measurement
            measurements_[2].sensor_id = hum_id;
            measurements_[2].sensor_type = SensorType::HUMIDITY;
        };

        int sample() override
        {
            int ret = fetch();
            if (ret)
            {
                return ret;
            }

            sensor_value val;
            uint32_t timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());

            // Get CO2
            get(SENSOR_CHAN_CO2, &val);
            measurements_[0].value = val;
            measurements_[0].timestamp = timestamp;

            // Get temperature
            get(SENSOR_CHAN_AMBIENT_TEMP, &val);
            measurements_[1].value = val;
            measurements_[1].timestamp = timestamp;

            // Get humidity
            get(SENSOR_CHAN_HUMIDITY, &val);
            measurements_[2].value = val;
            measurements_[2].timestamp = timestamp;

            return 0;
        }

        const char *getName() const override { return "SCD41 Sensor"; }
    };
}