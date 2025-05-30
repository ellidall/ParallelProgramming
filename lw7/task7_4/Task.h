#pragma once

#include <coroutine>
#include <optional>
#include <exception>
#include <utility>
#include <type_traits>
#include <stdexcept>

template <typename T>
struct Task;

template <typename PromiseType>
struct TaskPromiseBase {
    std::exception_ptr m_exception;
    std::coroutine_handle<> m_continuationHandle;

    std::suspend_always initial_suspend() noexcept { return {}; }

        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseType> h) noexcept {
                std::coroutine_handle<> continuation = h.promise().m_continuationHandle;
                if (continuation) {
                    return continuation;
                }
                return std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };
        FinalAwaiter final_suspend() noexcept { return {}; }

    void unhandled_exception() {
        m_exception = std::current_exception();
    }

    void SetContinuation(std::coroutine_handle<> cont) {
        m_continuationHandle = cont;
    }
};

template <typename T>
struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    // Определение promise_type для Task<T>
    struct promise_type : TaskPromiseBase<promise_type> {
        std::optional<T> m_value;

        Task<T> get_return_object() { // Возвращает Task<T>
            return Task<T>{handle_type::from_promise(*this)};
        }

        void return_value(T val) {
            m_value = std::move(val);
        }

        T GetResult() {
            if (this->m_exception) {
                std::rethrow_exception(this->m_exception);
            }
            if (!m_value.has_value()) {
                throw std::runtime_error("Task<T>: result was not set or already moved.");
            }
            return std::move(*m_value);
        }
    };

    handle_type m_coroHandle;

    explicit Task(handle_type h) : m_coroHandle(h) {} // Конструктор должен быть explicit, если он с одним аргументом
    Task(Task&& other) noexcept : m_coroHandle(std::exchange(other.m_coroHandle, nullptr)) {}
    ~Task() {
        if (m_coroHandle) {
            m_coroHandle.destroy();
        }
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_coroHandle) m_coroHandle.destroy();
            m_coroHandle = std::exchange(other.m_coroHandle, nullptr);
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    auto operator co_await() noexcept {
        struct Awaiter {
            handle_type m_taskCoroHandle;

            bool await_ready() noexcept {
                return !m_taskCoroHandle || m_taskCoroHandle.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitingCoro) noexcept {
                m_taskCoroHandle.promise().SetContinuation(awaitingCoro);
                return m_taskCoroHandle;
            }

            T await_resume() {
                return m_taskCoroHandle.promise().GetResult();
            }
        };
        return Awaiter{m_coroHandle};
    }

    void Start() {
        if (m_coroHandle && !m_coroHandle.done()) {
            m_coroHandle.resume();
        }
    }

    explicit operator bool() const { return m_coroHandle != nullptr; }

    T Get() {
        if (!m_coroHandle) throw std::runtime_error("Task::Get() on invalid task");
        if (!m_coroHandle.done()) {
            throw std::runtime_error("Task::Get() called on a task that is not yet complete.");
        }
        return m_coroHandle.promise().GetResult();
    }
};

template <>
struct Task<void> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type : TaskPromiseBase<promise_type> {
        Task<void> get_return_object() {
            return Task<void>{handle_type::from_promise(*this)};
        }

        void return_void() {}

        void GetResult() {
            if (this->m_exception) {
                std::rethrow_exception(this->m_exception);
            }
        }
    };

    handle_type m_coroHandle;

    explicit Task(handle_type h) : m_coroHandle(h) {}
    Task(Task&& other) noexcept : m_coroHandle(std::exchange(other.m_coroHandle, nullptr)) {}
    ~Task() {
        if (m_coroHandle) {
            m_coroHandle.destroy();
        }
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_coroHandle) m_coroHandle.destroy();
            m_coroHandle = std::exchange(other.m_coroHandle, nullptr);
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    auto operator co_await() noexcept {
        struct Awaiter {
            handle_type m_taskCoroHandle;

            bool await_ready() noexcept {
                return !m_taskCoroHandle || m_taskCoroHandle.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitingCoro) noexcept {
                m_taskCoroHandle.promise().SetContinuation(awaitingCoro);
                return m_taskCoroHandle;
            }

            void await_resume() {
                m_taskCoroHandle.promise().GetResult();
            }
        };
        return Awaiter{m_coroHandle};
    }

    void Start() {
        if (m_coroHandle && !m_coroHandle.done()) {
            m_coroHandle.resume();
        }
    }

    explicit operator bool() const { return m_coroHandle != nullptr; }
};
