#pragma once

#include "scheduler.hpp"

#include <array>
#include <cstdint>

namespace app {

class IMotor {
 public:
  virtual ~IMotor() = default;
  virtual bool move(std::int32_t steps) = 0;
  virtual bool done() const = 0;
  virtual void poll(std::uint32_t now) = 0;
};

class SimMotor final : public IMotor {
 public:
  explicit SimMotor(std::uint8_t id) : id_(id) {}

  bool move(std::int32_t steps) override;
  bool done() const override { return done_; }
  void poll(std::uint32_t now) override;

  bool busy() const { return busy_; }
  std::int32_t last_steps() const { return last_steps_; }

 private:
  std::uint8_t id_ = 0;
  bool busy_ = false;
  bool done_ = true;
  std::uint32_t complete_at_ = 0;
  std::int32_t last_steps_ = 0;
};

class MotorDone final : public Waiter {
 public:
  MotorDone(IMotor& motor, std::uint32_t timeout_ms) : Waiter(timeout_ms), motor_(motor) {}

  bool await_ready() const { return motor_.done(); }
  bool await_suspend(std::coroutine_handle<> handle);
  bool await_resume() const { return !timed_out(); }

 protected:
  bool ready(std::uint32_t now) const override;

 private:
  IMotor& motor_;
};

IMotor& GetMotor(std::uint8_t id);
void PollMotors(std::uint32_t now);

}  // namespace app
