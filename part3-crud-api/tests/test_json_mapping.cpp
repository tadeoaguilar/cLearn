#include <doctest/doctest.h>

#include "domain/errors.hpp"
#include "http/json_mapping.hpp"

using namespace tasks;
using namespace tasks::http;

TEST_CASE("Task serializes with snake_case enums and null due_date") {
    Task t{.id = 7, .title = "t", .status = TaskStatus::InProgress, .priority = Priority::High};
    auto j = to_json(t);
    CHECK(j["id"] == 7);
    CHECK(j["status"] == "in_progress");
    CHECK(j["priority"] == "high");
    CHECK(j["due_date"].is_null());
}

TEST_CASE("new_task_from_json parses a full body") {
    auto t = new_task_from_json(json::parse(R"({"title":"x","description":"d","status":"done","priority":"low","due_date":"2026-01-02"})"));
    CHECK(t.title == "x");
    CHECK(t.status == TaskStatus::Done);
    CHECK(t.priority == Priority::Low);
    CHECK(t.due_date == "2026-01-02");
}

TEST_CASE("new_task_from_json collects type errors, unknown fields and missing title") {
    try {
        new_task_from_json(json::parse(R"({"description": 5, "priority": "urgent", "colour": "red"})"));
        FAIL("expected ValidationError");
    } catch (const ValidationError& e) {
        const auto& f = e.fields();
        CHECK(f.at("title") == "is required");
        CHECK(f.at("description") == "must be a string");
        CHECK(f.at("priority").starts_with("must be one of"));
        CHECK(f.at("colour") == "unknown field");
    }
}

TEST_CASE("body must be an object") {
    CHECK_THROWS_AS(new_task_from_json(json::parse("[1,2,3]")), ValidationError);
    CHECK_THROWS_AS(patch_from_json(json::parse("\"text\"")), ValidationError);
}

TEST_CASE("patch distinguishes absent, null and value for due_date") {
    CHECK_FALSE(patch_from_json(json::parse(R"({"title":"a"})")).due_date.has_value());   // absent: don't touch
    auto cleared = patch_from_json(json::parse(R"({"due_date":null})")).due_date;
    REQUIRE(cleared.has_value());
    CHECK_FALSE(cleared->has_value());                                                      // null: clear
    auto set = patch_from_json(json::parse(R"({"due_date":"2026-05-05"})")).due_date;
    REQUIRE(set.has_value());
    CHECK(**set == "2026-05-05");                                                           // value: set
}
