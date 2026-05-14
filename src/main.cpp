/**
* @brief Implements the bare-metal main loop for the coroutine command demo.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "commands.hpp"
#include "diagnostics.hpp"
#include "frame.hpp"
#include "motor.hpp"
#include "scheduler.hpp"
#include "stm32f4xx_hal.h"

namespace
{

    app::FrameParser g_parser;
    app::AckWriter g_ackWriter;
    app::Commands g_commands{g_ackWriter};
    app::CommandDispatcher g_dispatcher{g_commands, g_ackWriter, app::GetScheduler()};

    /**
    * @brief Injects one in-memory MoveStep demo frame into the parser.
    * @param[in] None No input parameters.
    * @return None.
    */
    void PushDemoFrame()
    {
        app::Frame frame{};
        frame.m_seq = 1;
        frame.m_cmd = static_cast<std::uint8_t>(app::Command::eMoveStep);
        frame.m_payloadLen = 5;
        frame.m_payload[0] = 1;

        const std::int32_t steps = 100;
        frame.m_payload[1] = static_cast<std::uint8_t>(steps & 0xFF);
        frame.m_payload[2] = static_cast<std::uint8_t>((steps >> 8) & 0xFF);
        frame.m_payload[3] = static_cast<std::uint8_t>((steps >> 16) & 0xFF);
        frame.m_payload[4] = static_cast<std::uint8_t>((steps >> 24) & 0xFF);

        std::uint8_t bytes[32]{};
        const std::size_t size = app::EncodeFrame(frame, bytes, sizeof(bytes));
        for (std::size_t i = 0; i < size; ++i)
        {
            (void)g_parser.Push(bytes[i]);
        }
    }

    /**
    * @brief Drains parsed frames and dispatches each command frame.
    * @param[in] None No input parameters.
    * @return None.
    */
    void DrainFrames()
    {
        app::Frame frame{};
        while (g_parser.Pop(frame))
        {
            app::DiagInc(app::g_diagnostics.m_framesOk);
            g_dispatcher.Dispatch(frame);
        }
    }

    /**
    * @brief Drains ACK records and updates debug diagnostics.
    * @param[in] None No input parameters.
    * @return None.
    */
    void DrainAcks()
    {
        app::Ack ack{};
        while (g_ackWriter.Pop(ack))
        {
            app::g_diagnostics.m_lastStatus = static_cast<std::uint8_t>(ack.m_status);
            if (ack.m_status == app::Status::eOk)
            {
                app::DiagInc(app::g_diagnostics.m_ackOk);
            }
            else
            {
                app::DiagInc(app::g_diagnostics.m_ackError);
            }
        }
    }
}

/**
* @brief Firmware entry point.
* @param[in] None No input parameters.
* @return This function does not return during normal firmware execution.
*/
extern "C" int main(void)
{
    HAL_Init();
    PushDemoFrame();

    while (true)
    {
        app::DiagInc(app::g_diagnostics.m_loopCount);
        DrainFrames();
        app::PollMotors(HAL_GetTick());
        app::GetScheduler().Poll();
        DrainAcks();
    }
}
