#pragma once

#include <atomic>
#include <utility>

template <typename T>
class AtomicMax
{
public:
    explicit AtomicMax(T value) : m_value(value) {}

    void Update(T newValue) noexcept
    {
        auto current = GetValue();
        do {
            if (newValue <= current) {
                return;
            }
        } while (!m_value.compare_exchange_weak(current, newValue,
                std::memory_order_release,
                std::memory_order_relaxed));
    }

    //поправить барьеры

    T GetValue() const noexcept
    {
        return m_value.load(std::memory_order_relaxed);
    }

private:
    std::atomic<T> m_value;
};
