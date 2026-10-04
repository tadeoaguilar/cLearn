#include "service/task_service.hpp"

#include <map>
#include <string>

namespace tasks {

namespace {

using Problems = std::map<std::string, std::string>;

void check_title(std::string& title, Problems& problems) {
    title = trim(title);
    if (title.empty()) problems["title"] = "must not be empty";
    else if (title.size() > kMaxTitle) problems["title"] = "must be at most " + std::to_string(kMaxTitle) + " characters";
}

void check_description(std::string& description, Problems& problems) {
    description = trim(description);
    if (description.size() > kMaxDescription)
        problems["description"] = "must be at most " + std::to_string(kMaxDescription) + " characters";
}

void check_due_date(const std::optional<std::string>& due, Problems& problems) {
    if (due && !is_valid_date(*due)) problems["due_date"] = "must be a valid date in YYYY-MM-DD format";
}

} // namespace

void validate(NewTask& t) {
    Problems problems;
    check_title(t.title, problems);
    check_description(t.description, problems);
    check_due_date(t.due_date, problems);
    if (!problems.empty()) throw ValidationError(std::move(problems));
}

void validate(TaskPatch& p) {
    Problems problems;
    if (p.empty()) problems["body"] = "at least one field must be provided";
    if (p.title) check_title(*p.title, problems);
    if (p.description) check_description(*p.description, problems);
    if (p.due_date) check_due_date(*p.due_date, problems);
    if (!problems.empty()) throw ValidationError(std::move(problems));
}

void validate(ListQuery& q) {
    Problems problems;
    if (q.limit < 1 || q.limit > kMaxPageSize) problems["limit"] = "must be between 1 and " + std::to_string(kMaxPageSize);
    if (q.offset < 0) problems["offset"] = "must be >= 0";
    if (q.search) {
        *q.search = trim(*q.search);
        if (q.search->empty()) q.search.reset();
        else if (q.search->size() > 100) problems["q"] = "must be at most 100 characters";
    }
    if (!problems.empty()) throw ValidationError(std::move(problems));
}

Task TaskService::create(NewTask input) {
    validate(input);
    return repo_.create(input);
}

Task TaskService::get(TaskId id) {
    if (auto t = repo_.find(id)) return *t;
    throw NotFoundError("task " + std::to_string(id) + " not found");
}

ListResult TaskService::list(ListQuery query) {
    validate(query);
    return repo_.list(query);
}

Task TaskService::replace(TaskId id, NewTask input) {
    validate(input);
    TaskPatch all{input.title, input.description, input.status, input.priority, input.due_date};
    if (auto t = repo_.update(id, all)) return *t;
    throw NotFoundError("task " + std::to_string(id) + " not found");
}

Task TaskService::patch(TaskId id, TaskPatch patch) {
    validate(patch);
    if (auto t = repo_.update(id, patch)) return *t;
    throw NotFoundError("task " + std::to_string(id) + " not found");
}

void TaskService::remove(TaskId id) {
    if (!repo_.remove(id)) throw NotFoundError("task " + std::to_string(id) + " not found");
}

} // namespace tasks
