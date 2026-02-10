#pragma once

#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"

#include "sample_manager.hpp"
#include "config_manager.hpp"
#include "data_types.hpp"

namespace loragro
{
    class PowerManagement
    {
    public:
        PowerManagement(SampleManager &sample_mgr,
                        const uint8_t battery_sense_id,
                        const DeviceConfig &dev_cfg)
            : sample_mgr_(sample_mgr),
              battery_sense_id_(battery_sense_id),
              dev_cfg_(dev_cfg) {};

        int handle_sleep();

    private:
        SampleManager &sample_mgr_;
        const uint8_t &battery_sense_id_;
        const DeviceConfig &dev_cfg_;
    };
}
