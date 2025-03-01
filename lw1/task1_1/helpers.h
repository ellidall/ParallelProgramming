#pragma once

#include <iostream>
#include <cstring>
#include <chrono>
#include <sstream>

// Throws an exception if the result < 0
// Returns the result of the wrapped function execution
template<typename Func, typename... Args>
auto CheckFunctionCall(Func func, Args... args)
{
    static_assert(std::is_invocable_v<Func, Args...>, "Provided function is not invocable with the given arguments.");
    using ResultType = std::invoke_result_t<Func, Args...>;

    auto result = func(args...);

    if constexpr (std::is_integral_v<ResultType>)
    {
        if (result < 0)
        {
            int err = errno;
            std::ostringstream errorMessage;
            errorMessage << "Function '" << __func__ << "' failed with error code: "
                         << err << " (" << strerror(err) << ")";
            throw std::runtime_error(errorMessage.str());
        }
    }

    return result;
}

// Throws an exception with error message if the result is different from 'FALSE' or values converted to 'FALSE'
// Returns the result of the wrapped function execution
template<typename Func, typename... Args>
auto CheckZeroResult(const std::string& errorMessage, Func func, Args... args)
{
    static_assert(std::is_invocable_v<Func, Args...>, "Provided function is not invocable with the given arguments.");
    using ResultType = std::invoke_result_t<Func, Args...>;

    auto result = func(args...);

    if constexpr (std::is_integral_v<ResultType> || std::is_same_v<ResultType, bool>)
    {
        if (result)
        {
            std::ostringstream fullErrorMessage;
            fullErrorMessage << "Function '" << __func__ << "' failed." << std::endl
                             << "Returned non-zero value or 'true': " << std::endl
                             << errorMessage;
            throw std::runtime_error(fullErrorMessage.str());
        }
    }

    return result;
}


bool EqualsIgnoreCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
            [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

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
