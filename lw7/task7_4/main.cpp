#include "Task.h"
#include "Dispatcher.h"
#include "AsyncFile.h"

#include <iostream>
#include <vector>
#include <string>

Task<void> AsyncCopyFile(Dispatcher& dispatcher, std::string fromPath, std::string toPath)
{
    std::cout << "Начало копирования из " << fromPath << " в " << toPath << std::endl;
    AsyncFile inputFile;
    AsyncFile outputFile;

    try
    {
        inputFile = co_await AsyncOpenFile(dispatcher, fromPath, OpenMode::Read);
        if (!inputFile)
        {
            std::cerr << "Не удалось открыть входной файл: " << fromPath << std::endl;
            co_return;
        }
        std::cout << "Файл " << fromPath << " открыт для чтения." << std::endl;

        outputFile = co_await AsyncOpenFile(dispatcher, toPath, OpenMode::Write);
        if (!outputFile)
        {
            std::cerr << "Не удалось открыть/создать выходной файл: " << toPath << std::endl;
            co_return;
        }
        std::cout << "Файл " << toPath << " открыт для записи." << std::endl;

        std::vector<char> buffer(1024 * 1024);
        unsigned long long totalBytesCopied = 0;

        for (unsigned bytesRead;
             (bytesRead = co_await inputFile.ReadAsync(buffer.data(), buffer.size())) != 0;
                )
        {
            unsigned bytesWritten = co_await outputFile.WriteAsync(buffer.data(), bytesRead);
            std::cout << ".";
            std::cout.flush();
            if (bytesWritten != bytesRead)
            {
                std::cerr << "Ошибка записи: записано " << bytesWritten << " вместо " << bytesRead << " для файла "
                          << toPath << std::endl;
                break;
            }
            totalBytesCopied += bytesWritten;
        }
        std::cout << "Копирование " << fromPath << " -> " << toPath << " завершено. Всего скопировано: "
                  << totalBytesCopied << " байт." << std::endl;

    }
    catch (const std::system_error& e)
    {
        std::cerr << "Системная ошибка во время копирования " << fromPath << " -> " << toPath << ": " << e.what()
                  << " (код: " << e.code() << ")" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка во время копирования " << fromPath << " -> " << toPath << ": " << e.what() << std::endl;
    }
}


// добавить возможность выполнения нескольких await разу (Boost.cobalt.gather)
Task<void> AsyncCopyTwoFiles(Dispatcher& dispatcher)
{
    std::cout << "Запуск AsyncCopyTwoFiles" << std::endl;
    Task<void> t1 = AsyncCopyFile(dispatcher, "1.mp4", "out");
    Task<void> t2 = AsyncCopyFile(dispatcher, "input", "output");

    if (t1) co_await t1;
    std::cout << "Первое копирование (или его попытка) завершено." << std::endl;
    if (t2) co_await t2;
    std::cout << "Второе копирование (или его попытка) завершено." << std::endl;
}


int main()
{
    std::cout << "Программа запускается..." << std::endl;

    try
    {
        Dispatcher dispatcher(64);
        std::cout << "Dispatcher создан и запущен." << std::endl;

        Task<void> mainTask = AsyncCopyTwoFiles(dispatcher);
        std::cout << "Главная задача AsyncCopyTwoFiles создана. Запускаем..." << std::endl;

        mainTask.Start();

        std::cout << "Все задачи запущены. Dispatcher работает в фоновом режиме." << std::endl;
        std::cout << "Нажмите Enter для завершения программы..." << std::endl;
        std::cin.get();

        std::cout << "Остановка Dispatcher..." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Критическая ошибка в main: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Программа завершена." << std::endl;
    return 0;
}