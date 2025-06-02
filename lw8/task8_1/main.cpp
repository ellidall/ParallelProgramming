#include "BitonicSorter.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <execution>
#include "_libs/_timer.h"
#include "_libs/_logger.h"

int main() {
    std::vector<int> dataGpu(100000);
    std::generate(dataGpu.begin(), dataGpu.end(), []() { return rand(); });
    std::vector<int> dataCpu = dataGpu;

    BitonicSorter sorter;

    Timer timer;
    sorter.Sort(dataGpu);
    auto timeGpu = timer.GetElapsed();

    timer.Reset();
    std::sort(std::execution::par, dataCpu.begin(), dataCpu.end());
    auto timeCpu = timer.GetElapsed();

    if (dataCpu == dataGpu) {
        std::cout << "Sorting correct!" << std::endl;
    } else {
        std::cout << "Sorting incorrect!" << std::endl;
    }

    Logger::Println("GPU bitonic sort time: " + std::to_string(timeGpu));
    Logger::Println("CPU parallel std::sort time: " + std::to_string(timeCpu));

    return EXIT_SUCCESS;
}
