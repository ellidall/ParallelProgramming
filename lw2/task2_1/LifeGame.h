#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <thread>
#include "_helpers.h"
#include "_fs.h"
#include "_timer.h"

const char FILLED = '#';
const char EMPTY = '_';

struct LifeGameData
{
    int width = 0;
    int height = 0;
    std::vector<std::string> field;
};

class LifeGame
{
public:
    explicit LifeGame(int width, int height, std::vector<std::string>& field)
            : m_width(width), m_height(height), m_field(field)
    {
        m_newField = m_field;
    }

    [[nodiscard]] LifeGameData GetGameData() const
    {
        return LifeGameData{m_width, m_height, m_field};
    }

    void Step(int numThreads)
    {
        std::vector<std::jthread> threads;
        int chunkSize = m_height / numThreads;

        for (int i = 0; i < numThreads; i++)
        {
            int startY = i * chunkSize;
            int endY = (i == numThreads - 1) ? m_height : (i + 1) * chunkSize;
            threads.emplace_back(&LifeGame::UpdateSection, this, startY, endY);
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        m_field.swap(m_newField);
    }

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::string> m_field;
    std::vector<std::string> m_newField;

    int CountNeighbors(int x, int y)
    {
        static const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
        static const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};

        int count = 0;
        for (int i = 0; i < 8; i++)
        {
            int nx = (x + dx[i] + m_width) % m_width;
            int ny = (y + dy[i] + m_height) % m_height;
            if (m_field[ny][nx] == FILLED)
            {
                count++;
            }
        }
        return count;
    }

    void UpdateSection(int startY, int endY)
    {
        for (int y = startY; y < endY; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                int neighbors = CountNeighbors(x, y);
                m_newField[y][x] = (m_field[y][x] == FILLED
                                    ? (neighbors == 2 || neighbors == 3 ? FILLED : EMPTY)
                                    : (neighbors == 3 ? FILLED : EMPTY));
            }
        }
    }
};
