#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "LifeGame.h"

class LifeGameVisualizer
{
public:
    explicit LifeGameVisualizer(LifeGame& game, int cellSize = 10) : m_game(game), m_cellSize(cellSize)
    {
        LifeGameData data = game.GetGameData();
        m_width = data.width;
        m_height = data.height;

        m_window.create(sf::VideoMode(m_width * m_cellSize, m_height * m_cellSize), "Game of Life");
        m_text.setCharacterSize(16);
        m_text.setFillColor(sf::Color::White);
        m_text.setPosition(10, 10);
    }

    void Run(int numThreads)
    {
        sf::Clock clock;
        float totalTime = 0;
        int steps = 0;

        while (m_window.isOpen())
        {
            sf::Event event{};
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    m_window.close();
            }

            sf::Time elapsed = clock.restart();
            m_game.Step(numThreads);
            totalTime += elapsed.asSeconds();
            steps++;

            float avgTime = totalTime / static_cast<float>(steps);
            m_window.setTitle("Game of Life - Avg Time: " + std::to_string(avgTime) + "s");

            Draw();
            sf::sleep(sf::milliseconds(500));
        }
    }

private:
    LifeGame& m_game;
    sf::RenderWindow m_window;
    int m_width, m_height;
    int m_cellSize;
    sf::Text m_text;

    void Draw()
    {
        m_window.clear(sf::Color::White);

        LifeGameData data = m_game.GetGameData();
        sf::RectangleShape cell(sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize)));

        for (int y = 0; y < m_height; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                if (data.field[y][x] == FILLED)
                {
                    cell.setFillColor(sf::Color::Black);
                }
                else
                {
                    cell.setFillColor(sf::Color::White);
                }
                cell.setPosition(static_cast<float>(x * m_cellSize), static_cast<float>(y * m_cellSize));
                m_window.draw(cell);
            }
        }

        m_window.display();
    }
};

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
        if (!m_game)
        {
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
                oss << (dis(gen) < probability ? FILLED : EMPTY);
            }
            oss << std::endl;
        }
        FS::Save(outStream, oss);
    }

    void Visualize(int numThreads)
    {
        if (!m_game)
        {
            throw std::runtime_error("game not loaded");
        }

        LifeGameVisualizer visualizer(*m_game);
        visualizer.Run(numThreads);
    }

private:
    std::unique_ptr<LifeGame> m_game = nullptr;
};

