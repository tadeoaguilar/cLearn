#include "http/json_mapping.hpp"

#include <map>
#include <set>

#include "domain/errors.hpp"

namespace tasks::http {

json to_json(const Task& t) {
    return json{
        {"id", t.id},
        {"title", t.title},
        {"description", t.description},
        {"status", to_string(t.status)},
        {"priority", to_string(t.priority)},
        {"due_date", t.due_date ? json(*t.due_date) : json(nullptr)},
        {"created_at", t.created_at},
        {"updated_at", t.updated_at},
    };
}

json to_json(const ListResult& r, const ListQuery& q) {
    json items = json::array();
    for (const auto& t : r.items) items.push_back(to_json(t));
    return json{{"items", std::move(items)}, {"total", r.total}, {"limit", q.limit}, {"offset", q.offset}};
}

json error_body(std::string_view code, std::string_view message, const json& details) {
    json err{{"code", code}, {"message", message}};
    if (!details.empty()) err["details"] = details;
    return json{{"error", std::move(err)}};
}

namespace {

// Collects every problem in the body before throwing, so the client can fix them all at once.
class Reader {
public:
    explicit Reader(const json& body) : body_{body} {
        if (!body_.is_object()) throw ValidationError("body", "must be a JSON object");
    }

    void allow_only(std::initializer_list<const char*> fields) {
        std::set<std::string> allowed(fields.begin(), fields.end());
        for (const auto& [key, _] : body_.items())
            if (!allowed.contains(key)) problems_[key] = "unknown field";
    }

    std::optional<std::string> string(const char* field, bool required) {
        auto it = body_.find(field);
        if (it == body_.end()) {
            if (required) problems_[field] = "is required";
            return std::nullopt;
        }
        if (!it->is_string()) {
            problems_[field] = "must be a string";
            return std::nullopt;
        }
        return it->get<std::string>();
    }

    template <typename E, typename Parse>
    std::optional<E> enumeration(const char* field, Parse parse, const char* allowed) {
        auto s = string(field, false);
        if (!s) return std::nullopt;
        auto v = parse(*s);
        if (!v) problems_[field] = std::string("must be one of: ") + allowed;
        return v;
    }

    // Absent → nullopt; null → optional{nullopt} (clear); string → optional{value}
    std::optional<std::optional<std::string>> nullable_string(const char* field) {
        auto it = body_.find(field);
        if (it == body_.end()) return std::nullopt;
        if (it->is_null()) return std::optional<std::string>{};
        if (!it->is_string()) {
            problems_[field] = "must be a string or null";
            return std::nullopt;
        }
        return std::optional<std::string>{it->get<std::string>()};
    }

    void finish() {
        if (!problems_.empty()) throw ValidationError(std::move(problems_));
    }

private:
    const json& body_;
    std::map<std::string, std::string> problems_;
};

constexpr const char* kStatuses = "todo, in_progress, done";
constexpr const char* kPriorities = "low, medium, high";

} // namespace

NewTask new_task_from_json(const json& body) {
    Reader r{body};
    r.allow_only({"title", "description", "status", "priority", "due_date"});
    NewTask t;
    t.title = r.string("title", true).value_or("");
    t.description = r.string("description", false).value_or("");
    t.status = r.enumeration<TaskStatus>("status", parse_status, kStatuses).value_or(TaskStatus::Todo);
    t.priority = r.enumeration<Priority>("priority", parse_priority, kPriorities).value_or(Priority::Medium);
    t.due_date = r.nullable_string("due_date").value_or(std::nullopt);
    r.finish();
    return t;
}

TaskPatch patch_from_json(const json& body) {
    Reader r{body};
    r.allow_only({"title", "description", "status", "priority", "due_date"});
    TaskPatch p;
    p.title = r.string("title", false);
    p.description = r.string("description", false);
    p.status = r.enumeration<TaskStatus>("status", parse_status, kStatuses);
    p.priority = r.enumeration<Priority>("priority", parse_priority, kPriorities);
    p.due_date = r.nullable_string("due_date");
    r.finish();
    return p;
}

} // namespace tasks::http
