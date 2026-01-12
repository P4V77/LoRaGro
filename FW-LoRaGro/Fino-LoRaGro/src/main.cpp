/*
 * Copyright (c) 2025 P4V77
 * Licensed under the Apache License, Version 2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "power_rail_3v3.hpp"
#include "sample_manager.hpp"
#include "sensors/env_sensor_adapter.hpp"
#include "sensors/light_sensor_adapter.hpp"
#include "sensors/co2_sensor_adapter.hpp"
#include "od.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct device *const envi_dev =
    DEVICE_DT_GET(DT_ALIAS(envirmoental_sensor));

static const struct device *const light_dev =
    DEVICE_DT_GET(DT_ALIAS(light_sensor));

static const struct device *const co2_dev =
    DEVICE_DT_GET(DT_ALIAS(co2_sensor));

int main(void)
{
    LOG_INF("Registering sensors");

    loragro::PowerRail3V3 regulator;
    loragro::SampleManager sample_mgr;

    loragro::EnvSensorAdapter env_sensor(envi_dev, loragro::SensorID::ENV_TEPM, loragro::SensorID::ENV_RH, loragro::SensorID::ENV_PRESS);
    loragro::LightSensorAdapter light_sensor(light_dev, loragro::SensorID::AMB_LIGHT);
    loragro::CO2SensorAdapter co2_sensor(co2_dev, loragro::SensorID::CO2_CONCENTRATION, loragro::SensorID::CO2_TEMP, loragro::SensorID::CO2_RH);

    if (device_is_ready(envi_dev))
    {
        sample_mgr.add_sensor(&env_sensor);
    }
    if (device_is_ready(light_dev))
    {
        sample_mgr.add_sensor(&light_sensor);
    }
    if (device_is_ready(co2_dev))
    {
        sample_mgr.add_sensor(&co2_sensor);
    }

    LOG_INF("Starting application");

    while (true)
    {
        // 1. Turn ON 3V3 rail.
        regulator.powerOn();
        // 2. Initialization and Configuration of all sensors after powering ON
        sample_mgr.init_all();
        // 3. Sample all sensors
        sample_mgr.sample_all();
        // 4. Transmit batch of measurements over LoRa
        // auto batch = sample_mgr.get_batch();
        // lora_transmit(batch);
        // 5. Turning OFF 3V3 rail (all sensors and components) for maximum power saving
        regulator.powerOff();
        // 6. Deep sleep CPU
        // power_sleep_until(sample_interval_ms);
        k_sleep(K_MINUTES(15));
    }
}

// Simulated LoRa transmit function
// void lora_transmit(const std::vector<Measurement> &batch) {
//     printk("=== LoRa TX ===\n");
//     for (auto &m : batch) {
//         printk("Sensor %u type %u value %d timestamp %u\n",
//                m.sensor_id, m.sensor_type, m.value, m.timestamp);
//     }
//     printk("=== End TX ===\n\n");
// }
