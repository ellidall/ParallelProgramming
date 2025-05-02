// tv_boost.cpp
//
// Необходимые библиотеки:
//   - Boost (поддержка Boost::asio)
//   - OpenCV (core, highgui, imgcodecs, videoio)
//
// Сборка (пример для Visual Studio):
//   - Добавьте пути к заголовочным файлам Boost и OpenCV
//   - Добавьте необходимые библиотеки OpenCV
//   - Если используете CMake, можно создать соответствующий CMakeLists.txt
//
// Запуск:
//   Режим сервера: tv_boost.exe PORT
//   Режим клиента: tv_boost.exe ADDRESS PORT

#pragma once

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>

// Для конвертации порядка байт используем стандартные функции.
// В Windows можно подключить winsock2.h, а на Linux – arpa/inet.h.
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using boost::asio::ip::tcp;

// Типы передаваемых сообщений.
enum MessageType : uint8_t {
    VIDEO_DATA = 0,
    AUDIO_DATA = 1
};

#pragma pack(push, 1)
// Структура заголовка пакета (1 байт — тип, 4 байта — метка времени, 4 байта — размер полезной нагрузки)
struct MessageHeader {
    uint8_t type;
    uint32_t timestamp; // Перед отправкой преобразуем в сетевой порядок байт
    uint32_t dataSize;  // Размер данных (также передаем в сетевом порядке)
};
#pragma pack(pop)

// Глобальные переменные: список подключенных клиентов, мьютекс для защиты доступа и флаг работы.
std::vector<std::shared_ptr<tcp::socket>> clientSockets;
std::mutex clientsMutex;
std::atomic<bool> running(true);

// Функция широковещательной отправки данных всем подключенным клиентам.
void broadcastData(const char* data, size_t dataSize) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
        boost::system::error_code ec;
        // Boost::asio::write гарантирует, что данные отправлены полностью, если не возникнет ошибка.
        boost::asio::write(**it, boost::asio::buffer(data, dataSize), ec);
        if (ec) {
            // Если произошла ошибка отправки, удаляем сокет.
            it = clientSockets.erase(it);
        } else {
            ++it;
        }
    }
}

// Поток для захвата с веб-камеры и отправки сжатых кадров (JPEG) клиентам.
void videoThread() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Ошибка: невозможно открыть камеру." << std::endl;
        running = false;
        return;
    }

    // Параметры сжатия JPEG (качество 70)
    std::vector<int> jpegParams = { cv::IMWRITE_JPEG_QUALITY, 70 };

    while (running) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            std::cerr << "Ошибка: не удалось захватить кадр." << std::endl;
            continue;
        }

        // Получаем метку времени (секунды с начала эпохи)
        uint32_t timestamp = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count());

        // Сжимаем кадр в JPEG
        std::vector<uchar> buf;
        if (!cv::imencode(".jpg", frame, buf, jpegParams)) {
            std::cerr << "Ошибка: не удалось сжать кадр." << std::endl;
            continue;
        }

        // Формируем заголовок сообщения
        MessageHeader header;
        header.type = VIDEO_DATA;
        header.timestamp = htonl(timestamp);  // Приводим к сетевому порядку байт
        header.dataSize = htonl(static_cast<uint32_t>(buf.size()));

        // Готовим общий буфер: заголовок + сжатые данные
        size_t headerSize = sizeof(MessageHeader);
        size_t totalSize = headerSize + buf.size();
        std::vector<char> sendBuffer(totalSize);
        memcpy(sendBuffer.data(), &header, headerSize);
        memcpy(sendBuffer.data() + headerSize, buf.data(), buf.size());

        // Отправляем получившийся пакет всем клиентам
        broadcastData(sendBuffer.data(), totalSize);

        // Ограничиваем количество кадров – примерно 30 FPS.
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

// Поток для (примерного) захвата аудио и его отправки.
// В этом примере вместо настоящего аудио передается синтетический нулевой буфер.
// Для полноценного захвата и сжатия аудио можно использовать PortAudio и LAME (MP3).
void audioThread() {
    const int audioFrameSize = 512; // Размер аудио-фрейма
    std::vector<char> audioFrame(audioFrameSize, 0); // Здесь можно захватить реальные данные
    while (running) {
        uint32_t timestamp = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count());

        MessageHeader header;
        header.type = AUDIO_DATA;
        header.timestamp = htonl(timestamp);
        header.dataSize = htonl(audioFrameSize);

        size_t headerSize = sizeof(MessageHeader);
        size_t totalSize = headerSize + audioFrameSize;
        std::vector<char> sendBuffer(totalSize);
        memcpy(sendBuffer.data(), &header, headerSize);
        memcpy(sendBuffer.data() + headerSize, audioFrame.data(), audioFrameSize);

        broadcastData(sendBuffer.data(), totalSize);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Поток для принятия входящих подключений (работает на сервере)
// Здесь используется блокирующий метод accept, каждый принятый клиент сохраняется в списке.
void acceptClients(tcp::acceptor &acceptor, boost::asio::io_context &io_context) {
    while (running) {
        boost::system::error_code ec;
        auto socket = std::make_shared<tcp::socket>(io_context);
        acceptor.accept(*socket, ec);
        if (ec) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(socket);
        }
        std::cout << "Подключился клиент." << std::endl;
    }
}

