#include <iostream>
#include "helpers.h"
#include "Extractor.h"

const std::string FLAG_SEQUENTIAL = "-S";
const std::string FLAG_PARALLEL = "-P";

void PrintUsage()
{
    std::cerr << "Использование:" << std::endl
              << "  make-archive -S ARCHIVE [FILES]   - последовательный режим" << std::endl
              << "  make-archive -P N ARCHIVE [FILES] - параллельный режим с N процессами" << std::endl;
}

struct ProgramArgs
{
    bool isParallel = false;
    int numProcesses = 1;
    std::string archiveName;
    std::string outputFolder;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc < 4)
    {
        PrintUsage();
        throw std::invalid_argument("invalid num of arguments");
    }

    ProgramArgs args;
    std::string mode = argv[1];
    if (EqualsIgnoreCase(mode, FLAG_SEQUENTIAL))
    {
        args.isParallel = false;
        args.archiveName = argv[2];
        args.outputFolder = argv[3];
    }
    else if (EqualsIgnoreCase(mode, FLAG_PARALLEL))
    {
        if (argc < 5)
        {
            PrintUsage();
            throw std::invalid_argument("invalid num of arguments for parallel archiving");
        }
        try
        {
            args.numProcesses = std::stoi(argv[2]);
            if (args.numProcesses <= 0)
            {
                throw std::invalid_argument("the number of processes must be greater than '0'");
            }
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("incorrect number of processes:" + static_cast<std::string>(argv[2]));
        }

        args.isParallel = true;
        args.archiveName = argv[3];
        args.outputFolder = argv[4];
    }
    else
    {
        PrintUsage();
        throw std::invalid_argument("invalid operation flag");
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        auto args = ParseArgs(argc, argv);
        Archiver archiver(args.archiveName, args.outputFolder);

        Timer timer;
        if (args.isParallel)
        {
            archiver.ExtractParallel(args.numProcesses);
        }
        else
        {
            archiver.ExtractSequential();
        }
        std::cout << "Total time: " << timer.GetElapsed() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}