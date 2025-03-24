#pragma once

#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <utility>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"
#include "_libs/_timer.h"
#include "Bank.h"
#include "Actor.h"

class Simulation : public std::enable_shared_from_this<Simulation>
{
public:
    const double STEP_DURATION = 1.0;

    explicit Simulation(
            std::shared_ptr<Bank> bank, Money initialCash, double durationSeconds,
            bool isLoggingOn = true, bool isDurationOn = true
    )
            : m_initialCash(initialCash),
              m_bank(bank),
              m_simulationDuration(durationSeconds),
              m_isRunning(false),
              m_isLoggingOn(isLoggingOn),
              m_isDurationOn(isDurationOn)
    {
        InitializeActors();
    }

    ~Simulation()
    {
        Stop();
    }

    void Start()
    {
        m_isRunning.store(true);
    }

    void Stop()
    {
        m_isRunning.store(false);
    }

    void ExecuteSequential()
    {
        Start();
        Timer timer;
        while (m_isRunning.load())
        {
            if (timer.GetElapsed() >= m_simulationDuration) break;
            Timer localTimer;
            ExecuteSequentialStep();
            if (m_isDurationOn) AddSleep(localTimer);
        }
    }

    void ExecuteSequentialStep()
    {
        for (auto& actor : m_actors)
        {
            actor.ExecuteAction();
        }
    }

    void ExecuteParallel()
    {
        Start();
        std::vector<std::jthread> threads;
        threads.reserve(m_actors.size());

        for (auto& actor : m_actors)
        {
            threads.emplace_back([self = shared_from_this(), actorRef = std::ref(actor)]() {
                Timer timer;
                while (self->m_isRunning.load())
                {
                    if (timer.GetElapsed() >= self->m_simulationDuration) break;
                    Timer localTimer;
                    actorRef.get().ExecuteAction();
                    if (self->m_isDurationOn) AddSleep(localTimer);
                }
            });
        }
    }

private:
    std::shared_ptr<Bank> m_bank;
    std::vector<Actor> m_actors;
    Money m_initialCash;
    double m_simulationDuration;
    std::atomic<bool> m_isRunning;
    bool m_isLoggingOn;
    bool m_isDurationOn;

    void AddSleep(Timer& timer)
    {
        double delta = STEP_DURATION - timer.GetElapsed();
        if (delta > 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(delta));
        }
    }

    void InitializeActors()
    {
        AccountId homerId = m_bank->OpenAccount();
        AccountId margeId = m_bank->OpenAccount();
        AccountId apuId = m_bank->OpenAccount();

        m_actors.emplace_back(homerId, [bank = this->m_bank, homerId, margeId, isLoggingOn = m_isLoggingOn]() {
            ExceptionHandler exceptionHandler;

            exceptionHandler.Handle([bank, homerId, margeId, isLoggingOn]() {
                Money amount = 100;
                bank->SendMoney(homerId, margeId, amount);

                if (isLoggingOn)
                {
                    Logger::Println("Гомер перевёл " + std::to_string(amount) + " на счёт Мардж");
                    auto homerBalance = bank->GetAccountBalance(homerId);
                    auto margeBalance = bank->GetAccountBalance(margeId);
                    Logger::Println("Баланс Гомера: " + std::to_string(homerBalance));
                    Logger::Println("Баланс Мардж: " + std::to_string(margeBalance));
                }
            });

            if (exceptionHandler.WasExceptionCaught())
            {
                Logger::Error("Ошибка у Гомера " + exceptionHandler.GetErrorMessage());
            }
        });
        m_actors.emplace_back(margeId, [bank = this->m_bank, apuId, margeId, isLoggingOn = m_isLoggingOn]() {
            ExceptionHandler exceptionHandler;

            exceptionHandler.Handle([bank, apuId, margeId, isLoggingOn]() {
                Money amount = 50;
                bank->SendMoney(margeId, apuId, amount);

                if (isLoggingOn)
                {
                    Logger::Println("Мардж перевела " + std::to_string(amount) + " на счёт Апу за продукты");
                    auto apuBalance = bank->GetAccountBalance(apuId);
                    auto margeBalance = bank->GetAccountBalance(margeId);
                    Logger::Println("Баланс Апу: " + std::to_string(apuBalance));
                    Logger::Println("Баланс Мардж: " + std::to_string(margeBalance));
                }
            });

            if (exceptionHandler.WasExceptionCaught())
            {
                Logger::Error("Ошибка у Мардж " + exceptionHandler.GetErrorMessage());
            }
        });
        m_actors.emplace_back(apuId, [bank = this->m_bank, apuId, margeId, isLoggingOn = m_isLoggingOn]() {
        });

        if (auto accountsCount = m_bank->GetAccountsCount(); accountsCount > 0)
        {
            for (auto& actor : m_actors)
            {
                m_bank->DepositMoney(actor.GetId(), static_cast<Money>(m_initialCash / accountsCount));
            }
            m_bank->DepositMoney(m_actors.back().GetId(), static_cast<Money>(m_initialCash % accountsCount));
        }
        auto homerBalance = m_bank->GetAccountBalance(homerId);
        auto margeBalance = m_bank->GetAccountBalance(margeId);
        auto apuBalance = m_bank->GetAccountBalance(apuId);
        Logger::Println("Баланс Гомера: " + std::to_string(homerBalance));
        Logger::Println("Баланс Мардж: " + std::to_string(margeBalance));
        Logger::Println("Баланс Апу: " + std::to_string(apuBalance));
    }
};
