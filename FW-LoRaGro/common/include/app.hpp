#pragma once
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "power_rail_3v3.hpp"
#include "sample_manager.hpp"
#include "sensors/env_sensor_adapter.hpp"
#include "sensors/light_sensor_adapter.hpp"
#include "sensors/co2_sensor_adapter.hpp"
#include "sensors/soil_3in1_adapter.hpp"
#include "sensors/soil_capacitive_adapter.hpp"
#include "sensors/battery_sense.hpp"
#include "lora/lora_interface.hpp"
#include "lora/lora_auth.hpp"
#include "lora/lora_frame_codec.hpp"
#include "lora/lora_protocol_handler.hpp"
#include "config_manager.hpp"
#include "power_management.hpp"

namespace loragro
{

    class App
    {
    public:
        App();

        int init();
        void run();

    private:
        int register_sensors();
        void run_cycle();

    private:
        /* ---- Core ---- */
        PowerRail3V3 regulator_;
        SampleManager sample_mgr_;

        ConfigManager &cfg_;
        DeviceConfig dev_cfg_{};

        /* ---- LoRa stack ---- */
        Interface lora_transceiver_;
        Auth auth_;
        FrameCodec tx_codec_;
        ProtocolHandler rx_handler_;
        PowerManagement pwr_mgr_;

        /* ---- Sensors (persistent instances) ---- */
        EnvSensorAdapter env_sensor_;
        LightSensorAdapter light_sensor_;
        CO2SensorAdapter co2_sensor_;
        SoilSensor3in1ModbusAdapter soil_3in1_sensor_;
        SoilCapacitiveSensor soil_analog_sensor_;
        BatterySenseAdapter battery_sense_;
    };

} // namespace loragro
