#pragma once
#include <cstdint>
#include <stdlib.h>

#include "data_types.hpp"
#include "config_manager.hpp"

namespace loragro
{
    /* -------------------------------------------------
     * Lora RX Config Frame
     * -------------------------------------------------
     * [0] device_id (target)
     * [1] frame_type  (0=data, 1=config, 0xA5=ack, etc.)
     * [2] packet_counter
     * [3] command_count
     * [4] protocol_version
     * [5] command_id1
     * [6..9] config1
     * [10] command_idN
     * [11..14] configN
     * ------------------------------------------------- */

    class LoRaProtocolHandler
    {
    public:
        LoRaProtocolHandler(ConfigManager &cfg)
            : cfg_(cfg) {};

        DecodeResult decode(const uint8_t *buffer, const uint8_t buffer_len);

    private:
        const ConfigManager &cfg_;
        using HandlerFn = DecodeResult (LoRaProtocolHandler::*)(const uint8_t *, uint8_t);

        DecodeResult handle_set_device_id(const uint8_t *data, const uint8_t payload_ctr);
        DecodeResult handle_sampling_interval(const uint8_t *data, const uint8_t payload_ctr);
        DecodeResult handle_reboot(const uint8_t *data, const uint8_t payload_ctr);

        static constexpr HandlerFn dispatch_table[static_cast<uint8_t>(MessageOp::MAX_OP)] = {
            &LoRaProtocolHandler::handle_set_device_id,
            &LoRaProtocolHandler::handle_sampling_interval,
            &LoRaProtocolHandler::handle_reboot,
        };

        static constexpr size_t dispatch_table_size_ =
            sizeof(dispatch_table) / sizeof(dispatch_table[0]);
    };
}