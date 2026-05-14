#include "commands.hpp"
#include "diagnostics.hpp"
#include "frame.hpp"
#include "motor.hpp"
#include "scheduler.hpp"
#include "stm32f4xx_hal.h"

namespace {

app::FrameParser g_parser;
app::AckWriter g_ack_writer;
app::Commands g_commands{g_ack_writer};
app::CommandDispatcher g_dispatcher{g_commands, g_ack_writer, app::scheduler()};

void push_demo_frame() {
  app::Frame frame{};
  frame.seq = 1;
  frame.cmd = static_cast<std::uint8_t>(app::Command::MoveStep);
  frame.payload_len = 5;
  frame.payload[0] = 1;

  const std::int32_t steps = 100;
  frame.payload[1] = static_cast<std::uint8_t>(steps & 0xFF);
  frame.payload[2] = static_cast<std::uint8_t>((steps >> 8) & 0xFF);
  frame.payload[3] = static_cast<std::uint8_t>((steps >> 16) & 0xFF);
  frame.payload[4] = static_cast<std::uint8_t>((steps >> 24) & 0xFF);

  std::uint8_t bytes[32]{};
  const std::size_t size = app::encode_frame(frame, bytes, sizeof(bytes));
  for (std::size_t i = 0; i < size; ++i) {
    (void)g_parser.push(bytes[i]);
  }
}

void drain_frames() {
  app::Frame frame{};
  while (g_parser.pop(frame)) {
    app::diag_inc(app::g_diag.frames_ok);
    g_dispatcher.dispatch(frame);
  }
}

void drain_acks() {
  app::Ack ack{};
  while (g_ack_writer.pop(ack)) {
    app::g_diag.last_status = static_cast<std::uint8_t>(ack.status);
    if (ack.status == app::Status::Ok) {
      app::diag_inc(app::g_diag.ack_ok);
    } else {
      app::diag_inc(app::g_diag.ack_error);
    }
  }
}

}  // namespace

extern "C" int main(void) {
  HAL_Init();
  push_demo_frame();

  while (true) {
    app::diag_inc(app::g_diag.loop_count);
    drain_frames();
    app::PollMotors(HAL_GetTick());
    app::scheduler().poll();
    drain_acks();
  }
}
