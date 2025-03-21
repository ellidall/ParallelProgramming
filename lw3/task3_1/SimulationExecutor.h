#pragma once

#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <utility>
#include "Bank.h"
#include "_libs/_exceptionHandler.h"

class SimulationExecutor
{
public:
    struct Actor
    {
        AccountId id;
        std::function<void()> ExecuteAction = nullptr;;

        Actor(AccountId id, std::function<void()> action)
                : id(id), ExecuteAction(std::move(action))
        {}
    };

    explicit SimulationExecutor(std::shared_ptr<Bank> bank, double durationSeconds)
            : m_bank(bank), m_simulationDuration(durationSeconds)
    {
        InitializeActors();
    }

//    void ExecuteSequential()
//    {
//        auto start = std::chrono::steady_clock::now();
//
//        while (!stopSimulation)
//        {
//            auto now = std::chrono::steady_clock::now();
//            double elapsedTime = std::chrono::duration<double>(now - start).count();
//            if (elapsedTime >= m_simulationDuration) break;  // Если время вышло — остановка
//
//            for (auto& actor : actors)
//            {
//                if (actor.ExecuteAction)
//                {
//                    actor.ExecuteAction();
//                }
//            }
//
//            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Имитируем задержку шага симуляции
//        }
//    }

    void ExecuteSequentialStep()
    {
        for (auto& actor : actors)
        {
            if (actor.ExecuteAction)
            {
                actor.ExecuteAction();
            }
        }
    }

//    void ExecuteParallel()
//    {
//        auto start = std::chrono::steady_clock::now();
//        std::vector<std::thread> threads;
//
//        for (auto& actor : actors)
//        {
//            threads.emplace_back([this, &actor, start]() {
//                while (!stopSimulation)
//                {
//                    auto now = std::chrono::steady_clock::now();
//                    double elapsedTime = std::chrono::duration<double>(now - start).count();
//                    if (elapsedTime >= m_simulationDuration) break;
//
//                    if (actor.ExecuteAction)
//                    {
//                        actor.ExecuteAction();
//                    }
//
//                    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Имитируем задержку
//                }
//            });
//        }
//
//        for (auto& thread : threads)
//        {
//            if (thread.joinable()) thread.join();
//        }
//    }

// TODO: инициализация
// TODO: добавить логгер, добавить возможность отключать логирование
// TODO: внедрить таймер, переработать задержку шагов симуляции, а также остановка симуляции в консистентном состоянии,
//       то есть даже есть время вышло - шаг выполняется до конца
// TODO: обработка сигналов, тоже так же, если поймали сигнал, то всё равно шаг до конца

private:
    std::shared_ptr<Bank> m_bank;
    std::vector<Actor> actors;
    double m_simulationDuration;

    void InitializeActors()
    {
        AccountId homerId = m_bank->OpenAccount();
        AccountId margeId = m_bank->OpenAccount();

        actors.emplace_back(homerId, [bank = this->m_bank, homerId, margeId]() {
            ExceptionHandler exceptionHandler;

            exceptionHandler.Handle([bank, homerId, margeId]() {
                Money amount = 100;
                bank->SendMoney(homerId, margeId, amount);
                Logger::Println("Гомер перевёл " + std::to_string(amount) + " на счёт Мардж");
            });

            if (exceptionHandler.WasExceptionCaught())
            {
                Logger::Error("Ошибка перевода денег от Гомера к Мардж: " + exceptionHandler.GetErrorMessage());
            }
        });
    }
};
