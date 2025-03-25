#pragma once

#include "../_libs/_logger.h"
#include "Actor.h"

class Berns : public Actor
{
public:
    Berns(AccountId id, AccountId homerId, Bank& bank)
            : Actor(id), m_homerId(homerId), m_bank(bank)
    {}

    void Execute() override
    {
        Money salary = 1000;
        if (m_bank.TrySendMoney(m_id, m_homerId, salary))
        {
            Logger::OPrintln("Мистер Бернс выплатил зарплату Гомеру");
        }
    }

private:
    AccountId m_homerId;
    Bank& m_bank;
};
