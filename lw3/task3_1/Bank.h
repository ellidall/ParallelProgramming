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
    explicit Bank(Money initialMoney) : m_money(initialMoney)
    {
        if (initialMoney < 0)
        {
            throw std::out_of_range("Количество наличных в банке не может быть отрицательным");
        }
    };

    size_t GetAccountsCount() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return m_accounts.size();
    }

    AccountId OpenAccount()
    {
        std::lock_guard<std::mutex> lock(mtx);
        m_accounts[m_nextId] = 0;
        m_operationsCount++;
        return m_nextId++;
    }

    Money CloseAccount(AccountId accountId)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = m_accounts.find(accountId);
        if (it == m_accounts.end()) throw BankOperationError("Неизвестный аккаунт");

        Money balance = it->second;
        m_accounts.erase(it);
        m_money += balance;
        m_operationsCount++;
        return balance;
    }

    void DepositMoney(AccountId account, Money amount)
    {
        if (!TryDepositMoney(account, amount))
        {
            throw BankOperationError("Ошибка при депозите, AccountId = " + std::to_string(account));
        }
    }

    bool TryDepositMoney(AccountId account, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) return false;
        if (m_money < amount) return false;

        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) return false;

        it->second += amount;
        m_money -= amount;
        m_operationsCount++;
        return true;
    }

    void WithdrawMoney(AccountId account, Money amount)
    {
        if (!TryWithdrawMoney(account, amount))
        {
            throw BankOperationError("Ошибка при снятии денег, AccountId = " + std::to_string(account));
        }
    }

    bool TryWithdrawMoney(AccountId account, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) return false;

        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) return false;
        if (it->second < amount) return false; // Недостаточно средств на счету

        it->second -= amount;
        m_money += amount;
        m_operationsCount++;
        return true;
    }

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        if (!TrySendMoney(srcAccountId, dstAccountId, amount))
        {
            throw BankOperationError("Перевод невозможен");
        }
    }

    bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount < 0) return false;

        auto srcIt = m_accounts.find(srcAccountId);
        auto dstIt = m_accounts.find(dstAccountId);
        if (srcIt == m_accounts.end() || dstIt == m_accounts.end()) return false;
        if (srcIt->second < amount) return false;

        srcIt->second -= amount;
        dstIt->second += amount;
        m_operationsCount++;
        return true;
    }

    Money GetCash() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        Money total = 0;
        for (const auto& [accountId, balance] : m_accounts)
        {
            total += balance;
        }
        return total;
    }

    Money GetAccountBalance(AccountId accountId) const
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = m_accounts.find(accountId);
        if (it == m_accounts.end()) throw BankOperationError("Неизвестный аккаунт");
        return it->second;
    }

    unsigned long long GetOperationsCount() const
    {
        return m_operationsCount;
    }

private:
    mutable std::mutex mtx;
    AccountId m_nextId = 0;
    std::map<AccountId, Money> m_accounts;
    std::atomic<unsigned long long> m_operationsCount = 0;
    Money m_money;
};