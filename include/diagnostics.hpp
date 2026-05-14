/**
* @brief Declares firmware diagnostics counters used for debug watch windows.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include <cstdint>

namespace app
{

    struct Diagnostics
    {
        volatile std::uint32_t m_loopCount;
        volatile std::uint32_t m_framesOk;
        volatile std::uint32_t m_framesBadCrc;
        volatile std::uint32_t m_commandStarted;
        volatile std::uint32_t m_commandBusy;
        volatile std::uint32_t m_commandUnknown;
        volatile std::uint32_t m_motorStarted;
        volatile std::uint32_t m_motorCompleted;
        volatile std::uint32_t m_ackOk;
        volatile std::uint32_t m_ackError;
        volatile std::uint32_t m_schedulerResumes;
        volatile std::uint32_t m_schedulerTimeouts;
        volatile std::uint8_t m_lastStatus;
    };

    extern Diagnostics g_diagnostics;

    /**
    * @brief Increments a volatile diagnostic counter by one.
    * @param[in,out] value Counter reference to increment.
    * @return None.
    */
    inline void DiagInc(volatile std::uint32_t& value)
    {
        value = value + 1U;
    }
}
