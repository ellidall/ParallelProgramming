#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "../task6_2/AtomicMax.h"

TEST(AtomicMaxTest, InitialValue)
{
    AtomicMax<int> max(42);
    EXPECT_EQ(max.GetValue(), 42);
}

TEST(AtomicMaxTest, UpdateWithGreaterValue)
{
    AtomicMax<int> max(10);
    max.Update(20);
    EXPECT_EQ(max.GetValue(), 20);
}

TEST(AtomicMaxTest, UpdateWithSmallerValue)
{
    AtomicMax<int> max(50);
    max.Update(30);
    EXPECT_EQ(max.GetValue(), 50);
}

TEST(AtomicMaxTest, UpdateWithEqualValue)
{
    AtomicMax<int> max(100);
    max.Update(100);
    EXPECT_EQ(max.GetValue(), 100);
}

TEST(AtomicMaxTest, MultipleSequentialUpdates)
{
    AtomicMax<int> max(0);
    max.Update(5);
    max.Update(2);
    max.Update(7);
    max.Update(3);
    EXPECT_EQ(max.GetValue(), 7);
}

// тоб стартовали одновременно

TEST(AtomicMaxTest, MultiThreadedUpdates)
{
    AtomicMax<int> max(0);
    const int threadCount = 8;
    std::vector<std::thread> threads;

    threads.reserve(threadCount);
    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([i, &max]() {
            for (int j = 0; j < 100; ++j)
            {
                max.Update(i * 100 + j);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(max.GetValue(), (threadCount - 1) * 100 + 99);
}
