#include "repository/memory_task_repository.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <format>
#include <mutex>
#include <ranges>

namespace tasks {

std::string now_iso8601() {
    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%FT%TZ}", now);
}

namespace {

std::string lower(std::string_view s) {
    std::string out(s);
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool matches(const Task& t, const ListQuery& q, const std::string& needle) {
    if (q.status && t.status != *q.status) return false;
    if (q.priority && t.priority != *q.priority) return false;
    if (!needle.empty() && lower(t.title).find(needle) == std::string::npos &&
        lower(t.description).find(needle) == std::string::npos)
        return false;
    return true;
}

// Strict-weak "less than" for the requested sort field. Ties fall back to id so
// that pagination is stable, and tasks without a due date sort last (like SQL NULLS LAST).
bool less_by(const Task& a, const Task& b, SortField f) {
    switch (f) {
        case SortField::CreatedAt: if (a.created_at != b.created_at) return a.created_at < b.created_at; break;
        case SortField::UpdatedAt: if (a.updated_at != b.updated_at) return a.updated_at < b.updated_at; break;
        case SortField::Title:     if (a.title != b.title) return a.title < b.title; break;
        case SortField::Priority:  if (a.priority != b.priority) return a.priority < b.priority; break;
        case SortField::DueDate:   if (a.due_date != b.due_date) return a.due_date < b.due_date; break;
    }
    return a.id < b.id;
}

} // namespace

Task MemoryTaskRepository::create(const NewTask& in) {
    std::unique_lock lock(mutex_);
    Task t;
    t.id = next_id_++;
    t.title = in.title;
    t.description = in.description;
    t.status = in.status;
    t.priority = in.priority;
    t.due_date = in.due_date;
    t.created_at = t.updated_at = now_iso8601();
    tasks_.emplace(t.id, t);
    return t;
}

std::optional<Task> MemoryTaskRepository::find(TaskId id) {
    std::shared_lock lock(mutex_);
    if (auto it = tasks_.find(id); it != tasks_.end()) return it->second;
    return std::nullopt;
}

ListResult MemoryTaskRepository::list(const ListQuery& q) {
    std::vector<Task> matching;
    {
        std::shared_lock lock(mutex_);
        const std::string needle = q.search ? lower(*q.search) : std::string{};
        for (const auto& [id, t] : tasks_)
            if (matches(t, q, needle)) matching.push_back(t);
    } // sort outside the lock: we own the copies

    std::ranges::sort(matching, [&](const Task& a, const Task& b) {
        // Missing due dates go last in both directions
        if (q.sort == SortField::DueDate && a.due_date.has_value() != b.due_date.has_value()) return a.due_date.has_value();
        return q.descending ? less_by(b, a, q.sort) : less_by(a, b, q.sort);
    });

    ListResult result;
    result.total = static_cast<std::int64_t>(matching.size());
    auto page = matching | std::views::drop(q.offset) | std::views::take(q.limit);
    result.items.assign(page.begin(), page.end());
    return result;
}

std::optional<Task> MemoryTaskRepository::update(TaskId id, const TaskPatch& p) {
    std::unique_lock lock(mutex_);
    auto it = tasks_.find(id);
    if (it == tasks_.end()) return std::nullopt;
    Task& t = it->second;
    if (p.title) t.title = *p.title;
    if (p.description) t.description = *p.description;
    if (p.status) t.status = *p.status;
    if (p.priority) t.priority = *p.priority;
    if (p.due_date) t.due_date = *p.due_date;
    t.updated_at = now_iso8601();
    return t;
}

bool MemoryTaskRepository::remove(TaskId id) {
    std::unique_lock lock(mutex_);
    return tasks_.erase(id) == 1;
}

} // namespace tasks
