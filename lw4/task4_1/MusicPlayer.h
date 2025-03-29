#pragma once

#include <iostream>
#include <vector>
#include <SFML/Audio.hpp>
#include <fstream>
#include <cmath>
#include <sstream>
#include <thread>
#include <chrono>
#include <cmath>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"

class MusicPlayer
{
public:
    const int SAMPLE_RATE = 44100;
    const double PI = 3.141592653589793;
    const double BASE_AMPLITUDE = 30000.0;
    const double MS_IN_SECOND = 60000;

    explicit MusicPlayer(std::vector<std::string>& tokens, int tempo) : m_tokens(tokens), m_tempo(tempo)
    {
        m_durationMs = static_cast<int>(MS_IN_SECOND / m_tempo);
    }

    void PlayMusic()
    {
        for (const auto& token : m_tokens)
        {
            if (token.empty()) continue;

            sf::SoundBuffer buffer = GenerateSound(token);
            m_buffers.push_back(buffer);

            sf::Sound sound;
            sound.setBuffer(m_buffers.back());
            m_sounds.push_back(sound);

            sound.play();
            std::this_thread::sleep_for(std::chrono::milliseconds(m_durationMs));
        }

//        for (auto& sound : m_sounds)
//        {
//            sound.play();
//            std::this_thread::sleep_for(std::chrono::milliseconds(m_durationMs));
//        }
    }

private:
    std::vector<std::string> m_tokens;
    int m_tempo{0};
    int m_durationMs{0};
    std::vector<sf::Sound> m_sounds;
    std::vector<sf::SoundBuffer> m_buffers;

    sf::SoundBuffer GenerateSound(const std::string& token)
    {
        int sampleCount = SAMPLE_RATE * m_durationMs / 1000;
        std::vector<sf::Int16> samples(sampleCount, 0);

        bool hasRelease = false;
        std::string noteStr = token;
        if (noteStr.back() == '-')
        {
            hasRelease = true;
            noteStr.pop_back();
        }
        double freq = GetFrequency(noteStr);

        for (int i = 0; i < sampleCount; ++i)
        {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            double envelope = hasRelease ? (1.0 - (static_cast<double>(i) / sampleCount)) : 1.0;
            double sampleValue = BASE_AMPLITUDE * envelope * std::sin(2 * PI * freq * t);
            samples[i] += static_cast<sf::Int16>(sampleValue);
        }

        sf::SoundBuffer buffer;
        if (!buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE))
        {
            throw std::runtime_error("не удалось загрузить сэмплы в буфер");
        }
        return buffer;
    }

    static double GetFrequency(const std::string& note)
    {
        if (note.empty()) return 0.0;
        char letter = note[0];
        bool isSharp = false;
        int octave = 0;

        if (note.size() == 2)
        {
            isSharp = false;
            octave = note[1] - '0';
        }
        else if (note.size() >= 3)
        {
            if (note[1] == '#')
            {
                isSharp = true;
                octave = note[2] - '0';
            }
            else
            {
                isSharp = false;
                octave = note[1] - '0';
            }
        }

        int semitoneOffset = 0;
        switch (letter)
        {
            case 'C':
                semitoneOffset = -9;
                break;
            case 'D':
                semitoneOffset = -7;
                break;
            case 'E':
                semitoneOffset = -5;
                break;
            case 'F':
                semitoneOffset = -4;
                break;
            case 'G':
                semitoneOffset = -2;
                break;
            case 'A':
                semitoneOffset = 0;
                break;
            case 'B':
                semitoneOffset = 2;
                break;
            default:
                semitoneOffset = 0;
                break;
        }
        if (isSharp)
            semitoneOffset += 1;

        int totalSemitone = (octave - 4) * 12 + semitoneOffset;
        double frequency = 440.0 * std::pow(2.0, totalSemitone / 12.0);
        return frequency;
    }
};
