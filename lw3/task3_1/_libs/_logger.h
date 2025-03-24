#pragma once

#include <iostream>
#include <string>

class Logger {
public:
    // Выводит сообщение в std::cout без перевода строки
    static void Print(const std::string& message) {
        std::cout << message;
    }

    // Выводит сообщение в std::cout с переводом строки
    static void Println(const std::string& message) {
        std::cout << message << std::endl;
    }

    // Выводит сообщение об ошибке в std::cerr с переводом строки
    static void Error(const std::string& message) {
        std::cerr << "ERROR: " << message << std::endl;
    }
};
