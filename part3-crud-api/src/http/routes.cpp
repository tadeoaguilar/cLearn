#include "http/routes.hpp"

#include <charconv>
#include <chrono>
#include <string>

#include "domain/errors.hpp"
#include "http/json_mapping.hpp"

namespace tasks::http {

namespace {

constexpr const char* kJson = "application/json";

void send(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), kJson);
}

void send_error(httplib::Response& res, int status, std::string_view code, std::string_view message,
                const json& details = json::object()) {
    send(res, status, error_body(code, message, details));
}

TaskId parse_id(const httplib::Request& req) {
    const std::string& s = req.path_params.at("id");
    TaskId id{};
    auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), id);
    if (ec != std::errc{} || p != s.data() + s.size() || id <= 0) throw ValidationError("id", "must be a positive integer");
    return id;
}

json parse_body(const httplib::Request& req) {
    if (req.get_header_value("Content-Type").find(kJson) == std::string::npos)
        throw ValidationError("Content-Type", "must be application/json");
    return json::parse(req.body); // throws json::parse_error → 400
}

int int_param(const httplib::Request& req, const char* name, int fallback) {
    if (!req.has_param(name)) return fallback;
    std::string s = req.get_param_value(name);
    int v{};
    auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
    if (ec != std::errc{} || p != s.data() + s.size()) throw ValidationError(name, "must be an integer");
    return v;
}

ListQuery parse_list_query(const httplib::Request& req) {
    ListQuery q;
    std::map<std::string, std::string> problems;
    if (req.has_param("status")) {
        q.status = parse_status(req.get_param_value("status"));
        if (!q.status) problems["status"] = "must be one of: todo, in_progress, done";
    }
    if (req.has_param("priority")) {
        q.priority = parse_priority(req.get_param_value("priority"));
        if (!q.priority) problems["priority"] = "must be one of: low, medium, high";
    }
    if (req.has_param("q")) q.search = req.get_param_value("q");
    if (req.has_param("sort")) {
        auto s = parse_sort_field(req.get_param_value("sort"));
        if (s) q.sort = *s;
        else problems["sort"] = "must be one of: created_at, updated_at, due_date, priority, title";
    }
    if (req.has_param("order")) {
        auto o = req.get_param_value("order");
        if (o == "asc") q.descending = false;
        else if (o == "desc") q.descending = true;
        else problems["order"] = "must be asc or desc";
    }
    if (!problems.empty()) throw ValidationError(std::move(problems));
    q.limit = int_param(req, "limit", q.limit);
    q.offset = int_param(req, "offset", q.offset);
    return q;
}

// Every handler runs inside this wrapper, which turns exceptions into HTTP responses.
// This is the ONLY place that knows how domain errors map to status codes.
template <typename Handler>
httplib::Server::Handler guarded(Logger& logger, Handler handler) {
    return [&logger, handler](const httplib::Request& req, httplib::Response& res) {
        try {
            handler(req, res);
        } catch (const ValidationError& e) {
            json details = json::object();
            for (const auto& [field, problem] : e.fields()) details[field] = problem;
            send_error(res, 422, "validation_failed", "the request contains invalid fields", details);
        } catch (const json::parse_error& e) {
            send_error(res, 400, "malformed_json", e.what());
        } catch (const NotFoundError& e) {
            send_error(res, 404, "not_found", e.what());
        } catch (const StorageUnavailableError& e) {
            logger.error("storage unavailable: {}", e.what());
            send_error(res, 503, "unavailable", "storage is temporarily unavailable, retry later");
        } catch (const std::exception& e) {
            logger.error("unhandled error on {} {}: {}", req.method, req.path, e.what());
            send_error(res, 500, "internal_error", "an unexpected error occurred"); // never leak internals
        }
    };
}

thread_local std::chrono::steady_clock::time_point t_request_start; // one request per worker thread at a time

} // namespace

void register_routes(httplib::Server& server, TaskService& service, Logger& logger) {
    server.Get("/health", guarded(logger, [&service](const httplib::Request&, httplib::Response& res) {
        service.repository().ping();
        send(res, 200, json{{"status", "ok"}, {"storage", service.repository().name()}});
    }));

    server.Get("/tasks", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        auto query = parse_list_query(req);
        auto result = service.list(query);
        send(res, 200, to_json(result, query));
    }));

    server.Post("/tasks", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        auto task = service.create(new_task_from_json(parse_body(req)));
        res.set_header("Location", "/tasks/" + std::to_string(task.id));
        send(res, 201, to_json(task));
    }));

    server.Get("/tasks/:id", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        send(res, 200, to_json(service.get(parse_id(req))));
    }));

    server.Put("/tasks/:id", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        auto id = parse_id(req);
        send(res, 200, to_json(service.replace(id, new_task_from_json(parse_body(req)))));
    }));

    server.Patch("/tasks/:id", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        auto id = parse_id(req);
        send(res, 200, to_json(service.patch(id, patch_from_json(parse_body(req)))));
    }));

    server.Delete("/tasks/:id", guarded(logger, [&service](const httplib::Request& req, httplib::Response& res) {
        service.remove(parse_id(req));
        res.status = 204; // No Content
    }));

    // Unknown routes / methods: give JSON instead of an empty body.
    server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        if (!res.body.empty()) return httplib::Server::HandlerResponse::Unhandled; // a handler already wrote an error
        if (res.status == 404) send_error(res, 404, "not_found", "no such route");
        else if (res.status == 405) send_error(res, 405, "method_not_allowed", "method not allowed on this route");
        return httplib::Server::HandlerResponse::Handled;
    });

    // Last-resort safety net (exceptions thrown outside our handlers).
    server.set_exception_handler([&logger](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            logger.error("exception escaped handler for {} {}: {}", req.method, req.path, e.what());
        } catch (...) {
            logger.error("unknown exception for {} {}", req.method, req.path);
        }
        send_error(res, 500, "internal_error", "an unexpected error occurred");
    });

    // Access log with latency. pre_routing runs on the same worker thread as the logger callback.
    server.set_pre_routing_handler([](const httplib::Request&, httplib::Response&) {
        t_request_start = std::chrono::steady_clock::now();
        return httplib::Server::HandlerResponse::Unhandled;
    });
    server.set_logger([&logger](const httplib::Request& req, const httplib::Response& res) {
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t_request_start);
        logger.info("{} {} -> {} ({:.2f} ms)", req.method, req.path, res.status, static_cast<double>(us.count()) / 1000.0);
    });
}

} // namespace tasks::http
