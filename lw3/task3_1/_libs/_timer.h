#pragma once

#include <iostream>
#include <chrono>

class Timer
{
public:
    Timer()
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    void Reset()
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] double GetElapsed() const
    {
        return duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - startTime).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};