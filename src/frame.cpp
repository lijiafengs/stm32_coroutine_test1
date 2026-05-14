/**
* @brief Implements protocol CRC, frame encoding, parser, and ACK queue.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "frame.hpp"

#include "diagnostics.hpp"

namespace app
{

    namespace
    {

        constexpr std::uint8_t kSof = 0xA5;
    }

    /**
    * @brief Calculates the XOR CRC used by the demo protocol.
    * @param[in] data Pointer to the first byte in the input range.
    * @param[in] size Number of bytes to include in the CRC.
    * @return Calculated CRC byte.
    */
    std::uint8_t Crc8Xor(const std::uint8_t* data, std::size_t size)
    {
        std::uint8_t crc = 0;
        for (std::size_t i = 0; i < size; ++i)
        {
            crc ^= data[i];
        }
        return crc;
    }

    /**
    * @brief Encodes an application frame into wire-format bytes.
    * @param[in] frame Frame fields to encode.
    * @param[out] out Destination byte buffer.
    * @param[in] outSize Size of the destination byte buffer.
    * @return Number of encoded bytes, or zero on invalid arguments.
    */
    std::size_t EncodeFrame(const Frame& frame, std::uint8_t* out, std::size_t outSize)
    {
        const std::size_t total = static_cast<std::size_t>(frame.m_payloadLen) + 5U;
        if ((out == nullptr) || (outSize < total) || (frame.m_payloadLen > kMaxPayloadBytes))
        {
            return 0;
        }

        out[0] = kSof;
        out[1] = frame.m_payloadLen;
        out[2] = frame.m_seq;
        out[3] = frame.m_cmd;
        for (std::uint8_t i = 0; i < frame.m_payloadLen; ++i)
        {
            out[4U + i] = frame.m_payload[i];
        }
        out[4U + frame.m_payloadLen] =
            Crc8Xor(&out[1], static_cast<std::size_t>(frame.m_payloadLen) + 3U);
        return total;
    }

    /**
    * @brief Feeds one byte into the protocol parser state machine.
    * @param[in] byte Received byte to parse.
    * @return True when the byte was accepted, otherwise false on parse error.
    */
    bool FrameParser::Push(std::uint8_t byte)
    {
        switch (m_state)
        {
        case State::eStart:
            if (byte == kSof)
            {
                m_pending = {};
                m_crcAcc = 0;
                m_payloadIndex = 0;
                m_state = State::eLength;
            }
            return true;

        case State::eLength:
            if (byte > kMaxPayloadBytes)
            {
                Reset();
                return false;
            }
            m_pending.m_payloadLen = byte;
            m_crcAcc ^= byte;
            m_state = State::eSeq;
            return true;

        case State::eSeq:
            m_pending.m_seq = byte;
            m_crcAcc ^= byte;
            m_state = State::eCmd;
            return true;

        case State::eCmd:
            m_pending.m_cmd = byte;
            m_crcAcc ^= byte;
            m_state = (m_pending.m_payloadLen == 0U) ? State::eCrc : State::ePayload;
            return true;

        case State::ePayload:
            m_pending.m_payload[m_payloadIndex++] = byte;
            m_crcAcc ^= byte;
            if (m_payloadIndex >= m_pending.m_payloadLen)
            {
                m_state = State::eCrc;
            }
            return true;

        case State::eCrc:
            if (byte == m_crcAcc)
            {
                m_ready = m_pending;
                m_hasReady = true;
                Reset();
                return true;
            }
            Reset();
            DiagInc(g_diagnostics.m_framesBadCrc);
            return false;
        }

        Reset();
        return false;
    }

    /**
    * @brief Retrieves one complete frame from the parser.
    * @param[out] frame Destination frame populated when a frame is ready.
    * @return True when a frame was retrieved, otherwise false.
    */
    bool FrameParser::Pop(Frame& frame)
    {
        if (!m_hasReady)
        {
            return false;
        }
        frame = m_ready;
        m_hasReady = false;
        return true;
    }

    /**
    * @brief Resets parser state after a complete frame or parse error.
    * @param[in] None No input parameters.
    * @return None.
    */
    void FrameParser::Reset()
    {
        m_state = State::eStart;
        m_crcAcc = 0;
        m_payloadIndex = 0;
    }

    /**
    * @brief Queues an ACK record for later transmission.
    * @param[in] seq Sequence value copied from the received frame.
    * @param[in] cmd Command value copied from the received frame.
    * @param[in] status Command result status.
    * @return True when queued, otherwise false when the queue is full.
    */
    bool AckWriter::Push(std::uint8_t seq, std::uint8_t cmd, Status status)
    {
        if (m_count >= m_queue.size())
        {
            return false;
        }

        m_queue[m_tail] = Ack{seq, cmd, status};
        m_tail = (m_tail + 1U) % m_queue.size();
        m_count++;
        return true;
    }

    /**
    * @brief Retrieves one queued ACK record.
    * @param[out] ack Destination ACK record populated when available.
    * @return True when an ACK was retrieved, otherwise false.
    */
    bool AckWriter::Pop(Ack& ack)
    {
        if (m_count == 0U)
        {
            return false;
        }

        ack = m_queue[m_head];
        m_head = (m_head + 1U) % m_queue.size();
        m_count--;
        return true;
    }
}
