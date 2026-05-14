/**
* @brief Declares the heapless coroutine task wrapper used by command handlers.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#pragma once

#include "scheduler.hpp"

#include <coroutine>
#include <cstddef>
#include <cstdint>

namespace app
{

    class Task
    {
    public:
        struct promise_type
        {
            /**
            * @brief Allocates a coroutine frame from the fixed task frame pool.
            * @param[in] size Requested coroutine frame size in bytes.
            * @return Pointer to the allocated frame storage, or nullptr on failure.
            */
            static void* operator new(std::size_t size) noexcept;

            /**
            * @brief Releases a coroutine frame back to the fixed task frame pool.
            * @param[in,out] ptr Pointer previously returned by operator new.
            * @param[in] size Size passed by the compiler for the frame being deleted.
            * @return None.
            */
            static void operator delete(void* ptr, std::size_t size) noexcept;

            /**
            * @brief Creates an empty task when coroutine frame allocation fails.
            * @param[in] None No input parameters.
            * @return Empty task object.
            */
            static Task get_return_object_on_allocation_failure() noexcept;

            /**
            * @brief Creates a task object from this promise.
            * @param[in] None No input parameters.
            * @return Task object owning the coroutine handle.
            */
            Task get_return_object() noexcept;

            /**
            * @brief Suspends newly created coroutine tasks until explicitly started.
            * @param[in] None No input parameters.
            * @return Suspend-always awaiter.
            */
            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            /**
            * @brief Keeps completed coroutine frames suspended for scheduler cleanup.
            * @param[in] None No input parameters.
            * @return Suspend-always awaiter.
            */
            std::suspend_always final_suspend() noexcept
            {
                return {};
            }

            /**
            * @brief Handles normal completion of a void coroutine.
            * @param[in] None No input parameters.
            * @return None.
            */
            void return_void() noexcept
            {
            }

            /**
            * @brief Handles unexpected exceptions in a no-exceptions firmware build.
            * @param[in] None No input parameters.
            * @return None.
            */
            void unhandled_exception() noexcept;
        };

        /**
        * @brief Constructs an empty task.
        * @param[in] None No input parameters.
        * @return None.
        */
        Task() = default;

        /**
        * @brief Constructs a task from a coroutine handle.
        * @param[in] handle Coroutine handle to own.
        * @return None.
        */
        explicit Task(std::coroutine_handle<promise_type> handle) : m_handle(handle)
        {
        }

        /**
        * @brief Copy construction is disabled for unique coroutine ownership.
        * @param[in] None Copy source is intentionally unsupported.
        * @return None.
        */
        Task(const Task&) = delete;

        /**
        * @brief Copy assignment is disabled for unique coroutine ownership.
        * @param[in] None Copy source is intentionally unsupported.
        * @return Reference to this task when available; function is deleted.
        */
        Task& operator=(const Task&) = delete;

        /**
        * @brief Move-constructs a task and transfers coroutine ownership.
        * @param[in,out] other Source task to move from.
        * @return None.
        */
        Task(Task&& other) noexcept;

        /**
        * @brief Move-assigns a task and transfers coroutine ownership.
        * @param[in,out] other Source task to move from.
        * @return Reference to this task.
        */
        Task& operator=(Task&& other) noexcept;

        /**
        * @brief Destroys the owned coroutine frame if it has not been scheduled.
        * @param[in] None No input parameters.
        * @return None.
        */
        ~Task();

        /**
        * @brief Reports whether this task owns a valid coroutine handle.
        * @param[in] None No input parameters.
        * @return True when a coroutine handle is owned, otherwise false.
        */
        bool Valid() const
        {
            return static_cast<bool>(m_handle);
        }

        /**
        * @brief Starts the task by scheduling its coroutine handle.
        * @param[in,out] scheduler Scheduler that receives the coroutine handle.
        * @return True when the task was scheduled, otherwise false.
        */
        bool Start(Scheduler& scheduler);

    private:
        std::coroutine_handle<promise_type> m_handle{};
    };
}
