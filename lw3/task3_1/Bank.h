#pragma once

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <shared_mutex>

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
    struct Account
    {
        Money money{};
        mutable std::shared_mutex mutex;
    };

    explicit Bank(Money initialCash)
    {
        if (initialCash < 0)
        {
            throw std::out_of_range("Количество наличных в банке не может быть отрицательным");
        }
        m_cash = initialCash;
    };

    size_t GetAccountsCount() const
    {
        std::shared_lock lock(m_mutexAccounts);
        return m_accounts.size();
    }

    AccountId OpenAccount()
    {
        std::unique_lock lock(m_mutexAccounts);
        m_accounts.emplace(std::piecewise_construct, std::forward_as_tuple(m_nextId), std::forward_as_tuple());
        m_operationsCount++;
        return m_nextId++;
    }

    Money CloseAccount(AccountId accountId)
    {
        auto it = FindAccount(accountId);

        Money balance;
        {
            std::unique_lock lock(it->second.mutex);
            balance = it->second.money;
        }
        {
            std::unique_lock lock(m_mutexAccounts);
            m_accounts.erase(it);
        }
        {
            std::unique_lock lock(m_mutexCash);
            m_cash += balance;
        }
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
        if (amount < 0) throw std::out_of_range("Количество денег не может быть отрицательным");
        auto it = FindAccount(account);
        {
            std::unique_lock lock(m_mutexCash);
            if (m_cash < amount) return false;
            m_cash -= amount;
        }
        {
            std::unique_lock lock(it->second.mutex);
            it->second.money += amount;
        }

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
        if (amount < 0) throw std::out_of_range("Количество денег не может быть отрицательным");;
        auto it = FindAccount(account);
        {
            std::unique_lock lock(it->second.mutex);
            if (it->second.money < amount) return false;
            it->second.money -= amount;
        }
        {
            std::unique_lock lock(m_mutexCash);
            m_cash += amount;
        }
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
        if (amount < 0) throw std::out_of_range("Количество денег не может быть отрицательным");;

        std::shared_lock lock(m_mutexAccounts);
        auto srcIt = FindAccount(srcAccountId);
        auto dstIt = FindAccount(dstAccountId);

        {
            std::scoped_lock scopedLock(srcIt->second.mutex, dstIt->second.mutex);
            if (srcIt->second.money < amount) return false;
            srcIt->second.money -= amount;
            dstIt->second.money += amount;
        }

        m_operationsCount++;
        return true;
    }

    Money GetTotalMoney() const
    {
        std::shared_lock lock(m_mutexAccounts);
        Money total = 0;
        for (const auto& [accountId, acc] : m_accounts)
        {
            total += acc.money;
        }
        return total;
    }

    Money GetAccountBalance(AccountId accountId) const
    {
        auto it = FindAccount(accountId);
        std::shared_lock lock(it->second.mutex);
        return it->second.money;
    }

    unsigned long long GetOperationsCount() const
    {
        return m_operationsCount;
    }

private:
    mutable std::shared_mutex m_mutexCash;
    mutable std::shared_mutex m_mutexAccounts;// accountsMutex
    AccountId m_nextId = 0;
    std::unordered_map<AccountId, Account> m_accounts;
    std::atomic<unsigned long long> m_operationsCount = 0;
    Money m_cash;

    std::unordered_map<AccountId, Account>::iterator FindAccount(AccountId accountId)
    {
        std::shared_lock lock(m_mutexAccounts);
        auto it = m_accounts.find(accountId);
        if (it == m_accounts.end()) throw BankOperationError("Неизвестный аккаунт");
        return it;
    }

    std::unordered_map<AccountId, Account>::const_iterator FindAccount(AccountId accountId) const
    {
        std::shared_lock lock(m_mutexAccounts);
        auto it = m_accounts.find(accountId);
        if (it == m_accounts.end()) throw BankOperationError("Неизвестный аккаунт");
        return it;
    }
};