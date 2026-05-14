#pragma once

#include "app_config.hpp"

#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>

namespace app {

class Waiter {
 public:
  explicit Waiter(std::uint32_t timeout_ms);
  virtual ~Waiter() = default;

  bool should_resume(std::uint32_t now);
  bool timed_out() const { return timed_out_; }

 protected:
  virtual bool ready(std::uint32_t now) const = 0;

 private:
  std::uint32_t deadline_ = 0;
  std::uint32_t timeout_ms_ = 0;
  bool armed_ = false;
  bool timed_out_ = false;
};

class Scheduler {
 public:
  bool schedule(std::coroutine_handle<> handle);
  bool wait(std::coroutine_handle<> handle, Waiter& waiter);
  void poll();
  bool has_work() const;

 private:
  struct ReadySlot {
    std::coroutine_handle<> handle{};
    bool used = false;
  };

  struct WaitSlot {
    std::coroutine_handle<> handle{};
    Waiter* waiter = nullptr;
    bool used = false;
  };

  std::array<ReadySlot, kMaxActiveTasks> ready_{};
  std::array<WaitSlot, kMaxActiveTasks> waiting_{};
};

Scheduler& scheduler();

}  // namespace app
