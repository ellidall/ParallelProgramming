#include <iostream>
#include <string>
#include "GaussBlur.h"
#include "ImageProcessor.h"

const std::string COMMAND_APPLY = "apply";
const std::string COMMAND_VISUALIZE = "visualize";

void PrintUsage()
{
    std::cerr << "Usage:" << std::endl
              << "  gauss apply INPUT_FILE OUTPUT_FILE RADIUS" << std::endl
              << "  gauss visualize INPUT_FILE" << std::endl;
}

struct ProgramArgs
{
    bool isVisualize = false;
    std::string inputPath;
    std::string outputPath;
    int radius = 0;
    int numTreads = 1;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc < 3 || argc > 6)
    {
        PrintUsage();
        throw std::invalid_argument("not enough arguments provided");
    }

    ProgramArgs args;
    std::string command = argv[1];
    if (command == COMMAND_APPLY)
    {
        if (argc != 6)
        {
            PrintUsage();
            throw std::invalid_argument("invalid number of arguments for command: " + COMMAND_APPLY);
        }
        args.inputPath = argv[2];
        args.outputPath = argv[3];
        args.radius = std::stoi(argv[4]);
        args.numTreads = std::stoi(argv[5]);
    }
    else if (command == COMMAND_VISUALIZE)
    {
        if (argc != 3)
        {
            PrintUsage();
            throw std::invalid_argument("invalid number of arguments for command: " + COMMAND_VISUALIZE);
        }
        args.isVisualize = true;
        args.inputPath = argv[2];
    }
    else
    {
        PrintUsage();
        throw std::invalid_argument("unknown command: " + command);
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        ProgramArgs args = ParseArgs(argc, argv);

        if (args.isVisualize)
        {
            GaussBlur::ShowInteractiveBlur(args.inputPath);
        }
        else
        {
            cv::Mat image = ImageProcessor::LoadImage(args.inputPath);
            GaussBlur blur(args.radius, args.numTreads);
            blur.Apply(image);
            ImageProcessor::SaveImage(args.outputPath, image);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

