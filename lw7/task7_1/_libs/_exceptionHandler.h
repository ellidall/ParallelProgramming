#pragma once

#include <iostream>
#include <thread>
#include <exception>

class ExceptionHandler {
private:
    bool m_isExceptionCaught = false;
    std::string m_errorMessage;

public:
    template<typename Func, typename... Args>
    void Handle(Func&& func, Args&&... args) {
        try {
            std::forward<Func>(func)(std::forward<Args>(args)...);
        } catch (const std::exception& e) {
            m_isExceptionCaught = true;
            m_errorMessage = e.what();
        }
    }

    [[nodiscard]] bool WasExceptionCaught() const {
        return m_isExceptionCaught;
    }

    [[nodiscard]] const std::string& GetErrorMessage() const {
        return m_errorMessage;
    }
};
