#pragma once

#include "frame.hpp"
#include "task.hpp"

namespace app {

class Commands {
 public:
  explicit Commands(AckWriter& ack_writer) : ack_writer_(ack_writer) {}

  Task moveStep(Frame frame);

 private:
  AckWriter& ack_writer_;
};

class CommandDispatcher {
 public:
  CommandDispatcher(Commands& commands, AckWriter& ack_writer, Scheduler& scheduler)
      : commands_(commands), ack_writer_(ack_writer), scheduler_(scheduler) {}

  void dispatch(const Frame& frame);

 private:
  Commands& commands_;
  AckWriter& ack_writer_;
  Scheduler& scheduler_;
};

}  // namespace app
