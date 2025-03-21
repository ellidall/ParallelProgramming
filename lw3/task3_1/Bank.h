#pragma once

#include <iostream>
#include <unordered_map>
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
            throw BankOperationError("Initial m_cash cannot be negative");
        }
    };

    AccountId OpenAccount()
    {
        std::lock_guard<std::mutex> lock(mtx);
        static AccountId nextId = 1;
        accounts[nextId] = 0;
        m_operationsCount++;
        return nextId++;
    }

    Money CloseAccount(AccountId accountId)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (accounts.find(accountId) == accounts.end())
        {
            throw BankOperationError("Invalid account ID");
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
        if (amount < 0) throw std::out_of_range("Negative deposit amount");
        if (m_cash < amount) throw BankOperationError("Not enough m_cash in circulation");

        if (accounts.find(account) == accounts.end()) throw BankOperationError("Invalid account ID");

        accounts[account] += amount;
        m_cash -= amount;
        m_operationsCount++;
    }

    void WithdrawMoney(AccountId account, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Negative withdrawal amount");

        if (accounts.find(account) == accounts.end()) throw BankOperationError("Invalid account ID");

        if (accounts[account] < amount) throw BankOperationError("Not enough funds");

        accounts[account] -= amount;
        m_cash += amount;
        m_operationsCount++;
    }

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) throw std::out_of_range("Negative transfer amount");
        if (accounts.find(srcAccountId) == accounts.end() || accounts.find(dstAccountId) == accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }
        if (accounts[srcAccountId] < amount)
        {
            throw BankOperationError("Insufficient funds");
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
    std::unordered_map<AccountId, Money> accounts;
    Money m_cash;
    std::atomic<unsigned long long> m_operationsCount;
};