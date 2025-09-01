#include <iostream>
#include <mutex>
#include <thread>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <atomic>

using namespace std;
mutex m_stdOut; //needed for clean console output


class BankAccount {

private:
    string accountName;
    atomic<int> balance;   
    atomic<int> transactionCount; 

public:
    BankAccount(int money, string name) : balance(money), accountName(name), transactionCount(0) {}

    void deposit(int amount)
    {
        balance.fetch_add(amount);
        addInterest();
    }

    bool withdraw(int amount)
    {
        int oldBalance;
        do {
            oldBalance = balance.load();
            if (oldBalance < amount)
                return false;
        } while (!balance.compare_exchange_strong(oldBalance, oldBalance - amount));
        addInterest();
        return true;
    }

    unsigned int getBalance() {
        return balance.load(); 
    }

    string getAccountName()
    {
        return this->accountName;
    }

    void addInterest()
    {
        int operations = transactionCount.fetch_add(1);
        if (operations == 100)
        {
            transactionCount.store(0);
            int oldBalance;
            do {
                oldBalance = balance.load();
            } while (!balance.compare_exchange_strong(oldBalance, (int)(oldBalance * (1 + 0.0005))));
            lock_guard<mutex> lock(m_stdOut);
            cout << "Interest rate applied" << endl;
        }
    }
};

void randomTransactions(BankAccount& from, BankAccount& to)
{
    while (true)
    {
        unsigned amount = (unsigned int)(rand() % 1000);

        if (from.withdraw(amount))
        {
            to.deposit(amount);
            lock_guard<mutex> lock(m_stdOut);
            cout << "Transferred " << amount << " from " + from.getAccountName() << " to " << to.getAccountName() << endl;
            cout << from.getAccountName() << " balance: " << from.getBalance() << endl;
            cout << to.getAccountName() << " balance: " << to.getBalance() << endl;
        }
        else
        {
            lock_guard<mutex> lock(m_stdOut);
            cout << "Insufficient balance!" << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(150));
    }
}

int main(void)
{
    srand(time(NULL));
    BankAccount account1(10000, "Account 1");
    BankAccount account2(10000, "Account 2");

    thread t1(&randomTransactions, ref(account1), ref(account2));
    thread t2(&randomTransactions, ref(account1), ref(account2));
    thread t3(&randomTransactions, ref(account2), ref(account1));
    thread t4(&randomTransactions, ref(account2), ref(account1));

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();
    if (t3.joinable())
        t3.join();
    if (t4.joinable())
        t4.join();

    return 1;
}