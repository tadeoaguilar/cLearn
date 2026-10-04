// Connect, print server info, run a few simple queries.
// Start the DB first:  docker compose up -d db
#include <iostream>
#include <pqxx/pqxx>

#include "db_common.hpp"

int main() {
    try {
        pqxx::connection conn{db::database_url()};
        std::cout << "connected to database '" << conn.dbname() << "' as '" << conn.username() << "'\n";
        std::cout << "server version: " << conn.server_version() << '\n';

        pqxx::read_transaction tx{conn}; // read-only: the server rejects writes
        std::cout << tx.query_value<std::string>("SELECT version()") << "\n\n";

        auto now = tx.query_value<std::string>("SELECT now()::text");
        std::cout << "server time: " << now << '\n';

        // A query returning several rows and columns, with typed iteration
        for (auto [n, square] : tx.query<int, int>("SELECT n, n * n FROM generate_series(1, 5) AS n"))
            std::cout << n << "^2 = " << square << '\n';
    } catch (const pqxx::broken_connection& e) {
        std::cerr << "cannot connect: " << e.what() << "\nIs the database running? docker compose up -d db\n";
        return 1;
    }
}
