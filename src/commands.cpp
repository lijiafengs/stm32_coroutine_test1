/**
* @brief Implements coroutine command handlers and opcode dispatch.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "commands.hpp"

#include "app_config.hpp"
#include "diagnostics.hpp"
#include "motor.hpp"

namespace app
{

    namespace
    {

        /**
        * @brief Reads a signed 32-bit little-endian value from a byte buffer.
        * @param[in] data Pointer to at least four little-endian bytes.
        * @return Decoded signed 32-bit value.
        */
        std::int32_t ReadI32Le(const std::uint8_t* data)
        {
            std::uint32_t value = static_cast<std::uint32_t>(data[0]);
            value |= static_cast<std::uint32_t>(data[1]) << 8U;
            value |= static_cast<std::uint32_t>(data[2]) << 16U;
            value |= static_cast<std::uint32_t>(data[3]) << 24U;
            return static_cast<std::int32_t>(value);
        }
    }

    /**
    * @brief Handles the MoveStep command without blocking the main loop.
    * @param[in] frame Received command frame containing motor id and step count.
    * @return Coroutine task that completes after motor completion or timeout.
    */
    Task Commands::MoveStep(Frame frame)
    {
        DiagInc(g_diagnostics.m_commandStarted);

        if (frame.m_payloadLen < 5U)
        {
            (void)m_ackWriter.Push(frame.m_seq, frame.m_cmd, Status::eInvalidPayload);
            co_return;
        }

        const std::uint8_t motorId = frame.m_payload[0];
        const std::int32_t steps = ReadI32Le(&frame.m_payload[1]);
        IMotor& motor = GetMotor(motorId);

        if (!motor.Move(steps))
        {
            (void)m_ackWriter.Push(frame.m_seq, frame.m_cmd, Status::eBusy);
            co_return;
        }

        const bool completed = co_await MotorDone{motor, kMotorTimeoutMs};
        (void)m_ackWriter.Push(
            frame.m_seq, frame.m_cmd, completed ? Status::eOk : Status::eTimeout);
    }

    /**
    * @brief Dispatches a frame to the command coroutine selected by opcode.
    * @param[in] frame Decoded protocol frame to dispatch.
    * @return None.
    */
    void CommandDispatcher::Dispatch(const Frame& frame)
    {
        switch (static_cast<Command>(frame.m_cmd))
        {
        case Command::eMoveStep:
        {
            Task task = m_commands.MoveStep(frame);
            if (!task.Start(m_scheduler))
            {
                DiagInc(g_diagnostics.m_commandBusy);
                (void)m_ackWriter.Push(frame.m_seq, frame.m_cmd, Status::eBusy);
            }
            break;
        }

        default:
            DiagInc(g_diagnostics.m_commandUnknown);
            (void)m_ackWriter.Push(frame.m_seq, frame.m_cmd, Status::eUnsupported);
            break;
        }
    }
}
