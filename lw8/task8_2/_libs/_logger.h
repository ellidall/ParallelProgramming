#pragma once

#include <iostream>
#include <string>
#include <ostream>
#include <syncstream>

class Logger {
public:
    static void Print(const std::string& message) {
        std::cout << message;
    }

    static void Println(const std::string& message) {
        std::cout << message << std::endl;
    }

    static void Error(const std::string& message) {
        std::cerr << "ERROR: " << message << std::endl;
    }

    static void OPrint(const std::string& message) {
        std::string logMessage = message + "\n";
        std::osyncstream(std::cout) << logMessage;
    }

    static void OPrintln(const std::string& message) {
        std::string logMessage = message + "\n";
        std::osyncstream(std::cout) << logMessage;
    }

    static void OError(const std::string& message) {
        std::string logMessage = "ERROR: " + message + "\n";
        std::osyncstream(std::cout) << logMessage;
    }
};
