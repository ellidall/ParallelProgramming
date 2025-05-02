#include <iostream>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"
#include "TV.h"

const std::string EMPTY_STRING = "";

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println("\t./tv <port>");
    Logger::Println("\t./tv <address> <port>");
}

struct ProgramArgs
{
    std::string address;
    unsigned short port{};
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    ProgramArgs args;

    if (argc == 2)
    {
        args.address = EMPTY_STRING;
        args.port = std::stoi(argv[1]);
    }
    else if (argc == 3)
    {
        args.address = argv[1];
        args.port = std::stoi(argv[1]);
    }
    else
    {
        PrintUsage();
        throw std::invalid_argument("invalid count of arguments");
    }

    return args;
}

int main(int argc, char* argv[])
{
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        ProgramArgs args = ParseArgs(argc, argv);

        if (args.address == EMPTY_STRING) {
            start_tv_station(args.port);

        } else {
            start_tv_receiver(args.address, args.port);
        }
    });

    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}