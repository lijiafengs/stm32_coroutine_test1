/**
* @brief Implements simulated motors and motor completion awaiter.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "motor.hpp"

#include "app_config.hpp"
#include "diagnostics.hpp"
#include "stm32f4xx_hal.h"

namespace app
{

    namespace
    {

        std::array<SimMotor, 2> g_motors{SimMotor{1}, SimMotor{2}};

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
    * @brief Starts a simulated motor move.
    * @param[in] steps Number of relative steps to record for the simulated move.
    * @return True when the move starts, otherwise false when the motor is busy.
    */
    bool SimMotor::Move(std::int32_t steps)
    {
        if (m_busy)
        {
            return false;
        }

        m_lastSteps = steps;
        m_busy = true;
        m_done = false;
        m_completeAt = HAL_GetTick() + kSimMotorMoveMs;
        DiagInc(g_diagnostics.m_motorStarted);
        (void)m_id;
        return true;
    }

    /**
    * @brief Updates the simulated motor completion state.
    * @param[in] now Current system tick in milliseconds.
    * @return None.
    */
    void SimMotor::Poll(std::uint32_t now)
    {
        if (m_busy && TimeReached(now, m_completeAt))
        {
            m_busy = false;
            m_done = true;
            DiagInc(g_diagnostics.m_motorCompleted);
        }
    }

    /**
    * @brief Suspends a coroutine until the observed motor is done or timed out.
    * @param[in] handle Coroutine handle to put into the scheduler waiting queue.
    * @return True when the coroutine is suspended.
    */
    bool MotorDone::await_suspend(std::coroutine_handle<> handle)
    {
        return GetScheduler().Wait(handle, *this);
    }

    /**
    * @brief Checks whether the observed motor has completed.
    * @param[in] now Current system tick in milliseconds.
    * @return True when the motor is done, otherwise false.
    */
    bool MotorDone::Ready(std::uint32_t) const
    {
        return m_motor.Done();
    }

    /**
    * @brief Returns a demo motor by identifier.
    * @param[in] id Motor identifier; id 2 selects the second motor, all others select first.
    * @return Reference to the selected simulated motor through the motor interface.
    */
    IMotor& GetMotor(std::uint8_t id)
    {
        if (id == 2U)
        {
            return g_motors[1];
        }
        return g_motors[0];
    }

    /**
    * @brief Polls every simulated motor instance.
    * @param[in] now Current system tick in milliseconds.
    * @return None.
    */
    void PollMotors(std::uint32_t now)
    {
        for (auto& motor : g_motors)
        {
            motor.Poll(now);
        }
    }
}
