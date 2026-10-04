// The Task entity and the value types used to create, modify and query tasks.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tasks {

enum class TaskStatus { Todo, InProgress, Done };
enum class Priority { Low, Medium, High };

std::string_view to_string(TaskStatus s);
std::string_view to_string(Priority p);
std::optional<TaskStatus> parse_status(std::string_view s);
std::optional<Priority> parse_priority(std::string_view s);

using TaskId = std::int64_t;

struct Task {
    TaskId id{};
    std::string title;
    std::string description;
    TaskStatus status{TaskStatus::Todo};
    Priority priority{Priority::Medium};
    std::optional<std::string> due_date; // "YYYY-MM-DD"
    std::string created_at;              // ISO 8601 UTC, e.g. "2026-10-03T14:00:00Z"
    std::string updated_at;

    bool operator==(const Task&) const = default;
};

// Input for POST /tasks
struct NewTask {
    std::string title;
    std::string description;
    TaskStatus status{TaskStatus::Todo};
    Priority priority{Priority::Medium};
    std::optional<std::string> due_date;
};

// Input for PATCH /tasks/{id}: only the fields that are set change.
// due_date is doubly optional: outer = "was it sent?", inner = "set or clear (null)".
struct TaskPatch {
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<TaskStatus> status;
    std::optional<Priority> priority;
    std::optional<std::optional<std::string>> due_date;

    bool empty() const { return !title && !description && !status && !priority && !due_date; }
};

enum class SortField { CreatedAt, UpdatedAt, DueDate, Priority, Title };
std::optional<SortField> parse_sort_field(std::string_view s);

// Input for GET /tasks
struct ListQuery {
    std::optional<TaskStatus> status;
    std::optional<Priority> priority;
    std::optional<std::string> search; // case-insensitive substring of title or description
    SortField sort{SortField::CreatedAt};
    bool descending{true};
    int limit{20};
    int offset{0};
};

struct ListResult {
    std::vector<Task> items;
    std::int64_t total{}; // matching tasks ignoring limit/offset (for pagination UIs)
};

// ---- validation rules (pure functions, easy to unit test) ----
inline constexpr std::size_t kMaxTitle = 200;
inline constexpr std::size_t kMaxDescription = 2000;
inline constexpr int kMaxPageSize = 100;

bool is_valid_date(std::string_view yyyy_mm_dd);
std::string trim(std::string_view s);

} // namespace tasks
