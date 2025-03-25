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
#include "actor/Actor.h"
#include "actor/Bart.h"
#include "actor/Liza.h"
#include "actor/Marge.h"
#include "actor/Homer.h"
#include "actor/Berns.h"
#include "actor/Apu.h"

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
        Logger::OPrintln("-----------------------------");
        for (auto& actor : m_actors)
        {
            try
            {
                actor->DoExecute();
            }
            catch (const std::exception& e)
            {
                Logger::Error(e.what());
                return;
            }
        }
        Logger::OPrintln("-----------------------------");
    }

    void ExecuteParallel()
    {
        Start();
        std::vector<std::jthread> threads;
        threads.reserve(m_actors.size());

        for (auto& actor : m_actors)
        {
            threads.emplace_back([self = shared_from_this(), actorPtr = actor]() {
                Timer timer;
                while (self->m_isRunning.load())
                {
                    if (timer.GetElapsed() >= self->m_simulationDuration) break;
                    Timer localTimer;
                    actorPtr->DoExecute();
                    if (self->m_isDurationOn) self->AddSleep(localTimer);
                }
            });
        }
    }

private:
    std::shared_ptr<Bank> m_bank;
    std::vector<std::shared_ptr<Actor>> m_actors;
    Money m_initialCash;
    double m_simulationDuration;
    std::atomic<bool> m_isRunning;
    bool m_isLoggingOn;
    bool m_isDurationOn;

    void AddSleep(Timer& timer) const
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
        AccountId bartId = m_bank->OpenAccount();
        AccountId lizaId = m_bank->OpenAccount();
        AccountId apuId = m_bank->OpenAccount();
        AccountId bernsId = m_bank->OpenAccount();

        auto berns = std::make_shared<Berns>(bernsId, homerId, *m_bank);
        auto apu = std::make_shared<Apu>(apuId, bernsId, *m_bank);
        auto bart = std::make_shared<Bart>(bartId, *apu);
        auto liza = std::make_shared<Liza>(lizaId, *apu);
        auto marge = std::make_shared<Marge>(margeId, apuId, *m_bank);
        auto homer = std::make_shared<Homer>(homerId, margeId, bernsId, *m_
        bank, *bart, *liza);

        m_actors = {homer, marge, bart, liza, apu, berns};

        m_bank->DepositMoney(homerId, static_cast<Money>(m_initialCash / 4));
        m_bank->DepositMoney(margeId, static_cast<Money>(m_initialCash / 4));
        m_bank->DepositMoney(apuId, static_cast<Money>(m_initialCash / 4));
        m_bank->DepositMoney(bernsId, static_cast<Money>(m_initialCash / 4));
    }
};