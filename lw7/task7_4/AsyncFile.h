#pragma once

#include "Task.h"
#include "IoUringAwaitable.h"
#include "Dispatcher.h"
#include <string>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

enum class OpenMode
{
    Read,
    Write
};

class AsyncFile;

struct AsyncFileCloseAwaitable : FireAndForgetAwaitable
{
    int m_fdToClose;

    AsyncFileCloseAwaitable(Dispatcher* d, int fd) : FireAndForgetAwaitable(d), m_fdToClose(fd) {}

    void PrepareSqe(io_uring_sqe* sqe) override
    {
        io_uring_prep_close(sqe, m_fdToClose);
    }
};


class AsyncFile
{
public:
    AsyncFile() : m_fd(-1), m_dispatcherPtr(nullptr) {}

    AsyncFile(int fd, Dispatcher& dispatcher) : m_fd(fd), m_dispatcherPtr(&dispatcher) {}

    ~AsyncFile()
    {
        SubmitAsyncClose();
    }

    AsyncFile(AsyncFile&& other) noexcept
            : m_fd(std::exchange(other.m_fd, -1)),
              m_dispatcherPtr(std::exchange(other.m_dispatcherPtr, nullptr)) {}

     AsyncFile& operator=(AsyncFile&& other) noexcept
    {
        if (this != &other)
        {
            SubmitAsyncClose();
            m_fd = std::exchange(other.m_fd, -1);
            m_dispatcherPtr = std::exchange(other.m_dispatcherPtr, nullptr);
        }
        return *this;
    }

    AsyncFile(const AsyncFile&) = delete;
    AsyncFile& operator=(const AsyncFile&) = delete;

    explicit operator bool() const { return m_fd != -1; }

    struct ReadAwaitable : IoUringAwaitableBase
    {
        int m_fdToRead;
        char* m_buffer;
        unsigned m_size;

        ReadAwaitable(Dispatcher& d, int fd, char* buffer, unsigned size)
                : IoUringAwaitableBase(&d), m_fdToRead(fd), m_buffer(buffer), m_size(size) {}

        void PrepareSqe(io_uring_sqe* sqe) override
        {
            io_uring_prep_read(sqe, m_fdToRead, m_buffer, m_size, -1);
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            m_continuationHandle = h;
            if (!m_dispatcherPtr) throw std::runtime_error("ReadAwaitable: dispatcher not set (internal error).");
            m_dispatcherPtr->SubmitAwaitable(this);
        }

        unsigned await_resume()
        {
            if (m_exceptionPtr) std::rethrow_exception(m_exceptionPtr);
            if (m_cqeRes < 0)
            {
                throw std::system_error(-m_cqeRes, std::system_category(),
                        "AsyncFile ReadAsync failed (in await_resume)");
            }
            return static_cast<unsigned>(m_cqeRes);
        }
    };

     ReadAwaitable ReadAsync(char* buffer, unsigned size)
    {
        if (!m_dispatcherPtr) throw std::runtime_error("AsyncFile::ReadAsync: dispatcher not set.");
        if (m_fd == -1) throw std::runtime_error("AsyncFile::ReadAsync: file not open or already closed.");
        return ReadAwaitable(*m_dispatcherPtr, m_fd, buffer, size);
    }

    struct WriteAwaitable : IoUringAwaitableBase
    {
        int m_fdToWrite;
        const char* m_buffer;
        unsigned m_size;

        WriteAwaitable(Dispatcher& d, int fd, const char* buffer, unsigned size)
                : IoUringAwaitableBase(&d), m_fdToWrite(fd), m_buffer(buffer), m_size(size) {}

        void PrepareSqe(io_uring_sqe* sqe) override
        {
            io_uring_prep_write(sqe, m_fdToWrite, m_buffer, m_size, -1);
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            m_continuationHandle = h;
            if (!m_dispatcherPtr) throw std::runtime_error("WriteAwaitable: dispatcher not set (internal error).");
            m_dispatcherPtr->SubmitAwaitable(this);
        }

