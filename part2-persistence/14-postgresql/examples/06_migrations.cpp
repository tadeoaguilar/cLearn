// A minimal migration runner: ordered, versioned schema changes, each applied
// exactly once inside a transaction, recorded in schema_migrations.
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "db_common.hpp"

struct Migration {
    int version;
    std::string name;
    std::string sql;
};

// Append-only list: NEVER edit a migration after it has run somewhere. Add a new one.
const std::vector<Migration> kMigrations{
    {1, "create_notes", R"(CREATE TABLE demo_notes (
                               id   BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                               body TEXT NOT NULL))"},
    {2, "add_created_at", "ALTER TABLE demo_notes ADD COLUMN created_at TIMESTAMPTZ NOT NULL DEFAULT now()"},
    {3, "add_pinned_with_index", R"(ALTER TABLE demo_notes ADD COLUMN pinned BOOLEAN NOT NULL DEFAULT false;
                                    CREATE INDEX demo_notes_pinned_idx ON demo_notes(pinned) WHERE pinned)"},
};

int migrate(pqxx::connection& conn, const std::vector<Migration>& migrations) {
    {
        pqxx::work tx{conn};
        tx.exec(R"(CREATE TABLE IF NOT EXISTS demo_schema_migrations (
                     version    INT PRIMARY KEY,
                     name       TEXT NOT NULL,
                     applied_at TIMESTAMPTZ NOT NULL DEFAULT now()))");
        tx.commit();
    }
    int applied = 0;
    for (const auto& m : migrations) {
        pqxx::work tx{conn};
        // Advisory lock: if two app instances start at once, only one migrates at a time.
        tx.exec("SELECT pg_advisory_xact_lock(4242)");
        auto done = tx.query_value<bool>("SELECT EXISTS(SELECT 1 FROM demo_schema_migrations WHERE version = $1)",
                                         pqxx::params{m.version});
        if (done) continue;
        std::cout << "  applying " << m.version << "_" << m.name << '\n';
        tx.exec(m.sql);
        tx.exec("INSERT INTO demo_schema_migrations(version, name) VALUES ($1, $2)", pqxx::params{m.version, m.name});
        tx.commit(); // schema change + bookkeeping are atomic (PostgreSQL has transactional DDL!)
        ++applied;
    }
    return applied;
}

int main() {
    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS demo_notes, demo_schema_migrations");
        tx.commit();
    }

    std::cout << "first run (only migrations 1-2 exist yet):\n";
    std::vector<Migration> v1(kMigrations.begin(), kMigrations.begin() + 2);
    int n = migrate(conn, v1); // call first: migrate() prints, so don't nest it inside a << chain
    std::cout << "  applied " << n << '\n';

    std::cout << "second run (same list): idempotent\n";
    n = migrate(conn, v1);
    std::cout << "  applied " << n << '\n';

    std::cout << "third run (a new release adds migration 3):\n";
    n = migrate(conn, kMigrations);
    std::cout << "  applied " << n << '\n';

    pqxx::read_transaction tx{conn};
    std::cout << "history:\n";
    for (auto [v, name, at] : tx.query<int, std::string, std::string>(
             "SELECT version, name, applied_at::text FROM demo_schema_migrations ORDER BY version"))
        std::cout << "  " << v << " " << name << " @ " << at << '\n';
    std::cout << "columns of demo_notes:";
    for (auto [col] : tx.query<std::string>(
             "SELECT column_name FROM information_schema.columns WHERE table_name = 'demo_notes' ORDER BY ordinal_position"))
        std::cout << ' ' << col;
    std::cout << '\n';
}
