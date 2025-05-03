#include <stdexcept>

#include "Server.h"
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"
#include "Television.h"

struct Args
{
    Mode mode;
    std::string address;
    unsigned short port;
};

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println("\t/tv PORT");
    Logger::Println("\t/tv ADDRESS PORT");
}

Args ParseArgs(const int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        PrintUsage();
        throw std::invalid_argument("incorrect number of arguments");
    }
    else if (argc == 2)
    {
        const auto port = static_cast<unsigned short>(std::stoi(argv[1]));
        return Args{Mode::Station, "", port};
    }
    else if (argc == 3)
    {
        const std::string address = argv[1];
        const auto port = static_cast<unsigned short>(std::stoi(argv[2]));
        return {Mode::Receiver, address, port};
    }

    throw std::invalid_argument("incorrect number of arguments");
}

int main(const int argc, char* argv[])
{
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&] {
        unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
        Server server(port);
        server.Run();
        // auto [mode, address, port] = ParseArgs(argc, argv);
        // Television tv(mode, address, port);
        // tv.Run();
    });
    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
