#include "frame.hpp"

#include "diagnostics.hpp"

namespace app {

namespace {

constexpr std::uint8_t kSof = 0xA5;

}  // namespace

std::uint8_t crc8_xor(const std::uint8_t* data, std::size_t size) {
  std::uint8_t crc = 0;
  for (std::size_t i = 0; i < size; ++i) {
    crc ^= data[i];
  }
  return crc;
}

std::size_t encode_frame(const Frame& frame, std::uint8_t* out, std::size_t out_size) {
  const std::size_t total = static_cast<std::size_t>(frame.payload_len) + 5U;
  if ((out == nullptr) || (out_size < total) || (frame.payload_len > kMaxPayloadBytes)) {
    return 0;
  }

  out[0] = kSof;
  out[1] = frame.payload_len;
  out[2] = frame.seq;
  out[3] = frame.cmd;
  for (std::uint8_t i = 0; i < frame.payload_len; ++i) {
    out[4U + i] = frame.payload[i];
  }
  out[4U + frame.payload_len] = crc8_xor(&out[1], static_cast<std::size_t>(frame.payload_len) + 3U);
  return total;
}

bool FrameParser::push(std::uint8_t byte) {
  switch (state_) {
    case State::Start:
      if (byte == kSof) {
        pending_ = {};
        crc_acc_ = 0;
        payload_index_ = 0;
        state_ = State::Length;
      }
      return true;

    case State::Length:
      if (byte > kMaxPayloadBytes) {
        reset();
        return false;
      }
      pending_.payload_len = byte;
      crc_acc_ ^= byte;
      state_ = State::Seq;
      return true;

    case State::Seq:
      pending_.seq = byte;
      crc_acc_ ^= byte;
      state_ = State::Cmd;
      return true;

    case State::Cmd:
      pending_.cmd = byte;
      crc_acc_ ^= byte;
      state_ = (pending_.payload_len == 0U) ? State::Crc : State::Payload;
      return true;

    case State::Payload:
      pending_.payload[payload_index_++] = byte;
      crc_acc_ ^= byte;
      if (payload_index_ >= pending_.payload_len) {
        state_ = State::Crc;
      }
      return true;

    case State::Crc:
      if (byte == crc_acc_) {
        ready_ = pending_;
        has_ready_ = true;
        reset();
        return true;
      }
      reset();
      diag_inc(g_diag.frames_bad_crc);
      return false;
  }

  reset();
  return false;
}

bool FrameParser::pop(Frame& frame) {
  if (!has_ready_) {
    return false;
  }
  frame = ready_;
  has_ready_ = false;
  return true;
}

void FrameParser::reset() {
  state_ = State::Start;
  crc_acc_ = 0;
  payload_index_ = 0;
}

bool AckWriter::push(std::uint8_t seq, std::uint8_t cmd, Status status) {
  if (count_ >= queue_.size()) {
    return false;
  }

  queue_[tail_] = Ack{seq, cmd, status};
  tail_ = (tail_ + 1U) % queue_.size();
  count_++;
  return true;
}

bool AckWriter::pop(Ack& ack) {
  if (count_ == 0U) {
    return false;
  }

  ack = queue_[head_];
  head_ = (head_ + 1U) % queue_.size();
  count_--;
  return true;
}

}  // namespace app
