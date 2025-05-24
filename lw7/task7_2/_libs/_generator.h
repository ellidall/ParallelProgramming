#pragma once

#include <iostream>
#include <coroutine>
#include <exception>
#include <optional>

template<typename T>
class Generator;

template<typename T>
struct Promise
{
    T* m_valuePtr = nullptr;
    std::exception_ptr m_exception;

    Generator<T> get_return_object();

    std::suspend_always initial_suspend() { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    void unhandled_exception()
    {
        m_exception = std::current_exception();
    }

    template<typename U>
    std::suspend_always yield_value(U&& value)
    {
        
        if (m_valuePtr)
        {
            *m_valuePtr = std::forward<U>(value);
        }
        return {};
    }

    void return_void() {}
};

template<typename T, typename... Args>
struct std::coroutine_traits<Generator<T>, Args...> {
    using promise_type = Promise<T>;
};

template<typename T>
class Generator
{
public:
    using handleType = std::coroutine_handle<Promise<T>>;

    struct iterator
    {
        handleType h;
        std::optional<T> currentValue;

        explicit iterator(handleType h) : h(h)
        {
            if (h) step();
        }

        iterator& operator++()
        {
            if (h) step();
            return *this;
        }

        bool operator==(const iterator& other) const { return h == other.h; }

        bool operator!=(const iterator& other) const { return h != other.h; }

        const T& operator*() const { return *currentValue; }

        const T* operator->() const { return &*currentValue; }

        void step()
        {
            currentValue.reset();
            h.promise().m_valuePtr = &currentValue.emplace();

            h.resume();
            if (h.done())
            {
                if (h.promise().m_exception) std::rethrow_exception(h.promise().m_exception);
                h = nullptr;
                currentValue.reset();
            }
        }
    };

    explicit Generator(handleType h) : m_handle(h) {}

    ~Generator() { Destroy(); }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other) noexcept: m_handle(other.m_handle) { other.m_handle = nullptr; }

    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    iterator begin()
    {
        if (!m_handle || m_handle.done()) return end();
        return iterator(m_handle);
    }

    iterator end() { return iterator(nullptr); }

private:
    handleType m_handle;

    void Destroy()
    {
        if (m_handle) m_handle.destroy();
    }
};

template<typename T>
Generator<T> Promise<T>::get_return_object()
{
    return Generator<T>{std::coroutine_handle<Promise<T>>::from_promise(*this)};
}