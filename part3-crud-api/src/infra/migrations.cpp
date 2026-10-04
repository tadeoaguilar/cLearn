#include "infra/migrations.hpp"

#include <string>
#include <vector>

namespace tasks::infra {

namespace {

struct Migration {
    int version;
    const char* name;
    const char* sql;
};

// APPEND-ONLY. Never edit a migration that has been deployed; add a new one.
const std::vector<Migration> kMigrations{
    {1, "create_tasks", R"sql(
        CREATE TABLE tasks (
            id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
            title       TEXT NOT NULL CHECK (length(title) BETWEEN 1 AND 200),
            description TEXT NOT NULL DEFAULT '' CHECK (length(description) <= 2000),
            status      TEXT NOT NULL DEFAULT 'todo'   CHECK (status IN ('todo', 'in_progress', 'done')),
            priority    TEXT NOT NULL DEFAULT 'medium' CHECK (priority IN ('low', 'medium', 'high')),
            due_date    DATE,
            created_at  TIMESTAMPTZ NOT NULL DEFAULT now(),
            updated_at  TIMESTAMPTZ NOT NULL DEFAULT now()
        ))sql"},
    {2, "index_tasks_status_created", R"sql(
        CREATE INDEX tasks_status_created_idx ON tasks (status, created_at DESC))sql"},
    {3, "priority_rank_for_sorting", R"sql(
        -- Sorting by the TEXT priority would be alphabetical (high < low < medium).
        -- A generated column gives a numeric rank we can sort and index on.
        ALTER TABLE tasks ADD COLUMN priority_rank SMALLINT GENERATED ALWAYS AS (
            CASE priority WHEN 'low' THEN 0 WHEN 'medium' THEN 1 ELSE 2 END) STORED)sql"},
};

} // namespace

void run_migrations(pqxx::connection& conn, Logger& logger) {
    {
        pqxx::work tx{conn};
        tx.exec(R"(CREATE TABLE IF NOT EXISTS schema_migrations (
                     version    INT PRIMARY KEY,
                     name       TEXT NOT NULL,
                     applied_at TIMESTAMPTZ NOT NULL DEFAULT now()))");
        tx.commit();
    }
    for (const auto& m : kMigrations) {
        pqxx::work tx{conn};
        tx.exec("SELECT pg_advisory_xact_lock(727274)"); // serialize concurrent migrators
        if (tx.query_value<bool>("SELECT EXISTS (SELECT 1 FROM schema_migrations WHERE version = $1)", pqxx::params{m.version}))
            continue;
        tx.exec(m.sql);
        tx.exec("INSERT INTO schema_migrations(version, name) VALUES ($1, $2)", pqxx::params{m.version, m.name});
        tx.commit();
        logger.info("applied migration {:03}_{}", m.version, m.name);
    }
}

} // namespace tasks::infra
