#include "commands.hpp"

#include "app_config.hpp"
#include "diagnostics.hpp"
#include "motor.hpp"

namespace app {

namespace {

std::int32_t read_i32_le(const std::uint8_t* data) {
  return static_cast<std::int32_t>(
      (static_cast<std::uint32_t>(data[0])) |
      (static_cast<std::uint32_t>(data[1]) << 8U) |
      (static_cast<std::uint32_t>(data[2]) << 16U) |
      (static_cast<std::uint32_t>(data[3]) << 24U));
}

}  // namespace

Task Commands::moveStep(Frame frame) {
  diag_inc(g_diag.command_started);

  if (frame.payload_len < 5U) {
    (void)ack_writer_.push(frame.seq, frame.cmd, Status::InvalidPayload);
    co_return;
  }

  const std::uint8_t motor_id = frame.payload[0];
  const std::int32_t steps = read_i32_le(&frame.payload[1]);
  IMotor& motor = GetMotor(motor_id);

  if (!motor.move(steps)) {
    (void)ack_writer_.push(frame.seq, frame.cmd, Status::Busy);
    co_return;
  }

  const bool completed = co_await MotorDone{motor, kMotorTimeoutMs};
  (void)ack_writer_.push(frame.seq, frame.cmd, completed ? Status::Ok : Status::Timeout);
}

void CommandDispatcher::dispatch(const Frame& frame) {
  switch (static_cast<Command>(frame.cmd)) {
    case Command::MoveStep: {
      Task task = commands_.moveStep(frame);
      if (!task.start(scheduler_)) {
        diag_inc(g_diag.command_busy);
        (void)ack_writer_.push(frame.seq, frame.cmd, Status::Busy);
      }
      break;
    }

    default:
      diag_inc(g_diag.command_unknown);
      (void)ack_writer_.push(frame.seq, frame.cmd, Status::Unsupported);
      break;
  }
}

}  // namespace app
