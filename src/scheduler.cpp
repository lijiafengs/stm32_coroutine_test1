/**
* @brief Implements the fixed-slot cooperative coroutine scheduler.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "scheduler.hpp"

#include "diagnostics.hpp"
#include "stm32f4xx_hal.h"

namespace app
{

    namespace
    {

        /**
        * @brief Checks whether a deadline tick has been reached with wraparound support.
        * @param[in] now Current system tick in milliseconds.
        * @param[in] deadline Deadline tick in milliseconds.
        * @return True when now has reached or passed the deadline.
        */
        bool TimeReached(std::uint32_t now, std::uint32_t deadline)
        {
            return static_cast<std::int32_t>(now - deadline) >= 0;
        }
    }

    /**
    * @brief Constructs a waiter with a timeout.
    * @param[in] timeoutMs Timeout duration in milliseconds, or zero for no timeout.
    * @return None.
    */
    Waiter::Waiter(std::uint32_t timeoutMs) : m_timeoutMs(timeoutMs)
    {
    }

    /**
    * @brief Tests whether a suspended coroutine should resume.
    * @param[in] now Current system tick in milliseconds.
    * @return True when the condition is ready or timed out.
    */
    bool Waiter::ShouldResume(std::uint32_t now)
    {
        if (!m_armed)
        {
            m_armed = true;
            m_deadline = now + m_timeoutMs;
        }

        if (Ready(now))
        {
            return true;
        }

        if ((m_timeoutMs != 0U) && TimeReached(now, m_deadline))
        {
            m_timedOut = true;
            DiagInc(g_diagnostics.m_schedulerTimeouts);
            return true;
        }

        return false;
    }

    /**
    * @brief Adds a coroutine handle to the ready queue.
    * @param[in] handle Coroutine handle to schedule.
    * @return True when queued, otherwise false when no slot is available.
    */
    bool Scheduler::Schedule(std::coroutine_handle<> handle)
    {
        if (!handle)
        {
            return false;
        }

        for (auto& slot : m_ready)
        {
            if (!slot.used)
            {
                slot.handle = handle;
                slot.used = true;
                return true;
            }
        }
        return false;
    }

    /**
    * @brief Adds a coroutine handle and wait condition to the waiting queue.
    * @param[in] handle Coroutine handle to suspend.
    * @param[in,out] waiter Wait condition stored in the coroutine frame.
    * @return True when queued, otherwise false when no slot is available.
    */
    bool Scheduler::Wait(std::coroutine_handle<> handle, Waiter& waiter)
    {
        if (!handle)
        {
            return false;
        }

        for (auto& slot : m_waiting)
        {
            if (!slot.used)
            {
                slot.handle = handle;
                slot.waiter = &waiter;
                slot.used = true;
                return true;
            }
        }
        return false;
    }

    /**
    * @brief Runs one scheduler pass over waiting and ready slots.
    * @param[in] None No input parameters.
    * @return None.
    */
    void Scheduler::Poll()
    {
        const std::uint32_t now = HAL_GetTick();

        for (auto& slot : m_waiting)
        {
            if (slot.used && (slot.waiter != nullptr) && slot.waiter->ShouldResume(now))
            {
                auto handle = slot.handle;
                slot = {};
                (void)Schedule(handle);
            }
        }

        for (auto& slot : m_ready)
        {
            if (!slot.used)
            {
                continue;
            }

            auto handle = slot.handle;
            slot = {};

            if (!handle || handle.done())
            {
                if (handle)
                {
                    handle.destroy();
                }
                continue;
            }

            DiagInc(g_diagnostics.m_schedulerResumes);
            handle.resume();

            if (handle.done())
            {
                handle.destroy();
            }
        }
    }

    /**
    * @brief Reports whether any scheduler slot is active.
    * @param[in] None No input parameters.
    * @return True when a ready or waiting slot is used.
    */
    bool Scheduler::HasWork() const
    {
        for (const auto& slot : m_ready)
        {
            if (slot.used)
            {
                return true;
            }
        }
        for (const auto& slot : m_waiting)
        {
            if (slot.used)
            {
                return true;
            }
        }
        return false;
    }

    /**
    * @brief Returns the global scheduler instance.
    * @param[in] None No input parameters.
    * @return Reference to the scheduler singleton.
    */
    Scheduler& GetScheduler()
    {
        static Scheduler instance;
        return instance;
    }
}
