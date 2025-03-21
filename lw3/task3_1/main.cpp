#include <iostream>
#include <vector>
#include "_libs/_helpers.h"
#include "_libs/_logger.h"
#include "_libs/_exceptionHandler.h"
#include "SimulationExecutor.h"

const std::string FLAG_PARALLEL = "-P";
const std::string FLAG_SEQUENTIAL = "-S";

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println("\tbank " + FLAG_PARALLEL + " <money in circulation>");
    Logger::Println("\tbank " + FLAG_SEQUENTIAL + " <money in circulation>");
}

struct ProgramArgs
{
    Money initialCash = 0;
    bool isParallel = false;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc != 3)
    {
        PrintUsage();
        throw std::invalid_argument("invalid count of arguments");
    }

    ProgramArgs args;
    std::string flag = argv[1];
    if (EqualsIgnoreCase(flag, FLAG_PARALLEL))
    {
        args.isParallel = true;
    }
    else if (EqualsIgnoreCase(flag, FLAG_SEQUENTIAL))
    {
        args.isParallel = false;
    }
    else
    {
        PrintUsage();
        throw std::invalid_argument("invalid command");
    }

    args.initialCash = std::stoll(argv[2]);

    return args;
}

std::atomic<bool> shouldExit(false);
void SignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "Received signal " << signal << ", terminating..." << std::endl;
        shouldExit = true;
    }
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        ProgramArgs args = ParseArgs(argc, argv);

        auto bank = std::make_shared<Bank>(args.initialCash);
        SimulationExecutor simulationExecutor(bank, 10000);

        if (args.isParallel)
        {
            Logger::Println("Starting simulation in parallel mode...");
            // TODO: simulation
        }
        else
        {
            Logger::Println("Starting simulation in sequential mode...");
            while (!shouldExit.load())
            {
                std::string command;
                std::getline(std::cin, command);

                if (command == "step") {
                    simulationExecutor.ExecuteSequentialStep();
                }
            }
        }
    });

    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}