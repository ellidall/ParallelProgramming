#include <iostream>
#include <vector>
#include "GaussBlur.h"
#include "_helpers.h"
#include "ImageProcessor.h"

const std::string COMMAND_GENERATE = "generate";
const std::string COMMAND_STEP = "step";
const std::string COMMAND_VISUALIZE = "visualize";

void PrintUsage()
{
    std::cerr << "Использование:" << std::endl
              << "  life generate OUTPUT_FILE WIDTH HEIGHT PROBABILITY" << std::endl
              << "  life step INPUT_FILE NUM_THREADS [OUTPUT_FILE]" << std::endl;
}

struct ProgramArgs
{
    bool isGenerate = false;
    bool isVisualize = false;
    std::string inputPath;
    std::string outputPath;
    int width = 0;
    int height = 0;
    double probability = 0.0;
    int numThreads = 1;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc < 4)
    {
        PrintUsage();
        throw std::invalid_argument("not enough arguments");
    }

    ProgramArgs args;
    std::string command = argv[1];

    if (EqualsIgnoreCase(command, COMMAND_GENERATE))
    {
        if (argc != 6)
        {
            PrintUsage();
            throw std::invalid_argument("invalid count of arguments for command: " + COMMAND_GENERATE);
        }
        args.isGenerate = true;
        args.outputPath = argv[2];
        args.width = std::stoi(argv[3]);
        args.height = std::stoi(argv[4]);
        args.probability = std::stod(argv[5]);
    }
    else if (EqualsIgnoreCase(command, COMMAND_STEP))
    {
        if (argc > 5)
        {
            PrintUsage();
            throw std::invalid_argument("invalid count of arguments for command: " + COMMAND_STEP);
        }
        args.isGenerate = false;
        args.inputPath = argv[2];
        args.numThreads = std::stoi(argv[3]);
        if (argc == 5)
        {
            args.outputPath = argv[4];
        }
    }
    else if (EqualsIgnoreCase(command, COMMAND_VISUALIZE))
    {
        if (argc > 4)
        {
            PrintUsage();
            throw std::invalid_argument("invalid count of arguments for command: " + COMMAND_VISUALIZE);
        }
        args.isVisualize = true;
        args.inputPath = argv[2];
        args.numThreads = std::stoi(argv[3]);
    }
    else
    {
        PrintUsage();
        throw std::invalid_argument("invalid command");
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        ProgramArgs args = ParseArgs(argc, argv);
        LifeGameController gameController;

        if (args.isGenerate)
        {
            LifeGameController::Generate(args.outputPath, args.width, args.height, args.probability);
        }
        else if (args.isVisualize)
        {
            gameController.LoadGame(args.inputPath);
            gameController.Visualize(args.numThreads);
        }
        else
        {
            gameController.LoadGame(args.inputPath);
            gameController.RunStep(args.numThreads);
            gameController.SaveGame(args.outputPath.empty() ? args.inputPath : args.outputPath);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
