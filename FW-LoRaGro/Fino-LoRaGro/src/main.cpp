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
#include "sensors/soil_3in1_adapter.hpp"
#include "sensors/soil_capacitive_adapter.hpp"
#include "sensors/battery_sense.hpp"
#include "lora/lora_interface.hpp"
#include "lora/lora_packetizer.hpp"
#include "lora/lora_protocol_handler.hpp"
#include "config_manager.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct device *const envi_dev =
    DEVICE_DT_GET(DT_ALIAS(envirmoental_sensor));

static const struct device *const light_dev =
    DEVICE_DT_GET(DT_ALIAS(light_sensor));

static const struct device *const co2_dev =
    DEVICE_DT_GET(DT_ALIAS(co2_sensor));

static const struct device *const soil_modbus_dev =
    DEVICE_DT_GET(DT_ALIAS(soil_sensor));

static const struct device *const adc_dev =
    DEVICE_DT_GET(DT_ALIAS(adc0));

static const struct device *const lora_dev =
    DEVICE_DT_GET(DT_ALIAS(lora0));

int main(void)
{
    auto &cfg = loragro::ConfigManager::instance();
    cfg.load();
    const auto &dev_cfg = cfg.get();

    LOG_INF("Device Combined ID (Device ID + ID of Gateway Assigned to this Device): %d", dev_cfg.combined_id);

    LOG_INF("Registering sensors");

    loragro::PowerRail3V3 regulator;
    loragro::SampleManager sample_mgr;

    loragro::EnvSensorAdapter env_sensor(envi_dev, loragro::SensorID::ENV_TEMP, loragro::SensorID::ENV_RH, loragro::SensorID::ENV_PRESS);
    loragro::LightSensorAdapter light_sensor(light_dev, loragro::SensorID::AMB_LIGHT);
    loragro::CO2SensorAdapter co2_sensor(co2_dev, loragro::SensorID::CO2_CONC, loragro::SensorID::CO2_TEMP, loragro::SensorID::CO2_RH);
    loragro::SoilSensor3in1ModbusAdapter soil_3in1_sensor(soil_modbus_dev, loragro::SensorID::SOIL_MOISTURE, loragro::SensorID::SOIL_TEMP, loragro::SensorID::SOIL_EC);
    loragro::SoilCapacitiveSensor soil_analog_sensor(adc_dev, loragro::SensorID::SOIL_ANALOG_MOISTURE);
    loragro::BatterySenseAdapter battery_sense(adc_dev, loragro::SensorID::BATTERY_VOLTAGE);

    loragro::LoRaInterface lora_transiever(lora_dev);
    loragro::LoRaPacketizer lora_packetizer(cfg);
    loragro::LoRaProtocolHandler lora_rx_handler(cfg);

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
    if (device_is_ready(soil_modbus_dev))
    {
        sample_mgr.add_sensor(&soil_3in1_sensor);
    }
    if (device_is_ready(adc_dev) && soil_analog_sensor.is_connected())
    {
        sample_mgr.add_sensor(&soil_analog_sensor);
    }
    if (device_is_ready(adc_dev) && battery_sense.is_connected())
    {
        sample_mgr.add_sensor(&battery_sense);
    }

    LOG_INF("Starting application");

    while (true)
    {
        // 1. Turn ON 3V3 rail.
        regulator.powerOn();
        // 2. Initialization and Configuration after powering ON
        sample_mgr.init_all();
        lora_transiever.init(dev_cfg);
        // 3. Sample all sensors
        sample_mgr.sample_all();
        // 4. Get batch of measurements over LoRa
        auto batch = sample_mgr.get_batch();
        // 5. Build and Send LoRa packets
        lora_packetizer.begin(batch);
        uint8_t packet[255]{0xFF}; // safe max PHY size
        size_t max_payload = lora_transiever.get_max_payload();

        while (lora_packetizer.has_packet_to_send())
        {
            int len = lora_packetizer.build_packet(packet, max_payload);
            if (len <= 0)
            {
                break;
            }
            uint8_t packet_nmbr = lora_packetizer.get_packet_number(packet, len);
            int ret = lora_transiever.send_confirmed(packet, len, packet_nmbr);
            if (ret < 0)
            {
                LOG_ERR("Packet %d failed: %d", packet_nmbr, ret);
            }
            else
            {
                LOG_INF("Packet %d ACKed", packet_nmbr);
            }
        }
        // 6. Recieve and Decode any Messages from Gateway Node
        /*
        Some kind of lora config based on snri or rssi would be nice but that shoudle be driven by gateway
        However maybe when transmision fails could be nice to somehow check if signal ok and so on
        */
        while (true)
        {
            int recieved_bytes = lora_transiever.receive(packet, max_payload);

            if (recieved_bytes <= 0)
            {
                LOG_INF("No RX in timeout after sensor data transmission");
                break;
            }

            /* Gateway is waiting for acknowledge from Node so its not problem to process incoming data imidiately */
            auto result = lora_rx_handler.decode(packet, recieved_bytes);

            if (result == loragro::DecodeResult::PROTOCOL_MISMATCH ||
                result == loragro::DecodeResult::UNKNOWN_COMMAND)
            {
                lora_transiever.send_ack(packet, max_payload);
            }
        }
        // 6. Turning OFF 3V3 rail (all sensors and components) for maximum power saving
        regulator.powerOff();
        // 7. Deep sleep CPU
        // power_sleep_until(sample_interval_ms);
        k_sleep(K_MINUTES(15));
    }
}