// Функция для работы в режиме сервера (телестанция)
void runServer(const char* portStr) {
    try {
        boost::asio::io_context io_context;
        // Создаём приёмник на указанном порту
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::stoi(portStr)));
        std::cout << "Сервер запущен. Ожидание клиентов на порту " << portStr << "..." << std::endl;

        // Запускаем потоки: для принятия клиентов, захвата видео и аудио
        std::thread acceptThread(acceptClients, std::ref(acceptor), std::ref(io_context));
        std::thread videoSender(videoThread);
        std::thread audioSender(audioThread);

        std::cout << "Для завершения работы введите q и нажмите Enter." << std::endl;
        char cmd;
        while (std::cin >> cmd) {
            if (cmd == 'q' || cmd == 'Q') {
                running = false;
                break;
            }
        }

        acceptThread.join();
        videoSender.join();
        audioSender.join();

        // Корректное закрытие всех клиентских сокетов
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& sock : clientSockets) {
            boost::system::error_code ec;
            sock->shutdown(tcp::socket::shutdown_both, ec);
            sock->close(ec);
        }
        std::cout << "Сервер завершил работу." << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
    }
}

// Функция для работы в режиме клиента (телеприёмник)
void runClient(const char* address, const char* portStr) {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(address, portStr);
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Подключено к " << address << ":" << portStr << std::endl;
        cv::namedWindow("Video", cv::WINDOW_AUTOSIZE);

        while (running) {
            MessageHeader header;
            size_t headerSize = sizeof(MessageHeader);
            boost::system::error_code ec;
            // Читаем заголовок пакета
            size_t received = boost::asio::read(socket, boost::asio::buffer(&header, headerSize), ec);
            if (ec || received != headerSize) {
                std::cerr << "Ошибка получения заголовка или соединение разорвано." << std::endl;
                running = false;
                break;
            }
            // Приводим сетевые данные к локальному формату
            header.timestamp = ntohl(header.timestamp);
            uint32_t dataSize = ntohl(header.dataSize);

            std::vector<char> payload(dataSize);
            size_t totalReceived = boost::asio::read(socket, boost::asio::buffer(payload.data(), dataSize), ec);
            if (ec || totalReceived != dataSize) {
                std::cerr << "Ошибка получения данных или соединение разорвано." << std::endl;
                running = false;
                break;
            }

            if (header.type == VIDEO_DATA) {
                // Декодирование JPEG-изображения и вывод через OpenCV
                cv::Mat dataMat(1, dataSize, CV_8UC1, payload.data());
                cv::Mat frame = cv::imdecode(dataMat, cv::IMREAD_COLOR);
                if (!frame.empty()) {
                    cv::imshow("Video", frame);
                    if (cv::waitKey(1) == 27) { // Esc для выхода
                        running = false;
                        break;
                    }
                }
            } else if (header.type == AUDIO_DATA) {
                // Обработка аудио-пакета – в данном примере аудио не воспроизводится,
                // но можно интегрировать механизм воспроизведения (например, через PortAudio)
                std::cout << "Получен аудио-пакет, размер " << dataSize
                          << " байт, timestamp=" << header.timestamp << std::endl;
            } else {
                std::cerr << "Неизвестный тип пакета: " << int(header.type) << std::endl;
            }
        }

        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        socket.close(ec);
        cv::destroyAllWindows();
    } catch (std::exception &e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        // Режим сервера: tv_boost.exe PORT
        runServer(argv[1]);
    } else if (argc == 3) {
        // Режим клиента: tv_boost.exe ADDRESS PORT
        runClient(argv[1], argv[2]);
    } else {
        std::cout << "Использование:" << std::endl;
        std::cout << "  tv_boost.exe PORT            - запуск в режиме телестанции (сервер)" << std::endl;
        std::cout << "  tv_boost.exe ADDRESS PORT    - запуск в режиме телеприёмника (клиент)" << std::endl;
    }
    return 0;
}
