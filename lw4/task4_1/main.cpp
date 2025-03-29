#include <iostream>
#include <vector>
#include <SFML/Audio/SoundBuffer.hpp>
#include <valarray>
#include <SFML/Audio/Sound.hpp>
#include <fstream>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"
#include "MusicPlayer.h"

const std::string FLAG_END = "END";

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println("\t./music <inputFilePath>");
}

struct ProgramArgs
{
    std::string inputFilePath;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc != 2)
    {
        PrintUsage();
        throw std::invalid_argument("invalid count of arguments");
    }

    ProgramArgs args;
    args.inputFilePath = argv[1];

    return args;
}

struct PlayerArgs {
    std::vector<std::string> tokens;
    int tempo;
};

PlayerArgs GetPlayerArgsFromFile(std::string& inputFilePath) {
    std::ifstream file(inputFilePath);
    if (!file)
    {
        throw std::invalid_argument("Ошибка: не удалось открыть файл " + inputFilePath);
    }

    std::string line;
    std::getline(file, line);
    auto tempo = std::stoi(line);

    std::vector<std::string> tokens;
    while (std::getline(file, line))
    {
        if (line == FLAG_END) break;
        if (line.empty()) {
            tokens.emplace_back("");
            continue;
        }
        std::istringstream iss(line);
        std::string token;
        if (iss >> token)
        {
            tokens.push_back(token);
        }
    }

    return {tokens, tempo};
}

int main(int argc, char* argv[])
{
    // избавиться от щелчков
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        ProgramArgs args = ParseArgs(argc, argv);

        PlayerArgs playerArgs = GetPlayerArgsFromFile(args.inputFilePath);
        MusicPlayer player(playerArgs.tokens, playerArgs.tempo);
        player.PlayMusic();
    });

    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}