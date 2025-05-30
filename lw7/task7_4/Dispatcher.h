#pragma once

#include "IoUringAwaitable.h"
#include <liburing.h>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <system_error>
#include <iostream>
#include <cstring>

class Dispatcher
{
public:
    explicit Dispatcher(unsigned queueDepth = 64);
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void SubmitAwaitable(IoUringAwaitableBase* awaitableData);
    void Stop();

private:
    void RunLoop();

    io_uring m_ring{};

    std::jthread m_pollerThread;
    std::atomic<bool> m_stopFlag{false};
    unsigned m_queueDepth;
};

Dispatcher::Dispatcher(unsigned queueDepth) : m_queueDepth(queueDepth)
{
    int ret = io_uring_queue_init(m_queueDepth, &m_ring, 0);
    if (ret < 0)
    {
        throw std::system_error(-ret, std::system_category(), "io_uring_queue_init failed");
    }
    m_pollerThread = std::jthread(&Dispatcher::RunLoop, this);
}

Dispatcher::~Dispatcher()
{
    Stop();
    if (m_pollerThread.joinable())
    {
        m_pollerThread.join();
    }
    io_uring_queue_exit(&m_ring);
}

void Dispatcher::SubmitAwaitable(IoUringAwaitableBase* awaitableData)
{
    io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
    if (!sqe)
    {
        int submitted_now = io_uring_submit(&m_ring);
        if (submitted_now < 0 && submitted_now != -EBUSY)
        {
            throw std::system_error(-submitted_now, std::system_category(),
                    "io_uring_submit failed while trying to make space in SQE queue");
        }
        sqe = io_uring_get_sqe(&m_ring);
        if (!sqe)
        {
            throw std::runtime_error("io_uring submission queue is still full after an attempted submit");
        }
    }
    awaitableData->PrepareSqe(sqe);
    io_uring_sqe_set_data(sqe, awaitableData);

    int ret = io_uring_submit(&m_ring);
    if (ret < 0)
    {
        if (awaitableData)
        {
            io_uring_sqe_set_data(sqe, nullptr);
        }
        throw std::system_error(-ret, std::system_category(), "io_uring_submit failed for new SQE");
    }
}

void Dispatcher::Stop()
{
    if (!m_stopFlag.exchange(true))
    {
        io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
        if (sqe)
        {
            io_uring_prep_nop(sqe);
            io_uring_sqe_set_data(sqe, nullptr);
            io_uring_submit(&m_ring);
        }
    }
}

void Dispatcher::RunLoop()
{
    while (true)
    {
        io_uring_cqe* cqe = nullptr;

        if (m_stopFlag.load(std::memory_order_relaxed))
        {
            int ret_peek = io_uring_peek_cqe(&m_ring, &cqe);
            if (ret_peek == -EAGAIN || !cqe)
            {
                break;
            }
        }
        else
        {
            int ret_wait = io_uring_wait_cqe(&m_ring, &cqe);
            if (ret_wait < 0)
            {
                if (ret_wait == -EINTR)
                {
                    if (m_stopFlag.load(std::memory_order_relaxed)) break;
                    continue;
                }
                if (m_stopFlag.load(std::memory_order_relaxed)) break;

                std::cerr << "Dispatcher: io_uring_wait_cqe error: " << strerror(-ret_wait) << std::endl;
                if (!m_stopFlag.exchange(true))
                break;
            }
        }


        if (!cqe)
        {
            if (m_stopFlag.load(std::memory_order_relaxed)) break;
            continue;
        }

        auto* awaitableData = static_cast<IoUringAwaitableBase*>(io_uring_cqe_get_data(cqe));

        bool deleteAfterResume = false;
        if (awaitableData)
        {
            if (dynamic_cast<FireAndForgetAwaitable*>(awaitableData))
            {
                deleteAfterResume = true;
            }
            awaitableData->CompleteAndResume(cqe->res, cqe->flags);
            if (deleteAfterResume)
            {
                delete awaitableData;
            }
        }

        io_uring_cqe_seen(&m_ring, cqe);
    }
}
