#include <iostream>
#include <vector>
#include <SFML/Audio.hpp>
#include <fstream>
#include <cmath>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"

constexpr int SAMPLE_RATE = 44100;
constexpr double PI = 3.141592653589793;

std::map<std::string, double> noteFrequencies = {
        {"C0", 16.35}, {"C#0", 17.32}, {"D0", 18.35}, {"D#0", 19.45}, {"E0", 20.60},
        {"F0", 21.83}, {"F#0", 23.12}, {"G0", 24.50}, {"G#0", 25.96}, {"A0", 27.50},
        {"A#0", 29.14}, {"B0", 30.87}, {"C1", 32.70}, {"C#1", 34.65}, {"D1", 36.71},
        {"D#1", 38.89}, {"E1", 41.20}, {"F1", 43.65}, {"F#1", 46.25}, {"G1", 49.00},
        {"G#1", 51.91}, {"A1", 55.00}, {"A#1", 58.27}, {"B1", 61.74}, {"C2", 65.41},
        {"C#2", 69.30}, {"D2", 73.42}, {"D#2", 77.78}, {"E2", 82.41}, {"F2", 87.31},
        {"F#2", 92.50}, {"G2", 98.00}, {"G#2", 103.83}, {"A2", 110.00}, {"A#2", 116.54},
        {"B2", 123.47}, {"C3", 130.81}, {"C#3", 138.59}, {"D3", 146.83}, {"D#3", 155.56},
        {"E3", 164.81}, {"F3", 174.61}, {"F#3", 185.00}, {"G3", 196.00}, {"G#3", 207.65},
        {"A3", 220.00}, {"A#3", 233.08}, {"B3", 246.94}, {"C4", 261.63}, {"C#4", 277.18},
        {"D4", 293.66}, {"D#4", 311.13}, {"E4", 329.63}, {"F4", 349.23}, {"F#4", 369.99},
        {"G4", 392.00}, {"G#4", 415.30}, {"A4", 440.00}, {"A#4", 466.16}, {"B4", 493.88},
        {"C5", 523.25}, {"C#5", 554.37}, {"D5", 587.33}, {"D#5", 622.25}, {"E5", 659.25},
        {"F5", 698.46}, {"F#5", 739.99}, {"G5", 783.99}, {"G#5", 830.61}, {"A5", 880.00},
        {"A#5", 932.33}, {"B5", 987.77}, {"C6", 1046.50}, {"C#6", 1108.73}, {"D6", 1174.66},
        {"D#6", 1244.51}, {"E6", 1318.51}, {"F6", 1396.91}, {"F#6", 1479.98}, {"G6", 1567.98},
        {"G#6", 1661.22}, {"A6", 1760.00}, {"A#6", 1864.66}, {"B6", 1975.53}, {"C7", 2093.00},
        {"C#7", 2217.46}, {"D7", 2349.32}, {"D#7", 2489.02}, {"E7", 2637.02}, {"F7", 2793.83},
        {"F#7", 2959.96}, {"G7", 3135.96}, {"G#7", 3322.44}, {"A7", 3520.00}, {"A#7", 3729.31},
        {"B7", 3951.07}, {"C8", 4186.01}
};

// Функция генерации синусоидальной волны для одной ноты
sf::SoundBuffer generateSound(const std::vector<std::string>& notes, int duration_ms) {
    std::vector<sf::Int16> samples(SAMPLE_RATE * duration_ms / 1000);
    double amplitude = 30000.0;

    for (const auto& note : notes) {
        if (noteFrequencies.count(note) == 0) continue;

        double frequency = noteFrequencies[note];
        for (size_t i = 0; i < samples.size(); ++i) {
            samples[i] += static_cast<sf::Int16>(amplitude * sin(2 * PI * frequency * i / SAMPLE_RATE));
        }
    }

    sf::SoundBuffer buffer;
    buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
    return buffer;
}

// Функция обработки файла и воспроизведения музыки
void playMusic(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Ошибка: Не удалось открыть файл!\n";
        return;
    }

    std::string line;
    std::getline(file, line);
    int tempo = std::stoi(line);
    int duration_per_line_ms = 60000 / tempo;

    std::vector<sf::Sound> sounds;
    std::vector<sf::SoundBuffer> buffers;

    while (std::getline(file, line)) {
        if (line == "END") break;

        std::vector<std::string> notes;
        std::stringstream ss(line);
        std::string note;
        while (ss >> note) {
            notes.push_back(note);
        }

        sf::SoundBuffer buffer = generateSound(notes, duration_per_line_ms);
        buffers.push_back(buffer);

        sf::Sound sound;
        sound.setBuffer(buffers.back());
        sounds.push_back(sound);
    }

    for (auto& sound : sounds) {
        sound.play();
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_per_line_ms));
    }
}

void PrintUsage()
{
    Logger::Println("Usage:");
    Logger::Println("\t./music <inputFilePath>");
}

struct ProgramArgs
{
    std::string inputFilePath;
};

ProgramArgs ParseArgs(int argc, char* argv[])
{
    if (argc != 2)
    {
        PrintUsage();
        throw std::invalid_argument("invalid count of arguments");
    }

    ProgramArgs args;
    args.inputFilePath = argv[1];

    return args;
}

int main(int argc, char* argv[])
{
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        ProgramArgs args = ParseArgs(argc, argv);
        playMusic(args.inputFilePath);
    });

    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}