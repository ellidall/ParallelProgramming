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