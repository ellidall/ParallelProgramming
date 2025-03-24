#pragma once

#include <iostream>
#include <chrono>

class Timer
{
public:
    Timer()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void Reset()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] double GetElapsed() const
    {
        return duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - m_startTime).count();
    }

    template<typename Duration>
    [[nodiscard]] auto GetElapsed() const
    {
        return std::chrono::duration_cast<Duration>(std::chrono::high_resolution_clock::now() - m_startTime);
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};