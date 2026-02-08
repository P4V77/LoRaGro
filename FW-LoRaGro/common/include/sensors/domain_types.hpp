#pragma once

#include <cstdint>

#include <zephyr/drivers/sensor.h>

namespace loragro
{
    /* =========================================================
     * Sensor Type (logical grouping only)
     * ========================================================= */
    enum class SensorType : uint8_t
    {
        ENVIRONMENTAL = 0,
        AMBIENT_LIGHT = 1,
        CARBON_DIOXIDE = 2,
        SOIL = 3,
        BATTERY = 4,
    };

    /* =========================================================
     * Compact Sensor ID (1 byte)
     *
     *  7 6 5 4 | 3 2 1 0
     * +---------+---------+
     * | CLASS   | TYPE    |
     * +---------+---------+
     *
     * CLASS  (upper nibble)  : 0–15
     * TYPE   (lower nibble)  : 0–15
     * ========================================================= */
    namespace SensorID
    {
        /* Masks */
        constexpr uint8_t CLASS_MASK = 0xF0;
        constexpr uint8_t TYPE_MASK = 0x0F;

        /* Classes (upper nibble) */
        constexpr uint8_t ENV = 0x00;
        constexpr uint8_t AMB = 0x10;
        constexpr uint8_t CO2 = 0x20;
        constexpr uint8_t SOIL = 0x30;
        constexpr uint8_t BATTERY = 0x40;

        /* Environmental types */
        constexpr uint8_t ENV_TEMP = ENV | 0x00;
        constexpr uint8_t ENV_RH = ENV | 0x01;
        constexpr uint8_t ENV_PRESS = ENV | 0x02;

        /* Ambient light */
        constexpr uint8_t AMB_LIGHT = AMB | 0x00;

        /* CO2 */
        constexpr uint8_t CO2_TEMP = CO2 | 0x00;
        constexpr uint8_t CO2_RH = CO2 | 0x01;
        constexpr uint8_t CO2_CONC = CO2 | 0x02;

        /* Soil */
        constexpr uint8_t SOIL_TEMP = SOIL | 0x00;
        constexpr uint8_t SOIL_MOISTURE = SOIL | 0x01;
        constexpr uint8_t SOIL_EC = SOIL | 0x02;
        constexpr uint8_t SOIL_ANALOG_MOISTURE = SOIL | 0x03;

        /* Battery */
        constexpr uint8_t BATTERY_VOLTAGE = BATTERY | 0x00;

        /* Helpers */
        constexpr uint8_t sensor_class(uint8_t id)
        {
            return (id & CLASS_MASK) >> 4;
        }

        constexpr uint8_t value_type(uint8_t id)
        {
            return id & TYPE_MASK;
        }
    }

    /* =========================================================
     * Measurement container
     * ========================================================= */
    struct Measurement
    {
        uint8_t sensor_id;  /* Compact 1-byte ID */
        sensor_value value; /* Zephyr native value */
        uint32_t timestamp; /* Seconds since boot */
    };

    /* =========================================================
     * Batch view
     * ========================================================= */
    struct BatchView
    {
        const Measurement *data;
        size_t count;
    };
};