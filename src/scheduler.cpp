#include "scheduler.hpp"

#include "diagnostics.hpp"
#include "stm32f4xx_hal.h"

namespace app {

namespace {

bool time_reached(std::uint32_t now, std::uint32_t deadline) {
  return static_cast<std::int32_t>(now - deadline) >= 0;
}

}  // namespace

Waiter::Waiter(std::uint32_t timeout_ms) : timeout_ms_(timeout_ms) {}

bool Waiter::should_resume(std::uint32_t now) {
  if (!armed_) {
    armed_ = true;
    deadline_ = now + timeout_ms_;
  }

  if (ready(now)) {
    return true;
  }

  if ((timeout_ms_ != 0U) && time_reached(now, deadline_)) {
    timed_out_ = true;
    diag_inc(g_diag.scheduler_timeouts);
    return true;
  }

  return false;
}

bool Scheduler::schedule(std::coroutine_handle<> handle) {
  if (!handle) {
    return false;
  }

  for (auto& slot : ready_) {
    if (!slot.used) {
      slot.handle = handle;
      slot.used = true;
      return true;
    }
  }
  return false;
}

bool Scheduler::wait(std::coroutine_handle<> handle, Waiter& waiter) {
  if (!handle) {
    return false;
  }

  for (auto& slot : waiting_) {
    if (!slot.used) {
      slot.handle = handle;
      slot.waiter = &waiter;
      slot.used = true;
      return true;
    }
  }
  return false;
}

void Scheduler::poll() {
  const std::uint32_t now = HAL_GetTick();

  for (auto& slot : waiting_) {
    if (slot.used && (slot.waiter != nullptr) && slot.waiter->should_resume(now)) {
      auto handle = slot.handle;
      slot = {};
      (void)schedule(handle);
    }
  }

  for (auto& slot : ready_) {
    if (!slot.used) {
      continue;
    }

    auto handle = slot.handle;
    slot = {};

    if (!handle || handle.done()) {
      if (handle) {
        handle.destroy();
      }
      continue;
    }

    diag_inc(g_diag.scheduler_resumes);
    handle.resume();

    if (handle.done()) {
      handle.destroy();
    }
  }
}

bool Scheduler::has_work() const {
  for (const auto& slot : ready_) {
    if (slot.used) {
      return true;
    }
  }
  for (const auto& slot : waiting_) {
    if (slot.used) {
      return true;
    }
  }
  return false;
}

Scheduler& scheduler() {
  static Scheduler instance;
  return instance;
}

}  // namespace app
