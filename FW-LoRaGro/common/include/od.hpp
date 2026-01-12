#pragma once
#include <cstdint>

namespace loragro::SensorID
{
    /* 0x0??? Enviromental Sensor ID*/
    constexpr uint16_t ENV_TEPM = 0000;
    constexpr uint16_t ENV_RH = 0001;
    constexpr uint16_t ENV_PRESS = 0002;
    /* 1??? Ambient Light Sensor ID*/
    constexpr uint16_t AMB_LIGHT = 1000;
    /* 2??? C02 Sensor ID*/
    constexpr uint16_t CO2_CONCENTRATION = 2000;
    constexpr uint16_t CO2_TEMP = 2001;
    constexpr uint16_t CO2_RH = 2003;
};