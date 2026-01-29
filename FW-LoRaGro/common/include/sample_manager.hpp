/**
 * Measurement subsystem
 *
 * Power model:
 *  - Sensor rail (power_rail_3v3) is power-gated
 *  - Sensors lose state between sampling cycles
 *  - All sensors MUST be re-initialized after power enable
 *
 * Lifecycle:
 *  enable_3v3()
 *   → init_all()
 *   → sample_all()
 *   → disable_3v3()
 *
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#include "sensors/sensor_base.hpp"
#include "data_types.hpp"

namespace loragro
{
    class SampleManager
    {
    public:
        static constexpr uint8_t MAX_SENSORS = 32;
        static constexpr uint8_t MAX_MEASUREMENT = 255;

        int add_sensor(SensorBase *sensor);
        int init_all();
        int sample_all();
        const BatchView get_batch();

        size_t batch_size() const { return batch_size_; }

    private:
        std::array<SensorBase *, MAX_SENSORS> sensors_;
        uint8_t sensor_count_ = 0;

        std::array<Measurement, MAX_MEASUREMENT> batch_;
        uint8_t batch_size_ = 0;
    };

};