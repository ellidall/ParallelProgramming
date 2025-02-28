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
    Archiver(const std::string& archiveName, const std::vector<std::string>& inputFiles)
            : m_archiveName(archiveName), m_inputFiles(inputFiles)
    {}

    void CompressSequential()
    {
        for (const auto& file : m_inputFiles)
        {
            CompressFile(file);
        }
        CreateArchive();
        RemoveExtraFiles();
    }

    void CompressParallel(int numProcesses)
    {
        std::vector<pid_t> children;

        for (const auto& file : m_inputFiles)
        {
            while (children.size() >= static_cast<size_t>(numProcesses))
            {
                WaitForChild(children);
            }

            pid_t pid = CheckFunctionCall(fork);
            if (pid == 0)
            {
                CompressFile(file);
                return;
            }
            else if (pid > 0)
            {
                children.push_back(pid);
            }
        }

        while (!children.empty())
        {
            WaitForChild(children);
        }

        CreateArchive();
        RemoveExtraFiles();
    }

private:
    std::string m_archiveName;
    std::vector<std::string> m_inputFiles;

    static void CompressFile(const std::string& file)
    {
        std::string command = "gzip -k \"" + file + "\"";
        CheckNonZeroResult("compressing failed", system, command.c_str()) == 0;
    }

    void CreateArchive()
    {
        std::string tarCommand = "tar -cf \"" + m_archiveName + (!m_archiveName.ends_with(".tar" ) ? ".tar" : "") + "\"";
        for (const auto& file : m_inputFiles)
        {
            tarCommand += " \"" + file + ".gz\"";
        }

        CheckNonZeroResult("archiving failed", system, tarCommand.c_str());
    }

    void RemoveExtraFiles()
    {
        for (auto file : m_inputFiles)
        {
            file += ".gz";
            if (fs::exists(file))
            {
                fs::remove(file);
            }
        }
    }

    void WaitForChild(std::vector<pid_t>& children)
    {
        int status;
        pid_t finishedPid = wait(&status);
        if (finishedPid > 0)
        {
            //std::erase
            children.erase(std::remove(children.begin(), children.end(), finishedPid), children.end());
        }
    }
};
