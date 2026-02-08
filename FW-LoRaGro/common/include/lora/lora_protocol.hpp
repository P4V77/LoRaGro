#pragma once

#include <cstdint>
#include <cstddef>

namespace loragro
{
    /* =========================================================
     * Protocol constants
     * ========================================================= */

    static constexpr uint8_t PROTOCOL_VERSION = 1;

    /* 32-bit authentication tag appended to every frame */
    static constexpr size_t AUTH_TAG_SIZE = 4;

    /* =========================================================
     * Supported Operations (Gateway -> Node)
     * ========================================================= */
    enum class MessageOp : uint8_t
    {
        SET_ID = 1,
        SET_SAMPLING_INTERVAL,
        REBOOT,
        MAX_OP
    };

    /* =========================================================
     * Decode Result Status
     * ========================================================= */
    enum class DecodeResult : uint8_t
    {
        OK = 0,
        PROTOCOL_MISMATCH,
        INVALID_LENGTH,
        UNKNOWN_COMMAND,
        EXECUTED_REBOOT,
        DIFFERENT_ID,
        AUTH_FAILED
    };

    /* =========================================================
     * Frame Types
     * ========================================================= */
    enum class FrameType : uint8_t
    {
        DATA = 1,
        CONFIG,
        ERROR,
        ACK = 0xA5
    };

    /* =========================================================
     * Common Frame Layout (shared by ALL frames)
     * ---------------------------------------------------------
     * [0..1] target_id   (16-bit combined ID)
     * [2..3] source_id   (16-bit combined ID)
     * [4]    frame_type
     * [5]    packet_counter
     * ---------------------------------------------------------
     * Frame-specific fields start at HEADER_SIZE
     * ========================================================= */
    struct FrameLayout
    {
        static constexpr size_t TARGET_ID_MSB = 0;
        static constexpr size_t TARGET_ID_LSB = 1;

        static constexpr size_t SOURCE_ID_MSB = 2;
        static constexpr size_t SOURCE_ID_LSB = 3;

        static constexpr size_t FRAME_TYPE = 4;
        static constexpr size_t PACKET_CTR = 5;

        static constexpr size_t HEADER_SIZE = 6;

        /* Auth tag location:
           last 4 bytes of frame */
    };

    /* =========================================================
     * DATA Frame (Node -> Gateway)
     * ---------------------------------------------------------
     * After common header:
     *
     * [6]      measurement_count
     * [7..10]  timestamp (uint32_t)
     * [11..]   measurements
     * [...-4]  auth_tag (uint32_t)
     * ========================================================= */

    /* =========================================================
     * CONFIG Frame (Gateway -> Node)
     * ---------------------------------------------------------
     * After common header:
     *
     * [6]      command_count
     * [7]      protocol_version
     * [8..]    commands
     * [...-4]  auth_tag
     * ========================================================= */

    /* =========================================================
     * ERROR Frame
     * ---------------------------------------------------------
     * After common header:
     *
     * [6] protocol_version
     * [7] error_id
     * [...-4] auth_tag
     * ========================================================= */

    /* =========================================================
     * ACK Frame
     * ---------------------------------------------------------
     * After common header:
     *
     * (no extra fields)
     * [...-4] auth_tag
     * ========================================================= */

    /* =========================================================
     * ID Helpers
     * ---------------------------------------------------------
     * 16-bit ID format:
     * [ GATEWAY 5 bits | NODE 11 bits ]
     *
     * Max gateways = 32
     * Max nodes    = 2048
     * ========================================================= */

    static inline uint16_t make_combined_id(uint8_t gateway, uint16_t node)
    {
        return ((static_cast<uint16_t>(gateway & 0x1F) << 11) |
                (node & 0x7FF));
    }

    static inline uint8_t extract_gateway(uint16_t combined)
    {
        return (combined >> 11) & 0x1F;
    }

    static inline uint16_t extract_node(uint16_t combined)
    {
        return combined & 0x7FF;
    }

    /* =========================================================
     * Safe helpers for reading/writing 16-bit IDs
     * ========================================================= */

    static inline void write_u16_be(uint8_t *buf, size_t offset, uint16_t value)
    {
        buf[offset] = (value >> 8) & 0xFF;
        buf[offset + 1] = value & 0xFF;
    }

    static inline uint16_t read_u16_be(const uint8_t *buf, size_t offset)
    {
        return (static_cast<uint16_t>(buf[offset]) << 8) |
               buf[offset + 1];
    }

} // namespace loragro
