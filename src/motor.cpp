#include "motor.hpp"

#include "app_config.hpp"
#include "diagnostics.hpp"
#include "stm32f4xx_hal.h"

namespace app {

namespace {

std::array<SimMotor, 2> g_motors{SimMotor{1}, SimMotor{2}};

bool time_reached(std::uint32_t now, std::uint32_t deadline) {
  return static_cast<std::int32_t>(now - deadline) >= 0;
}

}  // namespace

bool SimMotor::move(std::int32_t steps) {
  if (busy_) {
    return false;
  }

  last_steps_ = steps;
  busy_ = true;
  done_ = false;
  complete_at_ = HAL_GetTick() + kSimMotorMoveMs;
  diag_inc(g_diag.motor_started);
  (void)id_;
  return true;
}

void SimMotor::poll(std::uint32_t now) {
  if (busy_ && time_reached(now, complete_at_)) {
    busy_ = false;
    done_ = true;
    diag_inc(g_diag.motor_completed);
  }
}

bool MotorDone::await_suspend(std::coroutine_handle<> handle) {
  return scheduler().wait(handle, *this);
}

bool MotorDone::ready(std::uint32_t) const {
  return motor_.done();
}

IMotor& GetMotor(std::uint8_t id) {
  if (id == 2U) {
    return g_motors[1];
  }
  return g_motors[0];
}

void PollMotors(std::uint32_t now) {
  for (auto& motor : g_motors) {
    motor.poll(now);
  }
}

}  // namespace app
