/*
 * Copyright (c) 2025 P4V77
 * Licensed under the Apache License, Version 2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "power_rail_3v3.hpp"
#include "sample_manager.hpp"
#include "sensors/env_sensor_adapter.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct device *const bme280_dev =
    DEVICE_DT_GET(DT_NODELABEL(bme280));

int main(void)
{
    printk("LoRaGro FiNo simulator start\n");

    loragro::PowerRail3V3 regulator;
    loragro::SampleManager sample_mgr;
    loragro::EnvSensorAdapter bme280(bme280_dev, 1000, 1001, 1002);

    sample_mgr.add_sensor(&bme280);

    LOG_INF("Starting sensor application");

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
