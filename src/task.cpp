/**
* @brief Implements heapless coroutine task frame allocation and ownership.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "task.hpp"

#include <cstddef>
#include <cstdint>

namespace app
{

    namespace
    {

        constexpr std::size_t kTaskFrameBytes = 512;

        struct TaskFrameSlot
        {
            alignas(std::max_align_t) std::uint8_t bytes[kTaskFrameBytes];
            bool used = false;
        };

        TaskFrameSlot g_task_slots[kMaxActiveTasks];
    }

    /**
    * @brief Allocates a coroutine frame from the fixed task frame pool.
    * @param[in] size Requested coroutine frame size in bytes.
    * @return Pointer to frame storage, or nullptr when no suitable slot is available.
    */
    void* Task::promise_type::operator new(std::size_t size) noexcept
    {
        if (size > kTaskFrameBytes)
        {
            return nullptr;
        }

        for (auto& slot : g_task_slots)
        {
            if (!slot.used)
            {
                slot.used = true;
                return slot.bytes;
            }
        }
        return nullptr;
    }

    /**
    * @brief Releases a coroutine frame back to the fixed task frame pool.
    * @param[in,out] ptr Pointer to the coroutine frame storage to release.
    * @param[in] size Size passed by the compiler for the frame being deleted.
    * @return None.
    */
    void Task::promise_type::operator delete(void* ptr, std::size_t) noexcept
    {
        if (ptr == nullptr)
        {
            return;
        }

        for (auto& slot : g_task_slots)
        {
            if (ptr == static_cast<void*>(slot.bytes))
            {
                slot.used = false;
                return;
            }
        }
    }

    /**
    * @brief Creates an empty task when coroutine frame allocation fails.
    * @param[in] None No input parameters.
    * @return Empty task object.
    */
    Task Task::promise_type::get_return_object_on_allocation_failure() noexcept
    {
        return {};
    }

    /**
    * @brief Creates a task object from this promise.
    * @param[in] None No input parameters.
    * @return Task object that owns the coroutine handle.
    */
    Task Task::promise_type::get_return_object() noexcept
    {
        return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    /**
    * @brief Handles unexpected exceptions in a no-exceptions firmware build.
    * @param[in] None No input parameters.
    * @return None.
    */
    void Task::promise_type::unhandled_exception() noexcept
    {
        while (true)
        {
        }
    }

    /**
    * @brief Move-constructs a task by taking another task's coroutine handle.
    * @param[in,out] other Source task whose handle is transferred.
    * @return None.
    */
    Task::Task(Task&& other) noexcept : m_handle(other.m_handle)
    {
        other.m_handle = {};
    }

    /**
    * @brief Move-assigns a task by taking another task's coroutine handle.
    * @param[in,out] other Source task whose handle is transferred.
    * @return Reference to this task.
    */
    Task& Task::operator=(Task&& other) noexcept
    {
        if (this != &other)
        {
            if (m_handle)
            {
                m_handle.destroy();
            }
            m_handle = other.m_handle;
            other.m_handle = {};
        }
        return *this;
    }

    /**
    * @brief Destroys the task and any still-owned coroutine frame.
    * @param[in] None No input parameters.
    * @return None.
    */
    Task::~Task()
    {
        if (m_handle)
        {
            m_handle.destroy();
        }
    }

    /**
    * @brief Starts the task by handing its coroutine handle to the scheduler.
    * @param[in,out] scheduler Scheduler that receives the coroutine handle.
    * @return True when scheduled, otherwise false.
    */
    bool Task::Start(Scheduler& scheduler)
    {
        if (!m_handle)
        {
            return false;
        }

        if (!scheduler.Schedule(m_handle))
        {
            return false;
        }

        m_handle = {};
        return true;
    }
}
