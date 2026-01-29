#pragma once
#include "cstdint"
#include "data_types.hpp"

namespace loragro
{
    class LoRaPacketizer
    {
    public:
        LoRaPacketizer() {};

        int begin(struct BatchView batch);
        int build_packet(uint8_t *packet, size_t packet_length);
        int get_packet_number(uint8_t *buffer, uint8_t len) const;
        int has_packet_to_send();

        void set_sensor_id(uint8_t device_id) { device_id_ = device_id; };

    private:
        struct BatchView batch_;
        uint8_t batch_count_offset_{0};
        uint8_t packet_ctr_{0};

        uint8_t device_id_{0x00};
    };
};