#pragma once

#include "../_libs/_logger.h"
#include "Actor.h"

class Apu : public Actor
{
public:
    Apu(AccountId id, AccountId bernsId, Bank& bank)
            : Actor(id), m_bernsId(bernsId), m_bank(bank)
    {}

    void Execute() override
    {
        Money electricityBill = 200;
        if (m_bank.TrySendMoney(m_id, m_bernsId, electricityBill))
        {
            Logger::OPrintln("Апу оплатил электричество");
        }

        if (m_money > 0)
        {
            m_bank.DepositMoney(m_id, m_money);
            m_money -= 0;
            Logger::OPrintln("Апу положил наличные в банк");
        }
    }

private:
    AccountId m_bernsId;
    Bank& m_bank;
};
