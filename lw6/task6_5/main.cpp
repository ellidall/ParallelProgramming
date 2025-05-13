#include <boost/lockfree/queue.hpp>
#include <thread>
#include <iostream>

boost::lockfree::queue<int> queue(128);

void producer() {
    for (int i = 0; i < 100; ++i) {
        while (!queue.push(i)) {
            std::this_thread::yield();
        }
    }
}

void consumer() {
    int value;
    for (int i = 0; i < 100; ++i) {
        while (!queue.pop(value)) {
            std::this_thread::yield();
        }
        std::cout << value << ' ';
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();

    return 0;
}

// что за пул