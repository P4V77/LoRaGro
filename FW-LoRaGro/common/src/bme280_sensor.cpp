/**
 * BME280 sensor driver
 *
 * Lifecycle:
 *  - Sensor VDD is power-gated
 *  - init() MUST be called after every power-on
 *  - init() reads calibration and configures the device
 *
 * Note:
 *  BME280 sleep mode is not relied upon, as the sensor
 *  is fully powered down between measurements.
 */

#include <errno.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sensors/bme280_sensor.hpp"
#include "regs/bme280_regs.hpp"

LOG_MODULE_REGISTER(bme280_sensor, LOG_LEVEL_INF);

namespace loragro
{

    /* ===================== INIT ===================== */

    int BME280Sensor::init()
    {
        if (!device_is_ready(i2c_.bus))
            return -ENODEV;

        uint8_t chip_id = 0;
        int ret = i2c_reg_read_byte_dt(&i2c_, bme280::REG_CHIP_ID, &chip_id);
        if (ret)
        {
            return ret;
        }

        if (chip_id != bme280::CHIP_ID)
        {
            return -EINVAL;
        }

        ret = read_calibration();
        if (ret)
        {
            return ret;
        }

        /* Weather monitoring configuration (forced mode, x1 oversampling) */
        i2c_reg_write_byte_dt(&i2c_, bme280::REG_CTRL_HUM, 0x01); // humidity x1
        i2c_reg_write_byte_dt(&i2c_, bme280::REG_CONFIG, 0x00);   // filter off

        return 0;
    }

    /* ===================== CALIBRATION ===================== */

    int BME280Sensor::read_calibration()
    {
        uint8_t buf1[26];
        uint8_t buf2[7];

        int ret = i2c_burst_read_dt(&i2c_, 0x88, buf1, sizeof(buf1));
        if (ret)
            return ret;

        ret = i2c_burst_read_dt(&i2c_, 0xE1, buf2, sizeof(buf2));
        if (ret)
            return ret;

        calib_.dig_T1 = (uint16_t)(buf1[1] << 8 | buf1[0]);
        calib_.dig_T2 = (int16_t)(buf1[3] << 8 | buf1[2]);
        calib_.dig_T3 = (int16_t)(buf1[5] << 8 | buf1[4]);

        calib_.dig_P1 = (uint16_t)(buf1[7] << 8 | buf1[6]);
        calib_.dig_P2 = (int16_t)(buf1[9] << 8 | buf1[8]);
        calib_.dig_P3 = (int16_t)(buf1[11] << 8 | buf1[10]);
        calib_.dig_P4 = (int16_t)(buf1[13] << 8 | buf1[12]);
        calib_.dig_P5 = (int16_t)(buf1[15] << 8 | buf1[14]);
        calib_.dig_P6 = (int16_t)(buf1[17] << 8 | buf1[16]);
        calib_.dig_P7 = (int16_t)(buf1[19] << 8 | buf1[18]);
        calib_.dig_P8 = (int16_t)(buf1[21] << 8 | buf1[20]);
        calib_.dig_P9 = (int16_t)(buf1[23] << 8 | buf1[22]);

        calib_.dig_H1 = buf1[25];
        calib_.dig_H2 = (int16_t)(buf2[1] << 8 | buf2[0]);
        calib_.dig_H3 = buf2[2];
        calib_.dig_H4 = (int16_t)((buf2[3] << 4) | (buf2[4] & 0x0F));
        calib_.dig_H5 = (int16_t)((buf2[5] << 4) | (buf2[4] >> 4));
        calib_.dig_H6 = (int8_t)buf2[6];

        return 0;
    }

    /* ===================== COMPENSATION ===================== */

    float BME280Sensor::compensate_temperature(int32_t adc_T)
    {
        float var1 =
            (((float)adc_T) / 16384.0f - ((float)calib_.dig_T1) / 1024.0f) *
            ((float)calib_.dig_T2);

        float var2 =
            ((((float)adc_T) / 131072.0f - ((float)calib_.dig_T1) / 8192.0f) *
             (((float)adc_T) / 131072.0f - ((float)calib_.dig_T1) / 8192.0f)) *
            ((float)calib_.dig_T3);

        t_fine_ = (int32_t)(var1 + var2);
        return (var1 + var2) / 5120.0f;
    }

    float BME280Sensor::compensate_pressure(int32_t adc_P)
    {
        float var1 = ((float)t_fine_ / 2.0f) - 64000.0f;
        float var2 = var1 * var1 * ((float)calib_.dig_P6) / 32768.0f;
        var2 += var1 * ((float)calib_.dig_P5) * 2.0f;
        var2 = (var2 / 4.0f) + (((float)calib_.dig_P4) * 65536.0f);

        var1 = (((float)calib_.dig_P3) * var1 * var1 / 524288.0f +
                ((float)calib_.dig_P2) * var1) /
               524288.0f;
        var1 = (1.0f + var1 / 32768.0f) * ((float)calib_.dig_P1);

        if (var1 == 0.0f)
            return 0.0f;

        float p = 1048576.0f - (float)adc_P;
        p = (p - (var2 / 4096.0f)) * 6250.0f / var1;

        var1 = ((float)calib_.dig_P9) * p * p / 2147483648.0f;
        var2 = p * ((float)calib_.dig_P8) / 32768.0f;

        return p + (var1 + var2 + ((float)calib_.dig_P7)) / 16.0f;
    }

    float BME280Sensor::compensate_humidity(int32_t adc_H)
    {
        float h = ((float)t_fine_) - 76800.0f;

        h = (adc_H -
             (((float)calib_.dig_H4) * 64.0f +
              ((float)calib_.dig_H5) / 16384.0f * h)) *
            (((float)calib_.dig_H2) / 65536.0f *
             (1.0f + ((float)calib_.dig_H6) / 67108864.0f * h *
                         (1.0f + ((float)calib_.dig_H3) / 67108864.0f * h)));

        h *= (1.0f - ((float)calib_.dig_H1) * h / 524288.0f);

        if (h > 100.0f)
            h = 100.0f;
        else if (h < 0.0f)
            h = 0.0f;

        return h;
    }

    /* ===================== SAMPLE ===================== */

    int BME280Sensor::sample()
    {
        /* Trigger forced measurement */
        LOG_DBG("Starting forced measurement on %s", this->getName());
        i2c_reg_write_byte_dt(&i2c_, bme280::REG_CTRL_MEAS, 0x25);

        uint8_t buf[8];
        int ret = i2c_burst_read_dt(&i2c_, bme280::REG_DATA_START, buf, sizeof(buf));
        if (ret)
            return ret;

        int32_t adc_p = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
        int32_t adc_t = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
        int32_t adc_h = (buf[6] << 8) | buf[7];

        float temperature = compensate_temperature(adc_t); // °C
        float pressure = compensate_pressure(adc_p);       // Pa
        float humidity = compensate_humidity(adc_h);       // %

        measurements_[0] = {temp_id_, SensorType::TEMPERATURE, temperature, 0};
        measurements_[1] = {hum_id_, SensorType::HUMIDITY, humidity, 0};
        measurements_[2] = {pres_id_, SensorType::PRESSURE, pressure, 0};

        return 0;
    }

} // namespace loragro
