#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using boost::asio::ip::udp;

void start_tv_station(unsigned short port) {
    boost::asio::io_context io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));
    udp::endpoint remote_endpoint;

    VideoCapture cap(0); // Захват видео с веб-камеры
    if (!cap.isOpened()) {
        std::cerr << "Ошибка: не удалось открыть веб-камеру!" << std::endl;
        return;
    }

    Mat frame;
    std::vector<uchar> encoded;
    std::vector<int> params = { IMWRITE_JPEG_QUALITY, 50 }; // JPEG сжатие 50%

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        imencode(".jpg", frame, encoded, params);
        size_t frame_size = encoded.size();

        socket.send_to(boost::asio::buffer(&frame_size, sizeof(size_t)), remote_endpoint);
        socket.send_to(boost::asio::buffer(encoded), remote_endpoint);
    }
}

void start_tv_receiver(const std::string& address, unsigned short port) {
    boost::asio::io_context  io_service;
    udp::socket socket(io_service);
    udp::endpoint server_endpoint(boost::asio::ip::make_address_v4(address), port);

    socket.open(udp::v4());
    socket.connect(server_endpoint);

    namedWindow("TV Stream", WINDOW_AUTOSIZE);

    while (true) {
        size_t frame_size;
        socket.receive(boost::asio::buffer(&frame_size, sizeof(size_t)));

        std::vector<uchar> encoded(frame_size);
        socket.receive(boost::asio::buffer(encoded));

        Mat frame = imdecode(encoded, IMREAD_COLOR);
        if (frame.empty()) continue;

        imshow("TV Stream", frame);
        if (waitKey(30) >= 0) break;
    }
}
