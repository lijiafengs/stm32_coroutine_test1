#include "task.hpp"

#include <cstddef>
#include <cstdint>

namespace app {

namespace {

constexpr std::size_t kTaskFrameBytes = 512;

struct TaskFrameSlot {
  alignas(std::max_align_t) std::uint8_t bytes[kTaskFrameBytes];
  bool used = false;
};

TaskFrameSlot g_task_slots[kMaxActiveTasks];

}  // namespace

void* Task::promise_type::operator new(std::size_t size) noexcept {
  if (size > kTaskFrameBytes) {
    return nullptr;
  }

  for (auto& slot : g_task_slots) {
    if (!slot.used) {
      slot.used = true;
      return slot.bytes;
    }
  }
  return nullptr;
}

void Task::promise_type::operator delete(void* ptr, std::size_t) noexcept {
  if (ptr == nullptr) {
    return;
  }

  for (auto& slot : g_task_slots) {
    if (ptr == static_cast<void*>(slot.bytes)) {
      slot.used = false;
      return;
    }
  }
}

Task Task::promise_type::get_return_object_on_allocation_failure() noexcept {
  return {};
}

Task Task::promise_type::get_return_object() noexcept {
  return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
}

void Task::promise_type::unhandled_exception() noexcept {
  while (true) {
  }
}

Task::Task(Task&& other) noexcept : handle_(other.handle_) {
  other.handle_ = {};
}

Task& Task::operator=(Task&& other) noexcept {
  if (this != &other) {
    if (handle_) {
      handle_.destroy();
    }
    handle_ = other.handle_;
    other.handle_ = {};
  }
  return *this;
}

Task::~Task() {
  if (handle_) {
    handle_.destroy();
  }
}

bool Task::start(Scheduler& scheduler) {
  if (!handle_) {
    return false;
  }

  if (!scheduler.schedule(handle_)) {
    return false;
  }

  handle_ = {};
  return true;
}

}  // namespace app
