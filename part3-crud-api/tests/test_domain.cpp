#include <doctest/doctest.h>

#include "domain/task.hpp"

using namespace tasks;

TEST_CASE("status and priority round-trip through strings") {
    for (auto s : {TaskStatus::Todo, TaskStatus::InProgress, TaskStatus::Done}) CHECK(parse_status(to_string(s)) == s);
    for (auto p : {Priority::Low, Priority::Medium, Priority::High}) CHECK(parse_priority(to_string(p)) == p);
    CHECK_FALSE(parse_status("DONE").has_value()); // case-sensitive on purpose
    CHECK_FALSE(parse_priority("urgent").has_value());
}

TEST_CASE("is_valid_date accepts only real YYYY-MM-DD dates") {
    CHECK(is_valid_date("2026-10-03"));
    CHECK(is_valid_date("2024-02-29")); // leap year
    CHECK_FALSE(is_valid_date("2026-02-29"));
    CHECK_FALSE(is_valid_date("2026-13-01"));
    CHECK_FALSE(is_valid_date("2026-1-01"));
    CHECK_FALSE(is_valid_date("03/10/2026"));
    CHECK_FALSE(is_valid_date(""));
    CHECK_FALSE(is_valid_date("2026-10-0x"));
}

TEST_CASE("trim") {
    CHECK(trim("  hi  ") == "hi");
    CHECK(trim("\t\n") == "");
    CHECK(trim("a b") == "a b");
}

TEST_CASE("TaskPatch::empty") {
    TaskPatch p;
    CHECK(p.empty());
    p.due_date = std::optional<std::string>{}; // "clear the due date" is a real change
    CHECK_FALSE(p.empty());
}
