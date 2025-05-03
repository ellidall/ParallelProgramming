#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "Television.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

constexpr int AUDIO_PORT = 5001;
constexpr int VIDEO_PORT = 5002;

void Television::Run()
{
    if (m_mode == Mode::Station)
    {
        RunStation();
    }
    else
    {
        RunReceiver();
    }
}

void Television::RunStation()
{
    std::cout << "Starting TV Station on port " << m_port << std::endl;

    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cerr << "Failed to open webcam.\n";
        return;
    }

    boost::asio::io_context ioContext;

    tcp::acceptor audioAcceptor(ioContext, tcp::endpoint(tcp::v4(), AUDIO_PORT));
    tcp::socket audioSocket(ioContext);
    audioAcceptor.accept(audioSocket);

    udp::socket videoSocket(ioContext, udp::endpoint(udp::v4(), VIDEO_PORT));

    while (true)
    {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty())
        {
            break;
        }

        std::vector<uchar> buffer;
        cv::imencode(".jpg", frame, buffer);

        uint64_t timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count());

        std::vector<uchar> packet(sizeof(timestamp) + buffer.size());
        memcpy(packet.data(), &timestamp, sizeof(timestamp));
        memcpy(packet.data() + sizeof(timestamp), buffer.data(), buffer.size());

        // Здесь videoSocket.send_to(...), если clientEndpoint известен

        std::string audioData = "AUDIOFRAME";
        boost::asio::write(audioSocket, boost::asio::buffer(audioData));
    }
}

void Television::RunReceiver()
{
    std::cout << "Connecting to TV Station at " << m_address << ":" << m_port << std::endl;

    boost::asio::io_context ioContext;

    tcp::socket audioSocket(ioContext);
    tcp::resolver resolver(ioContext);
    boost::asio::connect(audioSocket, resolver.resolve(m_address, std::to_string(AUDIO_PORT)));

    udp::socket videoSocket(ioContext, udp::endpoint(udp::v4(), VIDEO_PORT));

    std::vector<uchar> recvBuffer(65536);

    while (true)
    {
        udp::endpoint sender;
        size_t len = videoSocket.receive_from(boost::asio::buffer(recvBuffer), sender);

        if (len <= sizeof(uint64_t))
            continue;

        uint64_t timestamp;
        memcpy(&timestamp, recvBuffer.data(), sizeof(uint64_t));

        std::vector<uchar> imageData(recvBuffer.begin() + sizeof(uint64_t), recvBuffer.begin() + len);
        cv::Mat frame = cv::imdecode(imageData, cv::IMREAD_COLOR);
        if (!frame.empty())
        {
            cv::imshow("TV Receiver", frame);
            if (cv::waitKey(1) == 27)
                break;
        }

        char audioBuf[256];
        size_t audioLen = audioSocket.read_some(boost::asio::buffer(audioBuf));
        std::string audio(audioBuf, audioLen);
        std::cout << "Audio: " << audio << std::endl;
    }
}

bool Television::IsLittleEndian()
{
    uint16_t value = 1;
    return *reinterpret_cast<char*>(&value) == 1;
}
