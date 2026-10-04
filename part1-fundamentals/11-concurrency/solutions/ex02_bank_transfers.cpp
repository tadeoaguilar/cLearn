#include <array>
#include <atomic>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

struct Account {
    std::mutex m;
    int balance = 1000;
};

std::array<Account, 5> accounts;
std::atomic<int> done{0}, skipped{0};

void transfer(Account& from, Account& to, int amount) {
    if (&from == &to) return;
    std::scoped_lock lock(from.m, to.m); // locks both with a deadlock-avoidance algorithm
    if (from.balance < amount) {
        ++skipped;
        return;
    }
    from.balance -= amount;
    to.balance += amount;
    ++done;
}

int total() {
    int sum = 0;
    for (auto& a : accounts) {
        std::lock_guard lock(a.m);
        sum += a.balance;
    }
    return sum;
}

int main() {
    std::cout << "total before: " << total() << '\n';
    {
        std::vector<std::jthread> workers;
        for (int t = 0; t < 8; ++t) {
            workers.emplace_back([t] {
                std::mt19937 rng(static_cast<unsigned>(t));
                std::uniform_int_distribution<std::size_t> acc(0, accounts.size() - 1);
                std::uniform_int_distribution<int> amt(1, 300);
                for (int i = 0; i < 10'000; ++i) transfer(accounts[acc(rng)], accounts[acc(rng)], amt(rng));
            });
        }
    }
    std::cout << "transfers done: " << done << ", skipped (insufficient funds): " << skipped << '\n';
    for (std::size_t i = 0; i < accounts.size(); ++i) std::cout << "  account " << i << ": " << accounts[i].balance << '\n';
    std::cout << "total after:  " << total() << " (must equal before)\n";
}
