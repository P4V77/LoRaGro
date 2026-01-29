#pragma once

#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

namespace loragro
{
    namespace dt
    {
#define SOIL_NODE DT_ALIAS(soil_sensor)

        static_assert(DT_NODE_EXISTS(SOIL_NODE),
                      "DT alias 'soil_sensor' not defined");

        struct Soil3in1
        {
            static constexpr uint8_t slave_addr =
                DT_PROP(SOIL_NODE, slave_address);
        };

#undef SOIL_NODE
    }

    // Custom sensor channel for soil EC (must match your driver)
    enum p4v_soil_channel
    {
        SENSOR_CHAN_SOIL_EC = SENSOR_CHAN_PRIV_START,
    };

    class SoilSensor3in1ModbusAdapter : public ZephyrSensorAdapter<3>
    {
    public:
        SoilSensor3in1ModbusAdapter(const struct device *dev,
                                    const uint16_t moisture_id,
                                    const uint16_t temperature_id,
                                    const uint16_t conductivity_id)
            : ZephyrSensorAdapter(dev)
        {
            // Initialize measurement IDs
            measurements_[0].sensor_id = moisture_id;

            measurements_[1].sensor_id = temperature_id;

            measurements_[2].sensor_id = conductivity_id;
        }

        int sample() override
        {
            // Use Zephyr sensor API instead of direct Modbus
            int ret = this->fetch();
            if (ret != 0)
            {
                return ret;
            }

            uint32_t timestamp = TimeManager::best_effort_unix_s(k_uptime_seconds());
            sensor_value val;

            // Read humidity
            ret = get(SENSOR_CHAN_HUMIDITY, &val);
            if (!ret)
            {
                measurements_[0].value = val;
                measurements_[0].timestamp = timestamp;
            }
            else
            {
                return ret;
            }
            // Read temperature
            ret = get(SENSOR_CHAN_AMBIENT_TEMP, &val);
            if (!ret)
            {
                measurements_[1].value = val;
                measurements_[1].timestamp = timestamp;
            }
            else
            {
                return ret;
            }

            // Read conductivity (custom channel)
            ret = get(static_cast<sensor_channel>(SENSOR_CHAN_SOIL_EC),
                      &val);
            if (!ret)
            {
                measurements_[2].value = val;
                measurements_[2].timestamp = timestamp;
            }
            else
            {
                return ret;
            }

            return 0;
        }

        const char *
        getName() const override
        {
            return "DFRobot Soil Sensor 3in1 Soil Moisture, Temp and EC (Modbus)";
        }
    };
}