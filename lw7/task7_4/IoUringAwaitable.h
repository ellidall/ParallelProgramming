#pragma once

#include <coroutine>
#include <exception>
#include <system_error>
#include <liburing.h>
#include <stdexcept>
#include <iostream>

class Dispatcher;

// в класс с приватнымии полями
struct IoUringAwaitableBase {
    Dispatcher* m_dispatcherPtr = nullptr;
    std::coroutine_handle<> m_continuationHandle;
    int m_cqeRes = 0;
    uint32_t m_cqeFlags = 0;
    std::exception_ptr m_exceptionPtr = nullptr;

    explicit IoUringAwaitableBase(Dispatcher* d) : m_dispatcherPtr(d) {}
    IoUringAwaitableBase(const IoUringAwaitableBase&) = delete;
    IoUringAwaitableBase& operator=(const IoUringAwaitableBase&) = delete;
    virtual ~IoUringAwaitableBase() = default;

    bool await_ready() const noexcept { return false; }

    virtual void OnOperationComplete(int res, uint32_t flags) {
        m_cqeRes = res;
        m_cqeFlags = flags;
        if (res < 0) {
            m_exceptionPtr = std::make_exception_ptr(
                    std::system_error(-res, std::system_category(), "io_uring operation failed")
            );
        }
    }

    void CompleteAndResume(int res, uint32_t flags) {
        OnOperationComplete(res, flags);
        if (m_continuationHandle) {
            m_continuationHandle.resume();
        }
    }

    virtual void PrepareSqe(io_uring_sqe* sqe) = 0;
};

struct FireAndForgetAwaitable : IoUringAwaitableBase {
    using IoUringAwaitableBase::IoUringAwaitableBase;

    void OnOperationComplete(int res, uint32_t flags) override {
        IoUringAwaitableBase::OnOperationComplete(res, flags);
        if (m_exceptionPtr) {
            try {
                std::rethrow_exception(m_exceptionPtr);
            } catch (const std::exception& e) {
                 std::cerr << "Error in fire-and-forget operation: " << e.what() << std::endl;
            }
        }
    }
};
