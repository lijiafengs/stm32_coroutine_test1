/**
* @brief Declares protocol frames, ACK queue, CRC helper, and byte stream parser.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include "app_config.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace app
{

    enum class Command : std::uint8_t
    {
        eMoveStep = 0x01,
    };

    enum class Status : std::uint8_t
    {
        eOk = 0x00,
        eBadCrc = 0x01,
        eUnsupported = 0x02,
        eBusy = 0x03,
        eTimeout = 0x04,
        eInvalidPayload = 0x05,
        eQueueFull = 0x06,
    };

    struct Frame
    {
        std::uint8_t m_seq = 0;
        std::uint8_t m_cmd = 0;
        std::uint8_t m_payloadLen = 0;
        std::array<std::uint8_t, kMaxPayloadBytes> m_payload{};
    };

    struct Ack
    {
        std::uint8_t m_seq = 0;
        std::uint8_t m_cmd = 0;
        Status m_status = Status::eOk;
    };

    /**
    * @brief Calculates the protocol XOR CRC over a byte range.
    * @param[in] data Pointer to the first byte in the input range.
    * @param[in] size Number of bytes to include in the CRC.
    * @return Calculated CRC byte.
    */
    std::uint8_t Crc8Xor(const std::uint8_t* data, std::size_t size);

    /**
    * @brief Encodes a frame into the wire-format byte buffer.
    * @param[in] frame Frame fields to encode.
    * @param[out] out Destination byte buffer.
    * @param[in] outSize Size of the destination byte buffer.
    * @return Number of bytes written, or zero when the destination is invalid.
    */
    std::size_t EncodeFrame(const Frame& frame, std::uint8_t* out, std::size_t outSize);

    class FrameParser
    {
    public:
        /**
        * @brief Pushes one received byte into the parser state machine.
        * @param[in] byte Received byte to process.
        * @return True when the byte was accepted, otherwise false on parse error.
        */
        bool Push(std::uint8_t byte);

        /**
        * @brief Pops one complete decoded frame if available.
        * @param[out] frame Destination frame populated when a complete frame exists.
        * @return True when a frame was popped, otherwise false.
        */
        bool Pop(Frame& frame);

    private:
        enum class State : std::uint8_t
        {
            eStart,
            eLength,
            eSeq,
            eCmd,
            ePayload,
            eCrc
        };

        /**
        * @brief Resets the parser state machine to the start state.
        * @param[in] None No input parameters.
        * @return None.
        */
        void Reset();

        State m_state = State::eStart;
        Frame m_pending{};
        Frame m_ready{};
        bool m_hasReady = false;
        std::uint8_t m_crcAcc = 0;
        std::uint8_t m_payloadIndex = 0;
    };

    class AckWriter
    {
    public:
        /**
        * @brief Pushes one ACK record into the fixed-size ACK queue.
        * @param[in] seq Sequence value copied from the received frame.
        * @param[in] cmd Command value copied from the received frame.
        * @param[in] status Result status to report.
        * @return True when the ACK was queued, otherwise false when the queue is full.
        */
        bool Push(std::uint8_t seq, std::uint8_t cmd, Status status);

        /**
        * @brief Pops one ACK record from the fixed-size ACK queue.
        * @param[out] ack Destination ACK record populated when one is available.
        * @return True when an ACK was popped, otherwise false.
        */
        bool Pop(Ack& ack);

    private:
        std::array<Ack, kAckQueueDepth> m_queue{};
        std::size_t m_head = 0;
        std::size_t m_tail = 0;
        std::size_t m_count = 0;
    };
}
