#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

class FS
{
public:
    static void LoadStream(const std::string& path, std::fstream& stream, std::ios_base::openmode mode)
    {
        if (!fs::exists(path) && (mode & std::ios::in))
        {
            throw std::runtime_error("file does not exist: " + path);
        }

        stream.open(path, mode);
        if (!stream.is_open())
        {
            throw std::runtime_error("failed to open file: " + path);
        }
    }

    static void Save(std::fstream& outputStream, std::ostringstream& dataStream)
    {
        if (!outputStream.is_open())
        {
            throw std::runtime_error("failed to open the file for writing");
        }
        if (outputStream.fail() || dataStream.fail())
        {
            throw std::runtime_error("stream is in incorrect state --> can't write");
        }

        outputStream << dataStream.str();
        if (outputStream.fail())
        {
            throw std::runtime_error("error occurred while writing to the file");
        }
    }

    static void Save(std::fstream& outputStream, const std::string& data)
    {
        if (!outputStream.is_open())
        {
            throw std::runtime_error("failed to open the file for writing");
        }
        if (outputStream.fail())
        {
            throw std::runtime_error("stream is in incorrect state --> can't write");
        }

        outputStream << data;
        if (outputStream.fail())
        {
            throw std::runtime_error("error occurred while writing to the file");
        }
    }
};