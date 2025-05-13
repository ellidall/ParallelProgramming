#pragma once

#include <atomic>
#include <stdexcept>

class TicketOffice
{
public:
    explicit TicketOffice(int numTickets) : m_numTickets(numTickets)
    {}

    TicketOffice(const TicketOffice&) = delete;
    TicketOffice& operator=(const TicketOffice&) = delete;

    int SellTickets(int ticketsToBuy)
    {
        if (ticketsToBuy <= 0)
        {
            throw std::invalid_argument("Number of tickets to buy must be positive");
        }

        auto current = GetTicketsLeft();
        do
        {
            // поправить по заданию
            if (current < ticketsToBuy)
            {
                throw std::invalid_argument("Number of tickets to buy more than we have");
            }
        }
        while (!m_numTickets.compare_exchange_weak(current, current - ticketsToBuy,
                std::memory_order_acquire,
                std::memory_order_relaxed));
        // ыставить правильные моделти памяти

        return ticketsToBuy;
    }

    [[nodiscard]] int GetTicketsLeft() const noexcept
    {
        return m_numTickets.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> m_numTickets;
};
