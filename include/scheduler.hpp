/**
* @brief Declares the cooperative coroutine scheduler and wait condition base class.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include "app_config.hpp"

#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>

namespace app
{

    class Waiter
    {
    public:
        /**
        * @brief Constructs a wait condition with a timeout in milliseconds.
        * @param[in] timeoutMs Timeout duration in milliseconds, or zero for no timeout.
        * @return None.
        */
        explicit Waiter(std::uint32_t timeoutMs);

        /**
        * @brief Destroys the wait condition base object.
        * @param[in] None No input parameters.
        * @return None.
        */
        virtual ~Waiter() = default;

        /**
        * @brief Checks whether the waiting coroutine should resume.
        * @param[in] now Current system tick in milliseconds.
        * @return True when the condition is ready or timed out, otherwise false.
        */
        bool ShouldResume(std::uint32_t now);

        /**
        * @brief Reports whether this waiter ended because of timeout.
        * @param[in] None No input parameters.
        * @return True when the wait timed out, otherwise false.
        */
        bool TimedOut() const
        {
            return m_timedOut;
        }

    protected:
        /**
        * @brief Tests the concrete wait condition.
        * @param[in] now Current system tick in milliseconds.
        * @return True when the condition is ready, otherwise false.
        */
        virtual bool Ready(std::uint32_t now) const = 0;

    private:
        std::uint32_t m_deadline = 0;
        std::uint32_t m_timeoutMs = 0;
        bool m_armed = false;
        bool m_timedOut = false;
    };

    class Scheduler
    {
    public:
        /**
        * @brief Schedules a coroutine handle to resume on a later poll.
        * @param[in] handle Coroutine handle to place into the ready queue.
        * @return True when scheduled, otherwise false when no slot is available.
        */
        bool Schedule(std::coroutine_handle<> handle);

        /**
        * @brief Places a coroutine handle into the waiting queue.
        * @param[in] handle Coroutine handle to suspend.
        * @param[in,out] waiter Wait condition owned by the suspended coroutine frame.
        * @return True when queued, otherwise false when no slot is available.
        */
        bool Wait(std::coroutine_handle<> handle, Waiter& waiter);

        /**
        * @brief Polls waiting and ready coroutine queues once.
        * @param[in] None No input parameters.
        * @return None.
        */
        void Poll();

        /**
        * @brief Reports whether the scheduler has any active work.
        * @param[in] None No input parameters.
        * @return True when at least one ready or waiting slot is used.
        */
        bool HasWork() const;

    private:
        struct ReadySlot
        {
            std::coroutine_handle<> handle{};
            bool used = false;
        };

        struct WaitSlot
        {
            std::coroutine_handle<> handle{};
            Waiter* waiter = nullptr;
            bool used = false;
        };

        std::array<ReadySlot, kMaxActiveTasks> m_ready{};
        std::array<WaitSlot, kMaxActiveTasks> m_waiting{};
    };

    /**
    * @brief Returns the global scheduler singleton.
    * @param[in] None No input parameters.
    * @return Reference to the global scheduler instance.
    */
    Scheduler& GetScheduler();
}