        unsigned await_resume()
        {
            if (m_exceptionPtr) std::rethrow_exception(m_exceptionPtr);
            if (m_cqeRes < 0)
            {
                throw std::system_error(-m_cqeRes, std::system_category(),
                        "AsyncFile WriteAsync failed (in await_resume)");
            }
            return static_cast<unsigned>(m_cqeRes);
        }
    };

    WriteAwaitable WriteAsync(const char* buffer, unsigned size)
    {
        if (!m_dispatcherPtr) throw std::runtime_error("AsyncFile::WriteAsync: dispatcher not set.");
        if (m_fd == -1) throw std::runtime_error("AsyncFile::WriteAsync: file not open or already closed.");
        return WriteAwaitable(*m_dispatcherPtr, m_fd, buffer, size);
    }

private:
    void SubmitAsyncClose()
    {
        if (m_fd != -1 && m_dispatcherPtr)
        {
            auto* closeAwaitable = new AsyncFileCloseAwaitable(m_dispatcherPtr, m_fd);
            try
            {
                m_dispatcherPtr->SubmitAwaitable(closeAwaitable);
                m_fd = -1;
            }
            catch (const std::exception& e)
            {
                std::cerr << "AsyncFile: Не удалось отправить асинхронное закрытие для fd " << m_fd
                          << " в SubmitAsyncClose: " << e.what()
                          << ". Попытка синхронного закрытия." << std::endl;
                delete closeAwaitable;
                if (m_fd != -1 && ::close(m_fd) == -1)
                {
                    perror(("AsyncFile: ошибка синхронного close для fd " + std::to_string(m_fd)).c_str());
                }
                m_fd = -1;
            }
        }
        else if (m_fd != -1)
        {
            if (::close(m_fd) == -1)
            {
                perror(("AsyncFile: ошибка синхронного close (нет диспетчера) для fd " + std::to_string(m_fd)).c_str());
            }
            m_fd = -1;
        }
    }

    int m_fd = -1;
    Dispatcher* m_dispatcherPtr = nullptr;
};


struct OpenFileAwaitable : IoUringAwaitableBase
{
    std::string m_path;
    OpenMode m_mode;

    OpenFileAwaitable(Dispatcher& d, std::string path, OpenMode mode)
            : IoUringAwaitableBase(&d), m_path(std::move(path)), m_mode(mode) {}

    void PrepareSqe(io_uring_sqe* sqe) override
    {
        int flags = 0;
        mode_t createMode = 0;

        if (m_mode == OpenMode::Read)
        {
            flags = O_RDONLY;
        }
        else if (m_mode == OpenMode::Write)
        {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            createMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        }
        else
        {
            throw std::runtime_error("OpenFileAwaitable: Unsupported OpenMode");
        }
        io_uring_prep_openat(sqe, AT_FDCWD, m_path.c_str(), flags, createMode);
    }

    void await_suspend(std::coroutine_handle<> h)
    {
        m_continuationHandle = h;
        if (!m_dispatcherPtr) throw std::runtime_error("OpenFileAwaitable: dispatcher not set (internal error).");
        m_dispatcherPtr->SubmitAwaitable(this);
    }

    AsyncFile await_resume()
    {
        if (m_exceptionPtr)
        {
            std::rethrow_exception(m_exceptionPtr);
        }
        if (m_cqeRes < 0)
        {
            throw std::system_error(-m_cqeRes, std::system_category(),
                    "AsyncOpenFile failed (in await_resume) for path: " + m_path);
        }
        return AsyncFile(m_cqeRes, *m_dispatcherPtr);
    }
};

OpenFileAwaitable AsyncOpenFile(Dispatcher& dispatcher, std::string path, OpenMode mode)
{
    return OpenFileAwaitable(dispatcher, std::move(path), mode);
}
