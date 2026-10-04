// SQL injection (and how parameters prevent it), prepared statements, batching.
#include <chrono>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "db_common.hpp"

template <typename F>
long long ms(F f) {
    auto t = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count();
}

int main() {
    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS demo_users");
        tx.exec("CREATE TABLE demo_users (id SERIAL PRIMARY KEY, name TEXT NOT NULL, secret TEXT NOT NULL)");
        tx.exec("INSERT INTO demo_users(name, secret) VALUES ('ada', 'ada-secret'), ('bob', 'bob-secret')");
        tx.commit();
    }

    const std::string attacker_input = "nobody' OR '1'='1";

    std::cout << "1) UNSAFE string concatenation:\n";
    {
        pqxx::read_transaction tx{conn};
        std::string sql = "SELECT name, secret FROM demo_users WHERE name = '" + attacker_input + "'";
        std::cout << "   SQL sent: " << sql << '\n';
        for (auto [name, secret] : tx.query<std::string, std::string>(sql))
            std::cout << "   LEAKED: " << name << " / " << secret << '\n';
    }

    std::cout << "2) SAFE parameter ($1):\n";
    {
        pqxx::read_transaction tx{conn};
        auto r = tx.exec("SELECT name, secret FROM demo_users WHERE name = $1", pqxx::params{attacker_input});
        std::cout << "   rows returned: " << r.size() << " (the input is just a weird name)\n";
    }

    std::cout << "3) Prepared statement, executed many times:\n";
    conn.prepare("insert_user", "INSERT INTO demo_users(name, secret) VALUES ($1, $2)");
    constexpr int N = 2000;

    auto one_tx_per_row = ms([&] {
        for (int i = 0; i < N / 10; ++i) { // only N/10: this is the slow way
            pqxx::work tx{conn};
            tx.exec(pqxx::prepped{"insert_user"}, pqxx::params{"u" + std::to_string(i), "s"});
            tx.commit(); // a commit per row = a disk flush per row
        }
    });
    auto single_tx = ms([&] {
        pqxx::work tx{conn};
        for (int i = 0; i < N; ++i) tx.exec(pqxx::prepped{"insert_user"}, pqxx::params{"v" + std::to_string(i), "s"});
        tx.commit();
    });
    std::cout << "   " << N / 10 << " rows, one transaction each: " << one_tx_per_row << " ms\n";
    std::cout << "   " << N << " rows, one transaction total:  " << single_tx << " ms\n";

    std::cout << "4) ANY($1) with an array instead of N separate queries (avoids N+1):\n";
    {
        pqxx::read_transaction tx{conn};
        std::vector<std::string> names{"ada", "bob", "v42"};
        auto n = tx.query_value<int>("SELECT count(*) FROM demo_users WHERE name = ANY($1)", pqxx::params{names});
        std::cout << "   matched " << n << " users in one round trip\n";
    }
}
