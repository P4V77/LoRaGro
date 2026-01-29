#pragma once

#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/adc/adc_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

/* Get ADC channel and resolution from devicetree */
#define SOIL_CAP_NODE DT_PATH(zephyr_user)
#define SOIL_ADC_CHANNEL DT_IO_CHANNELS_INPUT_BY_IDX(SOIL_CAP_NODE, 0)
#define SOIL_ADC_RESOLUTION DT_PROP(SOIL_CAP_NODE, resolution)

namespace loragro
{
    class SoilCapacitiveSensor : public ZephyrSensorAdapter<1>
    {
    public:
        SoilCapacitiveSensor(const struct device *dev,
                             uint16_t moisture_id)
            : ZephyrSensorAdapter(dev)
        {
            measurements_[0].sensor_id = moisture_id;

            channel_cfg_.gain = ADC_GAIN_1_6;
            channel_cfg_.reference = ADC_REF_INTERNAL;
            channel_cfg_.acquisition_time = ADC_ACQ_TIME_DEFAULT;
            channel_cfg_.channel_id = SOIL_ADC_CHANNEL;
            channel_cfg_.differential = 0;

            sequence_.buffer = &raw_value_;
            sequence_.buffer_size = sizeof(raw_value_);
            sequence_.resolution = SOIL_ADC_RESOLUTION;
            sequence_.oversampling = 0;
            sequence_.channels = BIT(SOIL_ADC_CHANNEL);
        }

        int init() override
        {
            if (!device_is_ready(dev_))
            {
                return -ENODEV;
            }

            int ret = adc_channel_setup(dev_, &channel_cfg_);
            if (ret != 0)
            {
                return ret;
            }

#ifdef CONFIG_ADC_EMUL
            // Initialize with realistic sensor voltage
            adc_emul_const_raw_value_set(dev_, SOIL_ADC_CHANNEL, 2400);
#endif

            return 0;
        }

        int sample() override
        {
#ifdef CONFIG_ADC_EMUL
            static int fake_soil_raw = 1365;
            fake_soil_raw += 50;
            if (fake_soil_raw > 4095)
            {
                fake_soil_raw = 1365;
            }
            adc_emul_const_raw_value_set(dev_, SOIL_ADC_CHANNEL, fake_soil_raw);
#endif

            int ret = adc_read(dev_, &sequence_);
            if (ret)
                return ret;

            ret = adc_raw_to_millivolts(
                channel_cfg_.reference,
                channel_cfg_.gain,
                SOIL_ADC_RESOLUTION,
                &raw_value_);
            if (ret)
                return ret;

            int mv = raw_value_;
            uint8_t moisture = soil_mv_to_percent(mv);

            measurements_[0].value.val1 = moisture;
            measurements_[0].timestamp =
                TimeManager::best_effort_unix_s(k_uptime_seconds());

            return 0;
        }

        uint8_t soil_mv_to_percent(int32_t mv)
        {
            // Clamp high (too dry)
            if (mv >= SoilSensorConstants::SOIL_LUT[0].mv)
            {
                return SoilSensorConstants::SOIL_LUT[0].moisture;
            }

            // Clamp low (too wet)
            if (mv <= SoilSensorConstants::SOIL_LUT[SoilSensorConstants::SOIL_LUT_SIZE - 1].mv)
            {
                return SoilSensorConstants::SOIL_LUT[SoilSensorConstants::SOIL_LUT_SIZE - 1].moisture;
            }

            for (size_t i = 0; i < SoilSensorConstants::SOIL_LUT_SIZE - 1; i++)
            {
                const auto &p0 = SoilSensorConstants::SOIL_LUT[i];
                const auto &p1 = SoilSensorConstants::SOIL_LUT[i + 1];

                if (mv <= p0.mv && mv >= p1.mv)
                {
                    // Linear interpolation
                    return p0.moisture +
                           (p1.moisture - p0.moisture) *
                               (p0.mv - mv) /
                               (p0.mv - p1.mv);
                }
            }

            return 0; // should never hit
        }

        int is_connected() override
        {

            int ret = adc_read(dev_, &sequence_);
            if (ret)
                return ret;

            ret = adc_raw_to_millivolts(
                channel_cfg_.reference,
                channel_cfg_.gain,
                SOIL_ADC_RESOLUTION,
                &raw_value_);
            if (ret)
                return ret;

            int mv = raw_value_;
            adc_raw_to_millivolts(channel_cfg_.reference, channel_cfg_.gain, SOIL_ADC_RESOLUTION, &mv);

            if (mv < 100)
            {
                return -EIO;
            }
            return 0;
        }

        const char *getName() const override
        {
            return "Capacitive Soil Moisture (ADC)";
        }

    private:
        struct adc_channel_cfg channel_cfg_;
        struct adc_sequence sequence_;
        int32_t raw_value_;
    };

} // namespace loragro