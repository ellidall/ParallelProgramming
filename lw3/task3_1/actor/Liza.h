#pragma once

#include "../_libs/_logger.h"
#include "Actor.h"

class Liza : public Actor
{
public:
    Liza(AccountId id, Apu& apu): Actor(id), m_apu(apu) {}

    void Execute() override
    {
        Money spendAmount = 10;

        if (m_money >= spendAmount)
        {
            m_apu.SendMoney(m_apu, spendAmount);
            Logger::OPrintln("Лиза купила что-то у Апу");
        }
    }

private:
    Apu& m_apu;
};
