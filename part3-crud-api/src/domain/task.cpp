#include "domain/task.hpp"

#include <chrono>
#include <charconv>

namespace tasks {

std::string_view to_string(TaskStatus s) {
    switch (s) {
        case TaskStatus::Todo: return "todo";
        case TaskStatus::InProgress: return "in_progress";
        case TaskStatus::Done: return "done";
    }
    return "todo";
}

std::string_view to_string(Priority p) {
    switch (p) {
        case Priority::Low: return "low";
        case Priority::Medium: return "medium";
        case Priority::High: return "high";
    }
    return "medium";
}

std::optional<TaskStatus> parse_status(std::string_view s) {
    if (s == "todo") return TaskStatus::Todo;
    if (s == "in_progress") return TaskStatus::InProgress;
    if (s == "done") return TaskStatus::Done;
    return std::nullopt;
}

std::optional<Priority> parse_priority(std::string_view s) {
    if (s == "low") return Priority::Low;
    if (s == "medium") return Priority::Medium;
    if (s == "high") return Priority::High;
    return std::nullopt;
}

std::optional<SortField> parse_sort_field(std::string_view s) {
    if (s == "created_at") return SortField::CreatedAt;
    if (s == "updated_at") return SortField::UpdatedAt;
    if (s == "due_date") return SortField::DueDate;
    if (s == "priority") return SortField::Priority;
    if (s == "title") return SortField::Title;
    return std::nullopt;
}

bool is_valid_date(std::string_view s) {
    // Strict "YYYY-MM-DD", and a real calendar date (no 2026-02-30).
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    auto num = [&](std::size_t pos, std::size_t len, int& out) {
        auto [p, ec] = std::from_chars(s.data() + pos, s.data() + pos + len, out);
        return ec == std::errc{} && p == s.data() + pos + len;
    };
    int y{}, m{}, d{};
    if (!num(0, 4, y) || !num(5, 2, m) || !num(8, 2, d)) return false;
    using namespace std::chrono;
    return year_month_day{year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}}.ok();
}

std::string trim(std::string_view s) {
    constexpr std::string_view ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string_view::npos) return {};
    auto e = s.find_last_not_of(ws);
    return std::string(s.substr(b, e - b + 1));
}

} // namespace tasks
