// The same behavioral contract runs against every TaskRepository implementation.
// The PostgreSQL variant runs only when TEST_DATABASE_URL is set, e.g.
//   TEST_DATABASE_URL=postgresql://clearn:clearn@localhost:5432/clearn ./tasks_tests
// WARNING: it deletes all rows from the `tasks` table of that database.
#include <doctest/doctest.h>

#include <cstdlib>
#include <memory>
#include <set>

#include "repository/memory_task_repository.hpp"

#ifdef TASKS_WITH_POSTGRES
#include "app/logger.hpp"
#include "infra/connection_pool.hpp"
#include "infra/migrations.hpp"
#include "repository/pg_task_repository.hpp"
#endif

using namespace tasks;

namespace {

void run_contract(TaskRepository& repo) {
    SUBCASE("create assigns unique ids and timestamps") {
        auto a = repo.create({.title = "a"});
        auto b = repo.create({.title = "b"});
        CHECK(a.id != b.id);
        CHECK(a.created_at.size() == 20); // "YYYY-MM-DDTHH:MM:SSZ"
        CHECK(a.created_at.back() == 'Z');
    }
    SUBCASE("find returns what create returned") {
        auto a = repo.create({.title = "find me", .description = "d", .status = TaskStatus::InProgress,
                              .priority = Priority::High, .due_date = "2026-03-04"});
        auto found = repo.find(a.id);
        REQUIRE(found.has_value());
        CHECK(*found == a);
        CHECK_FALSE(repo.find(a.id + 100000).has_value());
    }
    SUBCASE("update applies a patch and reports missing ids") {
        auto a = repo.create({.title = "before", .due_date = "2026-01-01"});
        auto u = repo.update(a.id, {.title = "after", .due_date = std::optional<std::string>{}});
        REQUIRE(u.has_value());
        CHECK(u->title == "after");
        CHECK_FALSE(u->due_date.has_value());
        CHECK(u->priority == a.priority);
        CHECK_FALSE(repo.update(a.id + 100000, {.title = "x"}).has_value());
    }
    SUBCASE("remove") {
        auto a = repo.create({.title = "bye"});
        CHECK(repo.remove(a.id));
        CHECK_FALSE(repo.find(a.id).has_value());
        CHECK_FALSE(repo.remove(a.id));
    }
    SUBCASE("list filters, searches, sorts and paginates") {
        repo.create({.title = "Buy milk", .status = TaskStatus::Done, .priority = Priority::Low});
        repo.create({.title = "Write report", .description = "quarterly MILK numbers", .priority = Priority::High});
        repo.create({.title = "Call mom", .priority = Priority::Medium, .due_date = "2026-01-10"});
        repo.create({.title = "Fix bug", .status = TaskStatus::InProgress, .priority = Priority::High, .due_date = "2026-01-05"});

        CHECK(repo.list({.status = TaskStatus::Done}).total == 1);
        CHECK(repo.list({.priority = Priority::High}).total == 2);
        CHECK(repo.list({.search = "milk"}).total == 2); // title OR description, case-insensitive
        CHECK(repo.list({.search = "100%"}).total == 0); // '%' is literal, not a wildcard

        auto by_priority = repo.list({.sort = SortField::Priority, .descending = true});
        REQUIRE(by_priority.items.size() == 4);
        CHECK(by_priority.items.front().priority == Priority::High);
        CHECK(by_priority.items.back().priority == Priority::Low);

        auto by_due = repo.list({.sort = SortField::DueDate, .descending = false});
        CHECK(by_due.items[0].title == "Fix bug");
        CHECK(by_due.items[1].title == "Call mom");
        CHECK_FALSE(by_due.items[3].due_date.has_value()); // NULLs last

        auto p1 = repo.list({.sort = SortField::Title, .descending = false, .limit = 3, .offset = 0});
        auto p2 = repo.list({.sort = SortField::Title, .descending = false, .limit = 3, .offset = 3});
        CHECK(p1.total == 4);
        CHECK(p1.items.size() == 3);
        CHECK(p2.items.size() == 1);
        std::set<TaskId> ids;
        for (const auto& t : p1.items) ids.insert(t.id);
        for (const auto& t : p2.items) ids.insert(t.id);
        CHECK(ids.size() == 4);
    }
}

} // namespace

TEST_CASE("TaskRepository contract: MemoryTaskRepository") {
    MemoryTaskRepository repo;
    run_contract(repo);
}

#ifdef TASKS_WITH_POSTGRES
TEST_CASE("TaskRepository contract: PgTaskRepository") {
    const char* url = std::getenv("TEST_DATABASE_URL");
    if (!url || !*url) {
        MESSAGE("TEST_DATABASE_URL not set; skipping PostgreSQL contract tests");
        return;
    }
    Logger logger{LogLevel::Warn};
    infra::ConnectionPool pool{url, 2};
    {
        auto c = pool.acquire();
        infra::run_migrations(*c, logger);
        pqxx::work tx{*c};
        tx.exec("TRUNCATE tasks RESTART IDENTITY");
        tx.commit();
    }
    PgTaskRepository repo{pool};
    run_contract(repo);
    // doctest re-enters this test case once per SUBCASE, so the table is truncated before each one.
}
#endif
