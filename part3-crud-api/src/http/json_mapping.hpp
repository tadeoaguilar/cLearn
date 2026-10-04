// Converting between JSON (the wire format) and domain types.
// Parsing is strict: wrong types and unknown fields become ValidationErrors,
// so clients get precise feedback instead of silently ignored typos.
#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "domain/task.hpp"

namespace tasks::http {

using json = nlohmann::json;

json to_json(const Task& t);
json to_json(const ListResult& r, const ListQuery& q);

NewTask new_task_from_json(const json& body); // for POST (and PUT, where title is required too)
TaskPatch patch_from_json(const json& body);  // for PATCH

// {"error": {"code": ..., "message": ..., "details": {...}}}
json error_body(std::string_view code, std::string_view message, const json& details = json::object());

} // namespace tasks::http
