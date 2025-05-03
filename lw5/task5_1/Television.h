#pragma once

#include <string>

enum class Mode
{
    Station,
    Receiver
};

class Television
{
public:
    explicit Television(const Mode &mode, const std::string &address, const unsigned short m_port): m_mode(mode),
        m_address(address),
        m_port(m_port) {};

    void Run();

private:
    void RunStation();

    void RunReceiver();

    [[nodiscard]] bool IsLittleEndian();

private:
    Mode m_mode;
    std::string m_address;
    unsigned short m_port;
};
