#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "../task6_1/TicketOffice.h"

TEST(TicketOfficeTest, SingleThreadExactSale)
{
    TicketOffice office(10);
    int sold = office.SellTickets(10);
    EXPECT_EQ(sold, 10);
    EXPECT_EQ(office.GetTicketsLeft(), 0);
}

TEST(TicketOfficeTest, SingleThreadMultipleSales)
{
    TicketOffice office(10);
    EXPECT_EQ(office.SellTickets(4), 4);
    EXPECT_EQ(office.SellTickets(3), 3);
    EXPECT_EQ(office.SellTickets(3), 3);
    EXPECT_EQ(office.GetTicketsLeft(), 0);
}

TEST(TicketOfficeTest, ThrowsOnZeroOrNegativeOrBigger)
{
    TicketOffice office(10);
    EXPECT_THROW(office.SellTickets(0), std::invalid_argument);
    EXPECT_THROW(office.SellTickets(-5), std::invalid_argument);
    EXPECT_THROW(office.SellTickets(20), std::invalid_argument);
}

TEST(TicketOfficeTest, MultiThreadedSales)
{
    const int totalTickets = 1000;
    TicketOffice office(totalTickets);

    const int threadCount = 10;
    const int ticketsPerThread = 220;

    std::atomic<int> totalSold{0};
    std::atomic<int> failedSales{0};
    std::vector<std::jthread> threads;
    threads.reserve(threadCount);
    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&]() {
            try
            {
                int sold = office.SellTickets(ticketsPerThread);
                totalSold.fetch_add(sold, std::memory_order_relaxed);
            }
            catch (const std::exception& e)
            {
                failedSales.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    int remaining = office.GetTicketsLeft();
    EXPECT_LE(remaining, totalTickets);
    EXPECT_EQ(totalSold.load() + remaining, totalTickets);
    EXPECT_EQ(failedSales.load(), 6);
}