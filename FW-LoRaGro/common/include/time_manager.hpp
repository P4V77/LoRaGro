#pragma once

#include <cstdint>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

namespace loragro
{

    /**
     * TimeManager
     *
     * - Uses Zephyr monotonic uptime for sampling timestamps
     * - Supports optional synchronization to real (Unix) time
     * - Allows retroactive conversion of old samples
     *
     * All timestamps inside the system are stored as MONOTONIC seconds.
     * Conversion to real time is done only when needed.
     */
    class TimeManager
    {
    public:
        /** Monotonic uptime in seconds (always valid) */
        static uint32_t monotonic_s()
        {
            return k_uptime_seconds();
        }

        /**
         * Synchronize real (Unix epoch) time.
         *
         * unix_s: seconds since Unix epoch (UTC)
         */
        static void sync_unix_time_s(uint64_t unix_s)
        {
            uint32_t now_s = monotonic_s();
            epoch_base_s_ = unix_s - now_s;
            synced_ = true;
        }

        /** Returns true if real time is known */
        static bool is_synced()
        {
            return synced_;
        }

        /**
         * Convert monotonic timestamp to Unix time (seconds).
         *
         * Returns monotonic_s if not synchronized yet.
         */
        static uint32_t best_effort_unix_s(uint32_t monotonic_s)
        {
            if (!synced_)
            {
                return (uint32_t)monotonic_s; // uptime in seconds
            }
            return (uint32_t)(epoch_base_s_ + monotonic_s);
        }

    private:
        /** Unix epoch offset: unix_s = epoch_base_s_ + monotonic_s */
        inline static uint64_t epoch_base_s_ = 0;

        /** Whether real time has been synchronized */
        inline static bool synced_ = false;
    };

} // namespace loragro
