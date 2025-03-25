#pragma once

#include "../_libs/_logger.h"
#include "Actor.h"

class Marge : public Actor
{
public:
    Marge(AccountId id, AccountId apuId, Bank& bank)
            : Actor(id), m_apuId(apuId), m_bank(bank) {}

    void Execute() override
    {
        Money groceryCost = 500;

        if (m_bank.TrySendMoney(m_id, m_apuId, groceryCost))
        {
            Logger::OPrintln("Мардж купила продукты у Апу");
        }
    }

private:
    AccountId m_apuId;
    Bank& m_bank;
};
