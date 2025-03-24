#pragma once

#include <iostream>
#include <functional>
#include "Bank.h"

class Actor
{
public:
    Actor(AccountId id, std::function<void()> action)
            : m_id(id), m_action(std::move(action))
    {}

    [[nodiscard]] AccountId GetId() const
    {
        return m_id;
    }

    [[nodiscard]] Money GetCash() const
    {
        return m_cash;
    }

    void AddCash(Money cash)
    {
        if (cash < 0) throw std::out_of_range("Negative money amount");
        m_cash += cash;
    }

    void SendMoney(Actor& recipient, Money amount)
    {
        if (amount < 0) throw std::out_of_range("Negative money amount");
        if (m_cash < amount) throw std::runtime_error("Can't send more than is on balance");
        m_cash -= amount;
        recipient.AddCash(amount);
    }

    void ExecuteAction()
    {
        if (m_action)
        {
            m_action();
        }
    }

private:
    AccountId m_id;
    Money m_cash = 0;
    std::function<void()> m_action = nullptr;
};