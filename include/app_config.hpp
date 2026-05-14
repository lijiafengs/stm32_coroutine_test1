/**
* @brief Defines application-level compile-time configuration values.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace app
{

    inline constexpr std::size_t kMaxActiveTasks = 4;
    inline constexpr std::size_t kMaxPayloadBytes = 16;
    inline constexpr std::size_t kAckQueueDepth = 8;
    inline constexpr std::uint32_t kMotorTimeoutMs = 1000;
    inline constexpr std::uint32_t kSimMotorMoveMs = 30;
}
