#pragma once

#include "../_libs/_logger.h"
#include "Actor.h"
#include "Bart.h"
#include "Liza.h"

class Homer : public Actor
{
public:
    Homer(AccountId id, AccountId margeId, AccountId burnsId, Bank& bank, Bart& bart, Liza& liza)
            : Actor(id), m_margeId(margeId), m_burnsId(burnsId), m_bank(bank), m_bart(bart), m_liza
            (liza) {}

    ~Homer() override = default;

    void Execute() override
    {
        Money amount = 1000;
        Money amountForMarge = amount * 0.5;
        Money amountForElectricity = amount * 0.2;
        Money cashForBart = amount * 0.1;
        Money cashForLiza = amount * 0.1;

        if (m_bank.TrySendMoney(m_id, m_margeId, amountForMarge))
        {
            Logger::OPrintln("Гомер перевел деньги Мардж");
        }
        if (m_bank.TrySendMoney(m_id, m_burnsId, amountForElectricity))
        {
            Logger::OPrintln("Гомер оплатил электричество");
        }
        if (m_bank.TryWithdrawMoney(m_id, cashForBart + cashForLiza))
        {
            AddCash(cashForBart + cashForLiza);
            Logger::OPrintln("Гомер снял деньги");
        }

        if (m_money >= (cashForBart + cashForLiza))
        {
            SendMoney(m_bart, cashForBart);
            SendMoney(m_liza, cashForLiza);
            Logger::OPrintln("Гомер дал наличные Барту");
            Logger::OPrintln("Гомер дал наличные Лизе");
        }
    }

private:
    AccountId m_margeId;
    AccountId m_burnsId;
    Bank& m_bank;
    Bart& m_bart;
    Liza& m_liza;
};

