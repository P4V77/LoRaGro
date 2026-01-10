#pragma once
#include <cstddef>
#include "sensor_base.hpp"

namespace loragro
{

    template <size_t N>
    class Sensor : public SensorBase
    {
    public:
        const Measurement *measurements() const override
        {
            return measurements_;
        }

        size_t count() const override
        {
            return N;
        }

    protected:
        Measurement measurements_[N];
    };

} // namespace loragro
