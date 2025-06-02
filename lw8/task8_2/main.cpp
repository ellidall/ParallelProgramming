#include "BitonicSorter.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <execution>
#include <fstream>
#include "_libs/_timer.h"
#include "_libs/_logger.h"

void Print(std::ofstream& out, std::vector<int>& data){
    for (const auto& val : data) {
        out << val << std::endl;
    }
}

int main() {
    std::vector<int> dataGpu(100);
    std::generate(dataGpu.begin(), dataGpu.end(), []() { return rand(); });
    std::vector<int> dataCpu = dataGpu;

    BitonicSorter sorter;

    Timer timer;
    sorter.Sort(dataGpu);
    auto timeGpu = timer.GetElapsed();

    timer.Reset();
    std::sort(std::execution::par, dataCpu.begin(), dataCpu.end());
    auto timeCpu = timer.GetElapsed();

    std::ofstream outGpu("C:/Sasha/Studying/ParallelProgramming/lw8/task8_1/bin/outGpu.txt");
    std::ofstream outCpu("C:/Sasha/Studying/ParallelProgramming/lw8/task8_1/bin/outCpu.txt");
    Print(outGpu, dataGpu);
    Print(outCpu, dataCpu);

    Logger::Println("GPU bitonic sort time: " + std::to_string(timeGpu));
    Logger::Println("CPU parallel std::sort time: " + std::to_string(timeCpu));

    return EXIT_SUCCESS;
}
