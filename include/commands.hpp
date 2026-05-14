/**
* @brief Declares command handlers and opcode dispatcher.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include "frame.hpp"
#include "task.hpp"

namespace app
{

    class Commands
    {
    public:
        /**
        * @brief Constructs the command handler group with an ACK writer.
        * @param[in,out] ackWriter ACK writer used by command handlers.
        * @return None.
        */
        explicit Commands(AckWriter& ackWriter) : m_ackWriter(ackWriter)
        {
        }

        /**
        * @brief Handles the MoveStep command as a non-blocking coroutine.
        * @param[in] frame Received command frame.
        * @return Coroutine task object that must be started by the scheduler.
        */
        Task MoveStep(Frame frame);

    private:
        AckWriter& m_ackWriter;
    };

    class CommandDispatcher
    {
    public:
        /**
        * @brief Constructs the command dispatcher.
        * @param[in,out] commands Command handler collection.
        * @param[in,out] ackWriter ACK writer used for immediate dispatch errors.
        * @param[in,out] scheduler Coroutine scheduler used to start command tasks.
        * @return None.
        */
        CommandDispatcher(Commands& commands, AckWriter& ackWriter, Scheduler& scheduler)
            : m_commands(commands), m_ackWriter(ackWriter), m_scheduler(scheduler)
        {
        }

        /**
        * @brief Dispatches a decoded frame to the matching command handler.
        * @param[in] frame Decoded protocol frame.
        * @return None.
        */
        void Dispatch(const Frame& frame);

    private:
        Commands& m_commands;
        AckWriter& m_ackWriter;
        Scheduler& m_scheduler;
    };
}
