/**
* @brief Declares motor abstraction, simulated motor, and motor completion awaiter.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include "scheduler.hpp"

#include <array>
#include <cstdint>

namespace app
{

    class IMotor
    {
    public:
        /**
        * @brief Destroys a motor interface object.
        * @param[in] None No input parameters.
        * @return None.
        */
        virtual ~IMotor() = default;

        /**
        * @brief Starts a relative step move.
        * @param[in] steps Number of relative steps to move.
        * @return True when the move starts, otherwise false when busy.
        */
        virtual bool Move(std::int32_t steps) = 0;

        /**
        * @brief Reports whether the motor has completed its current move.
        * @param[in] None No input parameters.
        * @return True when the motor is done, otherwise false.
        */
        virtual bool Done() const = 0;

        /**
        * @brief Polls the motor state machine.
        * @param[in] now Current system tick in milliseconds.
        * @return None.
        */
        virtual void Poll(std::uint32_t now) = 0;
    };

    class SimMotor final : public IMotor
    {
    public:
        /**
        * @brief Constructs a simulated motor with an identifier.
        * @param[in] id Motor identifier used by the demo motor table.
        * @return None.
        */
        explicit SimMotor(std::uint8_t id) : m_id(id)
        {
        }

        /**
        * @brief Starts a simulated relative step move.
        * @param[in] steps Number of relative steps to simulate.
        * @return True when the move starts, otherwise false when busy.
        */
        bool Move(std::int32_t steps) override;

        /**
        * @brief Reports whether the simulated motor is done.
        * @param[in] None No input parameters.
        * @return True when the simulated motor is done, otherwise false.
        */
        bool Done() const override
        {
            return m_done;
        }

        /**
        * @brief Polls the simulated motor state machine.
        * @param[in] now Current system tick in milliseconds.
        * @return None.
        */
        void Poll(std::uint32_t now) override;

        /**
        * @brief Reports whether the simulated motor is busy.
        * @param[in] None No input parameters.
        * @return True when a move is in progress, otherwise false.
        */
        bool Busy() const
        {
            return m_busy;
        }

        /**
        * @brief Returns the last requested step count.
        * @param[in] None No input parameters.
        * @return Last requested step count.
        */
        std::int32_t LastSteps() const
        {
            return m_lastSteps;
        }

    private:
        std::uint8_t m_id = 0;
        bool m_busy = false;
        bool m_done = true;
        std::uint32_t m_completeAt = 0;
        std::int32_t m_lastSteps = 0;
    };

    class MotorDone final : public Waiter
    {
    public:
        /**
        * @brief Constructs an awaiter that waits for motor completion.
        * @param[in,out] motor Motor instance to observe.
        * @param[in] timeoutMs Timeout duration in milliseconds.
        * @return None.
        */
        MotorDone(IMotor& motor, std::uint32_t timeoutMs) : Waiter(timeoutMs), m_motor(motor)
        {
        }

        /**
        * @brief Checks whether the coroutine can continue without suspension.
        * @param[in] None No input parameters.
        * @return True when the motor is already done, otherwise false.
        */
        bool await_ready() const
        {
            return m_motor.Done();
        }

        /**
        * @brief Suspends the coroutine in the scheduler waiting queue.
        * @param[in] handle Coroutine handle to suspend.
        * @return True when the coroutine should remain suspended.
        */
        bool await_suspend(std::coroutine_handle<> handle);

        /**
        * @brief Returns the motor wait result after resumption.
        * @param[in] None No input parameters.
        * @return True when completed before timeout, otherwise false.
        */
        bool await_resume() const
        {
            return !TimedOut();
        }

    protected:
        /**
        * @brief Checks whether the observed motor is complete.
        * @param[in] now Current system tick in milliseconds.
        * @return True when the motor is done, otherwise false.
        */
        bool Ready(std::uint32_t now) const override;

    private:
        IMotor& m_motor;
    };

    /**
    * @brief Returns a motor instance by identifier.
    * @param[in] id Motor identifier.
    * @return Reference to the selected motor instance.
    */
    IMotor& GetMotor(std::uint8_t id);

    /**
    * @brief Polls all registered demo motors.
    * @param[in] now Current system tick in milliseconds.
    * @return None.
    */
    void PollMotors(std::uint32_t now);
}
