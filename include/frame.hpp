#pragma once

#include "app_config.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace app {

enum class Command : std::uint8_t {
  MoveStep = 0x01,
};

enum class Status : std::uint8_t {
  Ok = 0x00,
  BadCrc = 0x01,
  Unsupported = 0x02,
  Busy = 0x03,
  Timeout = 0x04,
  InvalidPayload = 0x05,
  QueueFull = 0x06,
};

struct Frame {
  std::uint8_t seq = 0;
  std::uint8_t cmd = 0;
  std::uint8_t payload_len = 0;
  std::array<std::uint8_t, kMaxPayloadBytes> payload{};
};

struct Ack {
  std::uint8_t seq = 0;
  std::uint8_t cmd = 0;
  Status status = Status::Ok;
};

std::uint8_t crc8_xor(const std::uint8_t* data, std::size_t size);
std::size_t encode_frame(const Frame& frame, std::uint8_t* out, std::size_t out_size);

class FrameParser {
 public:
  bool push(std::uint8_t byte);
  bool pop(Frame& frame);

 private:
  enum class State : std::uint8_t { Start, Length, Seq, Cmd, Payload, Crc };

  void reset();

  State state_ = State::Start;
  Frame pending_{};
  Frame ready_{};
  bool has_ready_ = false;
  std::uint8_t crc_acc_ = 0;
  std::uint8_t payload_index_ = 0;
};

class AckWriter {
 public:
  bool push(std::uint8_t seq, std::uint8_t cmd, Status status);
  bool pop(Ack& ack);

 private:
  std::array<Ack, kAckQueueDepth> queue_{};
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t count_ = 0;
};

}  // namespace app
