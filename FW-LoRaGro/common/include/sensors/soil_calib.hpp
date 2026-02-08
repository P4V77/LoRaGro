#pragma once

#include <cstdint>
#include <cstddef>

/*=========================================================
 * Soil Calibration Constants
 * ========================================================= */
namespace SoilSensorConstants
{
    constexpr uint16_t WET_SOIL_VOLTAGE = 1300; /* mV */
    constexpr uint16_t DRY_SOIL_VOLTAGE = 3000; /* mV */

    struct SoilLutPoint
    {
        int16_t mv;
        uint8_t moisture;
    };

    static constexpr SoilLutPoint SOIL_LUT[] =
        {
            {3000, 0},
            {2700, 10},
            {2400, 25},
            {2100, 45},
            {1800, 65},
            {1500, 85},
            {1200, 100},
    };

    static constexpr size_t SOIL_LUT_SIZE =
        sizeof(SOIL_LUT) / sizeof(SOIL_LUT[0]);
}
