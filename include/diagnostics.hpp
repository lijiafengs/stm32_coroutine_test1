#pragma once

#include <cstdint>

namespace app {

struct Diagnostics {
  volatile std::uint32_t loop_count;
  volatile std::uint32_t frames_ok;
  volatile std::uint32_t frames_bad_crc;
  volatile std::uint32_t command_started;
  volatile std::uint32_t command_busy;
  volatile std::uint32_t command_unknown;
  volatile std::uint32_t motor_started;
  volatile std::uint32_t motor_completed;
  volatile std::uint32_t ack_ok;
  volatile std::uint32_t ack_error;
  volatile std::uint32_t scheduler_resumes;
  volatile std::uint32_t scheduler_timeouts;
  volatile std::uint8_t last_status;
};

extern Diagnostics g_diag;

inline void diag_inc(volatile std::uint32_t& value) {
  value = value + 1U;
}

}  // namespace app
