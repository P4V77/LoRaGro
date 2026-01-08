#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include "sensor.hpp"

namespace loragro
{
    template <size_t N>
    class ZephyrSensorAdapter : public Sensor<N>
    {
    public:
        explicit ZephyrSensorAdapter(const struct device *dev)
            : dev_(dev) {}

        int init() override
        {
            return device_is_ready(dev_) ? 0 : -ENODEV;
        }

    protected:
        int fetch() const
        {
            return sensor_sample_fetch(dev_);
        }

        int get(enum sensor_channel chan,
                struct sensor_value *val) const
        {
            return sensor_channel_get(dev_, chan, val);
        }

        const struct device *dev_;
    };
}
