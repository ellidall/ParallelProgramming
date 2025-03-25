#include <iostream>
#include <vector>
#include "_libs/_helpers.h"
#include "_libs/_exceptionHandler.h"
#include "SimulationController.h"

const std::string FLAG_PARALLEL = "-P";
const std::string FLAG_SEQUENTIAL = "-S";
const std::string FLAG_TEST_MOD = "--test";
const std::string FLAG_LOGGING_ON = "-L";
const std::string FLAG_DURATION_ON = "-D";
const Money BANK_BALANCE = 100000;

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println(
            "\tbank " + FLAG_PARALLEL + " <money in circulation> <simulation duration> <optional: -L> <optional: -D>");
    Logger::Println("\tbank " + FLAG_SEQUENTIAL +
                    " <money in circulation> <simulation duration> <optional: -L> <optional: -D>");
    Logger::Println("\tbank " + FLAG_SEQUENTIAL + " <money in circulation> --test <optional: -L> <optional: -D>");
}

struct ProgramArgs
{
    bool isParallel = false;
    bool isTestMod = false;
    bool isLoggingOn = false;
    bool isDurationOn = false;
    Money initialCash = 0;
    double durationLimit = 0;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc < 4 || argc > 6)
    {
        PrintUsage();
        throw std::invalid_argument("invalid count of arguments");
    }

    ProgramArgs args;
    std::string flag = argv[1];
    if (EqualsIgnoreCase(flag, FLAG_PARALLEL)) args.isParallel = true;
    else if (EqualsIgnoreCase(flag, FLAG_SEQUENTIAL)) args.isParallel = false;
    else
    {
        PrintUsage();
        throw std::invalid_argument("invalid command <" + flag + ">");
    }

    args.initialCash = std::stoll(argv[2]);

    if (EqualsIgnoreCase(argv[3], FLAG_TEST_MOD)) args.isTestMod = true;
    else args.durationLimit = std::stod(argv[3]);

    if (argc >= 5 && EqualsIgnoreCase(argv[4], FLAG_LOGGING_ON)) args.isLoggingOn = true;
    if (argc == 6 && EqualsIgnoreCase(argv[5], FLAG_DURATION_ON)) args.isDurationOn = true;

    return args;
}

std::atomic<SimulationController*> activeController = nullptr;

void GlobalSignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        if (auto controller = activeController.load())
        {
            controller->StopSimulation();
        }
    }
}

int main(int argc, char* argv[])
{
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        ProgramArgs args = ParseArgs(argc, argv);

        auto bank = std::make_shared<Bank>(BANK_BALANCE);
        auto sim = std::make_shared<Simulation>(bank, args.initialCash,
                args.durationLimit, args.isLoggingOn, args.isDurationOn);
        SimulationController simulationController(sim, args.isTestMod, args.isParallel);

        if (!args.isTestMod)
        {
            activeController.store(&simulationController);
            std::signal(SIGINT, GlobalSignalHandler);
            std::signal(SIGTERM, GlobalSignalHandler);
        }

        simulationController.RunSimulation();
        activeController.store(nullptr);
    });

    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}