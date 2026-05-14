#pragma once

#include "scheduler.hpp"

#include <coroutine>
#include <cstddef>
#include <cstdint>

namespace app {

class Task {
 public:
  struct promise_type {
    static void* operator new(std::size_t size) noexcept;
    static void operator delete(void* ptr, std::size_t size) noexcept;
    static Task get_return_object_on_allocation_failure() noexcept;

    Task get_return_object() noexcept;
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() noexcept;
  };

  Task() = default;
  explicit Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  Task(Task&& other) noexcept;
  Task& operator=(Task&& other) noexcept;
  ~Task();

  bool valid() const { return static_cast<bool>(handle_); }
  bool start(Scheduler& scheduler);

 private:
  std::coroutine_handle<promise_type> handle_{};
};

}  // namespace app
