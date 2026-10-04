// Transactions: atomic transfers, automatic rollback, constraints, row locks, retries.
#include <chrono>
#include <iostream>
#include <pqxx/pqxx>
#include <stdexcept>
#include <thread>

#include "db_common.hpp"

void show(pqxx::connection& conn) {
    pqxx::read_transaction tx{conn};
    for (auto [name, balance] : tx.query<std::string, int>("SELECT name, balance FROM demo_accounts ORDER BY name"))
        std::cout << "    " << name << ": " << balance << '\n';
}

// Moves money atomically. The CHECK constraint makes overdrafts impossible.
void transfer(pqxx::connection& conn, const std::string& from, const std::string& to, int amount) {
    pqxx::work tx{conn};
    tx.exec("UPDATE demo_accounts SET balance = balance - $1 WHERE name = $2", pqxx::params{amount, from});
    tx.exec("UPDATE demo_accounts SET balance = balance + $1 WHERE name = $2", pqxx::params{amount, to});
    tx.commit();
}

// Runs `body` in a SERIALIZABLE transaction, retrying on serialization failures.
template <typename F>
void with_retry(pqxx::connection& conn, F body, int max_attempts = 5) {
    for (int attempt = 1;; ++attempt) {
        try {
            pqxx::transaction<pqxx::isolation_level::serializable> tx{conn};
            body(tx);
            tx.commit();
            return;
        } catch (const pqxx::serialization_failure&) {
            if (attempt == max_attempts) throw;
            std::cout << "    serialization conflict, retrying (attempt " << attempt + 1 << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * attempt));
        }
    }
}

int main() {
    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS demo_accounts");
        tx.exec("CREATE TABLE demo_accounts (name TEXT PRIMARY KEY, balance INT NOT NULL CHECK (balance >= 0))");
        tx.exec("INSERT INTO demo_accounts VALUES ('alice', 100), ('bob', 50)");
        tx.commit();
    }
    std::cout << "initial:\n";
    show(conn);

    std::cout << "1) transfer 30 alice -> bob (commits):\n";
    transfer(conn, "alice", "bob", 30);
    show(conn);

    std::cout << "2) transfer 500 alice -> bob (violates CHECK, whole transaction rolled back):\n";
    try {
        transfer(conn, "alice", "bob", 500);
    } catch (const pqxx::check_violation& e) {
        std::cout << "    check_violation: " << e.sqlstate() << '\n';
    }
    show(conn);

    std::cout << "3) exception in our own code before commit -> automatic rollback:\n";
    try {
        pqxx::work tx{conn};
        tx.exec("UPDATE demo_accounts SET balance = 0 WHERE name = 'bob'");
        throw std::runtime_error("something went wrong in C++");
        // tx.commit(); never reached -> ~work() rolls back
    } catch (const std::exception& e) {
        std::cout << "    " << e.what() << '\n';
    }
    show(conn);

    std::cout << "4) SELECT ... FOR UPDATE: read-modify-write without lost updates\n";
    {
        pqxx::work tx{conn};
        // The row stays locked until commit: concurrent transactions wanting it must wait.
        int bal = tx.query_value<int>("SELECT balance FROM demo_accounts WHERE name = 'bob' FOR UPDATE");
        tx.exec("UPDATE demo_accounts SET balance = $1 WHERE name = 'bob'", pqxx::params{bal + 10});
        tx.commit();
    }
    show(conn);

    std::cout << "5) serializable transaction with retry helper:\n";
    with_retry(conn, [](auto& tx) {
        int total = tx.template query_value<int>("SELECT sum(balance) FROM demo_accounts");
        tx.exec("INSERT INTO demo_accounts VALUES ('audit', $1)", pqxx::params{total / 100});
    });
    show(conn);
}
