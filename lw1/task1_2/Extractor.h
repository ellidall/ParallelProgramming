#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include "helpers.h"

namespace fs = std::filesystem;

class Archiver
{
public:
    Archiver(const std::string& archiveName, const std::string& outputFolder)
            : m_archiveName(archiveName), m_outputFolder(outputFolder)
    {}

    void ExtractSequential()
    {
        ExtractArchive();

        for (const auto& file : fs::directory_iterator(m_outputFolder))
        {
            if (file.path().extension() == ".gz")
            {
                DecompressFile(file.path().string());
            }
        }
    }

    void ExtractParallel(int numProcesses)
    {
        ExtractArchive();

        std::vector<pid_t> children;
        std::vector<std::string> fileNames;
        for (const auto& file : fs::directory_iterator(m_outputFolder))
        {
            fileNames.push_back(file.path().string());
        }
        for (const auto& file :fileNames)
        {
            if (file.substr(file.size() - 3) == ".gz")
            {
                std::string outputFile = file.substr(0, file.size() - 3);
                if (fs::exists(outputFile))
                {
                    continue;
                }
                while (children.size() >= static_cast<size_t>(numProcesses))
                {
                    WaitForChild(children);
                }

                pid_t pid = CheckFunctionCall(fork);
                if (pid == 0)
                {
                    DecompressFile(file);
                    if (fs::exists(file))
                    {
                        fs::remove(file);
                    }
                    return;
                }
                else if (pid > 0)
                {
                    children.push_back(pid);
                }
            }
        }

        while (!children.empty())
        {
            WaitForChild(children);
        }
    }

private:
    std::string m_archiveName;
    std::string m_outputFolder;

    void ExtractArchive()
    {
        std::string tarCommand = "tar -xf \"" + m_archiveName + "\" -C \"" + m_outputFolder + "\"";
        CheckNonZeroResult("Extracting archive failed", system, tarCommand.c_str());
    }

    static void DecompressFile(const std::string& gzFile)
    {
        std::string command = "gunzip \"" + gzFile + "\"";
        CheckNonZeroResult("Decompressing failed", system, command.c_str());
    }

    void WaitForChild(std::vector<pid_t>& children)
    {
        int status;
        pid_t finishedPid = wait(&status);
        if (finishedPid > 0)
        {
            children.erase(std::remove(children.begin(), children.end(), finishedPid), children.end());
        }
    }
};
