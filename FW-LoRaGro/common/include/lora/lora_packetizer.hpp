#pragma once
#include "cstdint"
#include "data_types.hpp"
#include "config_manager.hpp"

namespace loragro
{

    /* -------------------------------------------------
     * Lora Data Frame
     * -------------------------------------------------
     * [0] device_id
     * [1] reserved/error frame
     * [2] packet_counter
     * [3] measurement_count
     * [4..7] timestamp (shared for whole batch)
     * [8..payload_max] measurement data
     * ------------------------------------------------- */

    /* -------------------------------------------------
     * LoRa Error Frame
     * -------------------------------------------------
     * [0] device_id
     * [1] error frame flag
     * [2] protocol_id
     * [3] error_id
     * ------------------------------------------------- */

    /*
     * Simple ACK format:
     * [0] = 0xA5 (ACK marker)
     * [1] = Device ID
     * [2] = Packet counter
     */

    class LoRaPacketizer
    {
    public:
        LoRaPacketizer(ConfigManager cfg)
            : cfg_(cfg) {};

        int begin(struct BatchView batch);
        int build_packet(uint8_t *packet, size_t packet_length);
        int build_packet(uint8_t *packet, size_t packet_length, DecodeResult error_id);
        int get_packet_number(uint8_t *buffer, uint8_t len) const;
        int get_device_id() const;
        int has_packet_to_send();

        void set_sensor_id(uint8_t device_id) { device_id_ = device_id; };

    private:
        struct BatchView batch_;
        const ConfigManager &cfg_;

        uint8_t batch_count_offset_{0};
        uint8_t packet_ctr_{0};

        uint8_t device_id_{0x00};
    };
};