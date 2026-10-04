#include <atomic>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <thread>
#include <vector>

#include "db_common.hpp"

constexpr int kStock = 10, kBuyers = 8, kQty = 3;

void reset() {
    pqxx::connection conn{db::database_url()};
    pqxx::work tx{conn};
    tx.exec("DROP TABLE IF EXISTS ex_products");
    tx.exec("CREATE TABLE ex_products (id INT PRIMARY KEY, stock INT NOT NULL)"); // no CHECK on purpose
    tx.exec("INSERT INTO ex_products VALUES (1, $1)", pqxx::params{kStock});
    tx.commit();
}

// Naive: read, decide in C++, write back an absolute value. Classic lost update.
bool buy_naive(pqxx::connection& c) {
    pqxx::work tx{c};
    int stock = tx.query_value<int>("SELECT stock FROM ex_products WHERE id = 1");
    if (stock < kQty) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // widen the race window
    tx.exec("UPDATE ex_products SET stock = $1 WHERE id = 1", pqxx::params{stock - kQty});
    tx.commit();
    return true;
}

// Correct A: let the database do the check-and-decrement atomically.
bool buy_atomic(pqxx::connection& c) {
    pqxx::work tx{c};
    auto r = tx.exec("UPDATE ex_products SET stock = stock - $1 WHERE id = 1 AND stock >= $1", pqxx::params{kQty});
    tx.commit();
    return r.affected_rows() == 1;
}

// Correct B: lock the row, then read-modify-write inside the same transaction.
bool buy_locked(pqxx::connection& c) {
    pqxx::work tx{c};
    int stock = tx.query_value<int>("SELECT stock FROM ex_products WHERE id = 1 FOR UPDATE");
    if (stock < kQty) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    tx.exec("UPDATE ex_products SET stock = $1 WHERE id = 1", pqxx::params{stock - kQty});
    tx.commit();
    return true;
}

template <typename F>
void run(const char* label, F buy) {
    reset();
    std::atomic<int> sold{0};
    {
        std::vector<std::jthread> buyers;
        for (int i = 0; i < kBuyers; ++i) {
            buyers.emplace_back([&] {
                pqxx::connection conn{db::database_url()}; // connections are NOT thread-safe: one each
                if (buy(conn)) sold += kQty;
            });
        }
    }
    pqxx::connection conn{db::database_url()};
    pqxx::read_transaction tx{conn};
    int final_stock = tx.query_value<int>("SELECT stock FROM ex_products WHERE id = 1");
    std::cout << label << ": sold " << sold << " units, final stock " << final_stock
              << (sold > kStock ? "   <-- OVERSOLD!" : "   ok") << '\n';
}

int main() {
    std::cout << "stock " << kStock << ", " << kBuyers << " buyers x " << kQty << " units\n";
    run("naive  ", buy_naive);
    run("atomic ", buy_atomic);
    run("locked ", buy_locked);
}
