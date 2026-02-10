#pragma once
#include "zephyr_sensor_adapter.hpp"
#include "time_manager.hpp"
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/adc/adc_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

#define BATTERY_NODE DT_PATH(zephyr_user)
#define BATTERY_ADC_CHANNEL DT_IO_CHANNELS_INPUT_BY_IDX(BATTERY_NODE, 1)
#define BATTERY_ADC_RESOLUTION DT_PROP(BATTERY_NODE, resolution)

namespace loragro
{
    class BatterySenseAdapter : public ZephyrSensorAdapter<1>
    {
    public:
        BatterySenseAdapter(const struct device *adc_dev, uint16_t battery_mv_id)
            : ZephyrSensorAdapter(adc_dev)
        {
            measurements_[0].sensor_id = battery_mv_id;

            channel_cfg_.gain = ADC_GAIN_1_6;
            channel_cfg_.reference = ADC_REF_INTERNAL;
            channel_cfg_.acquisition_time = ADC_ACQ_TIME_DEFAULT;
            channel_cfg_.channel_id = BATTERY_ADC_CHANNEL;
            channel_cfg_.differential = 0;

            sequence_.buffer = &raw_value_;
            sequence_.buffer_size = sizeof(raw_value_);
            sequence_.resolution = BATTERY_ADC_RESOLUTION;
            sequence_.oversampling = 0;
            sequence_.channels = BIT(BATTERY_ADC_CHANNEL);

            /* Voltage divider from DT */
            r1_ohm_ = DT_PROP_BY_IDX(BATTERY_NODE, voltage_divider, 0);
            r2_ohm_ = DT_PROP_BY_IDX(BATTERY_NODE, voltage_divider, 1);
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
            // Initialize with realistic battery voltage
            adc_emul_const_raw_value_set(dev_, BATTERY_ADC_CHANNEL, 2100);
#endif

            return 0;
        }

        int sample() override
        {
            int ret;

#ifdef CONFIG_ADC_EMUL
            /* Simulate battery ramp in RAW domain */
            static int32_t fake_raw = 1400;

            fake_raw += 25;

            if (fake_raw > 2300) // upper limit
                fake_raw = 1300; // wrap around

            adc_emul_const_raw_value_set(dev_, BATTERY_ADC_CHANNEL, fake_raw);
#endif

            /* Trigger ADC sampling */
            ret = adc_read(dev_, &sequence_);
            if (ret < 0)
            {
                return ret;
            }

            int32_t raw = raw_value_;

            /* ---- Convert RAW -> mV at ADC pin ---- */

            constexpr int32_t max_adc = (1 << BATTERY_ADC_RESOLUTION) - 1; // e.g. 4095 (12bit)
            constexpr int32_t vref_mv = 600;                               // nRF internal reference
            constexpr int32_t gain_multiplier = 6;                         // gain = 1/6

            int32_t adc_mv = (raw * vref_mv * gain_multiplier) / max_adc;

            /* ---- Compensate resistor divider (from DT ideally) ---- */

            int32_t battery_mv = adc_mv * (r1_ohm_ + r2_ohm_) / r2_ohm_;

            /* ---- Store measurement ---- */

            measurements_[0].sensor_id = measurements_[0].sensor_id;
            measurements_[0].timestamp =
                TimeManager::best_effort_unix_s(k_uptime_seconds());

            /* Store as volts + microvolts */
            measurements_[0].value.val1 = battery_mv; // integer volts
            return 0;
        }

        int is_connected() override
        {
            int ret = adc_read(dev_, &sequence_);
            if (ret)
            {
                return ret;
            }

            ret = adc_raw_to_millivolts(
                channel_cfg_.reference,
                channel_cfg_.gain,
                BATTERY_ADC_RESOLUTION,
                &raw_value_);
            if (ret)
            {
                return ret;
            }

            // Check ADC voltage (before divider scaling)
            if (raw_value_ < 100)
            {
                return -EIO;
            }

            // Also check actual battery voltage range
            int32_t battery_mv = scale_by_divider(raw_value_);
            if (battery_mv < 1100 || battery_mv > 2200)
            {
                return -EIO;
            }

            return 0;
        }

        const char *getName() const override
        {
            return "Battery Voltage (ADC)";
        }

    private:
        int32_t scale_by_divider(int32_t mv) const
        {
            /* Vbat = Vadc * (R1 + R2) / R2 */
            return (mv * (r1_ohm_ + r2_ohm_)) / r2_ohm_;
        }

    private:
        struct adc_channel_cfg channel_cfg_;
        struct adc_sequence sequence_;
        int32_t raw_value_;
        uint32_t r1_ohm_;
        uint32_t r2_ohm_;
    };
} // namespace loragro