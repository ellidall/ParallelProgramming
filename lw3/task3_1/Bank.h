#pragma once

#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
#include <stdexcept>

using AccountId = unsigned long long;
using Money = long long;

class BankOperationError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Bank
{
public:
    explicit Bank(Money initialCash) : m_cash(initialCash), m_operationsCount(0)
    {
        if (initialCash < 0)
        {
            throw std::out_of_range("Начальное количество денег в банке не может быть отрицательным");
        }
    };

    size_t GetAccountsCount()
    {
        return accounts.size();
    }

    AccountId OpenAccount()
    {
        std::lock_guard<std::mutex> lock(mtx);
        accounts[m_nextId] = 0;
        m_operationsCount++;
        return m_nextId++;
    }

    Money CloseAccount(AccountId accountId)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (accounts.find(accountId) == accounts.end())
        {
            throw BankOperationError("Неизвестный аккаунт");
        }
        Money balance = accounts[accountId];
        m_cash += balance;
        accounts.erase(accountId);
        m_operationsCount++;
        return balance;
    }

    void DepositMoney(AccountId account, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Нельзя положить на счёт отрицательное число");
        if (m_cash < amount) throw BankOperationError("Недостаточно денег в банке ");

        if (accounts.find(account) == accounts.end()) throw BankOperationError("Invalid account ID");

        accounts[account] += amount;
        m_cash -= amount;
        m_operationsCount++;
    }

    void WithdrawMoney(AccountId account, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Нельзя снять со счёта отрицательное число");
        if (accounts.find(account) == accounts.end()) throw BankOperationError("Неизвестный аккаунт");
        if (accounts[account] < amount) throw BankOperationError("Недостаточно средств");

        accounts[account] -= amount;
        m_cash += amount;
        m_operationsCount++;
    }

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Нельзя отправить на счёт отрицательное число");
        if (accounts.find(srcAccountId) == accounts.end() || accounts.find(dstAccountId) == accounts.end())
        {
            throw BankOperationError("Неизвестный аккаунт");
        }
        if (accounts[srcAccountId] < amount)
        {
            throw BankOperationError("Недостаточно средств");
        }

        accounts[srcAccountId] -= amount;
        accounts[dstAccountId] += amount;
        m_operationsCount++;
    }

    bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Negative transfer amount");
        if (accounts.find(srcAccountId) == accounts.end() || accounts.find(dstAccountId) == accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }
        if (accounts[srcAccountId] < amount) return false;

        accounts[srcAccountId] -= amount;
        accounts[dstAccountId] += amount;
        m_operationsCount++;
        return true;
    }

    Money GetCash() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return m_cash;
    }

    Money GetAccountBalance(AccountId accountId) const
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (accounts.find(accountId) == accounts.end()) throw BankOperationError("Invalid account ID");
        return accounts.at(accountId);
    }

    unsigned long long GetOperationsCount() const
    {
        return m_operationsCount;
    }

private:
    mutable std::mutex mtx;
    AccountId m_nextId = 0;
    std::map<AccountId, Money> accounts;
    Money m_cash;
    std::atomic<unsigned long long> m_operationsCount;
};