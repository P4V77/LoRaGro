#pragma once
#include <cstdint>
#include <stdlib.h>

#include "data_types.hpp"
#include "config_manager.hpp"

namespace loragro
{
    /* LoRa RX Data Frame */
    /*
    [0]  protocol_version   (1B)
    [1]  command_id         (1B)
    [2]  payload_length N   (1B)
    [3..N] payload
    [last] optional CRC (optional if PHY already CRC protected)
    */
    class LoRaProtocolHandler
    {
    public:
        LoRaProtocolHandler(ConfigManager &cfg)
            : cfg_(cfg) {};

        DecodeResult decode(const uint8_t *buffer, const uint8_t buffer_len);

    private:
        const ConfigManager &cfg_;
        using HandlerFn = DecodeResult (LoRaProtocolHandler::*)(const uint8_t *, uint8_t);

        DecodeResult handle_set_device_id(const uint8_t *data, const uint8_t data_len);
        DecodeResult handle_sampling_interval(const uint8_t *data, const uint8_t data_len);
        DecodeResult handle_reboot(const uint8_t *data, const uint8_t data_len);

        static constexpr HandlerFn dispatch_table[static_cast<uint8_t>(MessageOp::MAX_OP)] = {
            &LoRaProtocolHandler::handle_set_device_id,
            &LoRaProtocolHandler::handle_sampling_interval,
            &LoRaProtocolHandler::handle_reboot,
        };

        static constexpr size_t dispatch_table_size_ =
            sizeof(dispatch_table) / sizeof(dispatch_table[0]);
    };
}