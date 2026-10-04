#include <doctest/doctest.h>

#include "domain/errors.hpp"
#include "repository/memory_task_repository.hpp"
#include "service/task_service.hpp"

using namespace tasks;

namespace {
struct Fixture {
    MemoryTaskRepository repo;
    TaskService service{repo};
};
} // namespace

TEST_CASE_FIXTURE(Fixture, "create trims input and applies defaults") {
    auto t = service.create({.title = "  Write tests  ", .description = " soon "});
    CHECK(t.id > 0);
    CHECK(t.title == "Write tests");
    CHECK(t.description == "soon");
    CHECK(t.status == TaskStatus::Todo);
    CHECK(t.priority == Priority::Medium);
    CHECK(t.created_at == t.updated_at);
}

TEST_CASE_FIXTURE(Fixture, "create reports every invalid field at once") {
    NewTask bad{.title = "   ", .description = std::string(kMaxDescription + 1, 'x'), .due_date = "2026-02-30"};
    try {
        service.create(bad);
        FAIL("expected ValidationError");
    } catch (const ValidationError& e) {
        CHECK(e.fields().size() == 3);
        CHECK(e.fields().contains("title"));
        CHECK(e.fields().contains("description"));
        CHECK(e.fields().contains("due_date"));
    }
}

TEST_CASE_FIXTURE(Fixture, "title length limit") {
    CHECK_NOTHROW(service.create({.title = std::string(kMaxTitle, 'a')}));
    CHECK_THROWS_AS(service.create({.title = std::string(kMaxTitle + 1, 'a')}), ValidationError);
}

TEST_CASE_FIXTURE(Fixture, "get / remove of missing tasks throw NotFoundError") {
    CHECK_THROWS_AS(service.get(42), NotFoundError);
    CHECK_THROWS_AS(service.remove(42), NotFoundError);
    CHECK_THROWS_AS(service.patch(42, TaskPatch{.status = TaskStatus::Done}), NotFoundError);
}

TEST_CASE_FIXTURE(Fixture, "patch changes only the given fields") {
    auto t = service.create({.title = "Original", .priority = Priority::Low, .due_date = "2026-12-01"});
    auto p = service.patch(t.id, {.status = TaskStatus::Done});
    CHECK(p.status == TaskStatus::Done);
    CHECK(p.title == "Original");
    CHECK(p.priority == Priority::Low);
    CHECK(p.due_date == "2026-12-01");

    auto cleared = service.patch(t.id, {.due_date = std::optional<std::string>{}});
    CHECK_FALSE(cleared.due_date.has_value());
}

TEST_CASE_FIXTURE(Fixture, "empty patch is rejected") {
    auto t = service.create({.title = "x"});
    CHECK_THROWS_AS(service.patch(t.id, {}), ValidationError);
}

TEST_CASE_FIXTURE(Fixture, "replace overwrites every field") {
    auto t = service.create({.title = "a", .description = "d", .priority = Priority::High, .due_date = "2026-01-01"});
    auto r = service.replace(t.id, {.title = "b"});
    CHECK(r.title == "b");
    CHECK(r.description.empty());
    CHECK(r.priority == Priority::Medium);
    CHECK_FALSE(r.due_date.has_value());
}

TEST_CASE_FIXTURE(Fixture, "list validates paging parameters") {
    CHECK_THROWS_AS(service.list({.limit = 0}), ValidationError);
    CHECK_THROWS_AS(service.list({.limit = kMaxPageSize + 1}), ValidationError);
    CHECK_THROWS_AS(service.list({.offset = -1}), ValidationError);
    CHECK_NOTHROW(service.list({.limit = kMaxPageSize}));
}

TEST_CASE_FIXTURE(Fixture, "blank search is ignored") {
    service.create({.title = "one"});
    service.create({.title = "two"});
    CHECK(service.list({.search = "   "}).total == 2);
}
