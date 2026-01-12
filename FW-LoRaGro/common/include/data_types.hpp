#pragma once
#include <cstdint>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

namespace loragro
{
    // Sensor type identifiers
    enum class SensorType : uint8_t
    {
        TEMPERATURE = 1,
        HUMIDITY = 2,
        PRESSURE = 3,
        AMBIENT_LIGHT = 4,
        CARBON_DIOXIDE = 5,
        // add more as needed
    };

    struct Measurement
    {
        uint16_t sensor_id; // unique id per physical channel
        SensorType sensor_type;
        sensor_value value; // raw value or scaled
        uint32_t timestamp; // seconds uptime
    };

}