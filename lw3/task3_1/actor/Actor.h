#pragma once

#include <iostream>
#include <functional>
#include <functional>
#include <shared_mutex>
#include <stdexcept>
#include "../Bank.h"

class Actor
{
public:
    explicit Actor(AccountId id) : m_id(id) {}

    virtual ~Actor() = default;

    [[nodiscard]] AccountId GetId() const
    {
        std::shared_lock lock(m_mutex);
        return m_id;
    }

    [[nodiscard]] Money GetCash() const
    {
        std::shared_lock lock(m_mutex);
        return m_money;
    }

    void AddCash(Money cash)
    {
        if (cash < 0) throw std::out_of_range("Сумма не может быть отрицательной");

        std::unique_lock lock(m_mutex);
        m_money += cash;
    }

    void SendMoney(Actor& recipient, Money amount)
    {
        if (amount < 0) throw std::out_of_range("Сумма перевода не может быть отрицательной");

        std::unique_lock lock(m_mutex);
        if (m_money < amount)
        {
            throw std::runtime_error("Недостаточно средств для перевода");
        }
        m_money -= amount;

        lock.unlock();
        recipient.AddCash(amount);
    }

    void EnableLock(bool enable)
    {
        std::unique_lock lock(m_mutex);
        m_isLockEnabled = enable;
    }

    void DoExecute() {
        ExceptionHandler exceptionHandler;
        exceptionHandler.Handle([this]() {
            Execute();
        });

        if (exceptionHandler.WasExceptionCaught())
        {
            Logger::OError(exceptionHandler.GetErrorMessage());
            return;
        }
    }

    virtual void Execute() = 0;

protected:
    AccountId m_id;
    Money m_money = 0;
    mutable std::shared_mutex m_mutex;
    bool m_isLockEnabled = true;
};
