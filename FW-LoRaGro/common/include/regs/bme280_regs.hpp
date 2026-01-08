#pragma once
#include <cstdint>

namespace loragro::bme280
{

    /* Chip identification */
    static constexpr uint8_t CHIP_ID = 0x60;

    /* Control & config registers */
    static constexpr uint8_t REG_CHIP_ID = 0xD0;
    static constexpr uint8_t REG_RESET = 0xE0;
    static constexpr uint8_t REG_CTRL_HUM = 0xF2;
    static constexpr uint8_t REG_STATUS = 0xF3;
    static constexpr uint8_t REG_CTRL_MEAS = 0xF4;
    static constexpr uint8_t REG_CONFIG = 0xF5;

    /* Measurement data registers (burst-readable block) */
    static constexpr uint8_t REG_PRESS_MSB = 0xF7;
    static constexpr uint8_t REG_PRESS_LSB = 0xF8;
    static constexpr uint8_t REG_PRESS_XLSB = 0xF9;

    static constexpr uint8_t REG_TEMP_MSB = 0xFA;
    static constexpr uint8_t REG_TEMP_LSB = 0xFB;
    static constexpr uint8_t REG_TEMP_XLSB = 0xFC;

    static constexpr uint8_t REG_HUM_MSB = 0xFD;
    static constexpr uint8_t REG_HUM_LSB = 0xFE;

    /* Burst read helpers */
    static constexpr uint8_t REG_DATA_START = REG_PRESS_MSB;
    static constexpr uint8_t DATA_LEN = 8; // F7..FE

    /* Calib helpers */
    static constexpr uint8_t REG_CALIB_00 = 0x88;
    static constexpr uint8_t REG_CALIB_26 = 0xE1;

} // namespace loragro::bme280
