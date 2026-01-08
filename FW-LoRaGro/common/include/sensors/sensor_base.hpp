#pragma once
#include <cstddef>
#include "data_types.hpp"

namespace loragro
{

    class SensorBase
    {
    public:
        virtual ~SensorBase() = default;

        virtual int init() = 0;
        virtual int sample() = 0;

        virtual const Measurement *measurements() const = 0;
        virtual size_t count() const = 0;
        virtual const char *getName() const = 0;
    };

} // namespace loragro
