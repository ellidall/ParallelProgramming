#pragma once

#include <iostream>
#include "LifeGame.h"

class LifeGameController
{
public:
    LifeGameController() = default;

    void RunStep(int numThreads)
    {
        Timer timer;
        m_game->Step(numThreads);
        std::cout << "Total time: " << timer.GetElapsed() << " seconds" << std::endl;
    }

    void LoadGame(const std::string& inputPath)
    {
        std::fstream input;
        FS::LoadStream(inputPath, input, std::ios::in);

        int width, height;
        std::vector<std::string> field;

        input >> width >> height;
        input.ignore();

        field.resize(height);
        for (int y = 0; y < height; y++)
        {
            std::getline(input, field[y]);
        }

        m_game = std::make_unique<LifeGame>(width, height, field);
    }

    void SaveGame(const std::string& outputPath)
    {
        if (!m_game) {
            throw std::runtime_error("game not loaded");
        }

        std::fstream output;
        FS::LoadStream(outputPath, output, std::ios::out | std::ios::trunc);
        output.clear();
        std::ostringstream oss;
        LifeGameData gameData = m_game->GetGameData();

        oss << gameData.width << " " << gameData.height << std::endl;
        for (const auto& row : gameData.field)
        {
            oss << row << std::endl;
        }

        FS::Save(output, oss);
    }

    static void Generate(const std::string& outputFile, int width, int height, double probability)
    {
        std::fstream outStream;
        FS::LoadStream(outputFile, outStream, std::ios::out | std::ios::trunc);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        std::ostringstream oss;
        oss << width << " " << height << std::endl;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                oss << (dis(gen) < probability ? '#' : ' ');
            }
            oss << std::endl;
        }
        FS::Save(outStream, oss);
    }

private:
    std::unique_ptr<LifeGame> m_game = nullptr;
};