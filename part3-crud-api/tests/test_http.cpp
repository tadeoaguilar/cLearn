// End-to-end tests: a real HTTP server on a random local port, talked to by a
// real HTTP client, backed by the in-memory repository.
#include <doctest/doctest.h>

#include <thread>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "http/routes.hpp"
#include "repository/memory_task_repository.hpp"

using namespace tasks;
using json = nlohmann::json;

namespace {

class TestServer {
public:
    TestServer() {
        http::register_routes(server_, service_, logger_);
        port_ = server_.bind_to_any_port("127.0.0.1");
        thread_ = std::jthread([this] { server_.listen_after_bind(); });
        server_.wait_until_ready();
    }
    ~TestServer() { server_.stop(); }

    httplib::Client client() const {
        httplib::Client c("127.0.0.1", port_);
        c.set_connection_timeout(2);
        return c;
    }

private:
    Logger logger_{LogLevel::Error}; // keep test output quiet
    MemoryTaskRepository repo_;
    TaskService service_{repo_};
    httplib::Server server_;
    int port_{};
    std::jthread thread_;
};

json body(const httplib::Result& r) { return json::parse(r->body); }

} // namespace

TEST_CASE("HTTP: full CRUD lifecycle") {
    TestServer srv;
    auto cli = srv.client();

    auto health = cli.Get("/health");
    REQUIRE(health);
    CHECK(health->status == 200);
    CHECK(body(health)["storage"] == "memory");

    auto created = cli.Post("/tasks", R"({"title":"Learn C++","priority":"high"})", "application/json");
    REQUIRE(created);
    CHECK(created->status == 201);
    auto task = body(created);
    auto id = task["id"].get<std::int64_t>();
    CHECK(created->get_header_value("Location") == "/tasks/" + std::to_string(id));
    CHECK(task["status"] == "todo");

    auto got = cli.Get("/tasks/" + std::to_string(id));
    REQUIRE(got);
    CHECK(got->status == 200);
    CHECK(body(got) == task);

    auto patched = cli.Patch("/tasks/" + std::to_string(id), R"({"status":"done"})", "application/json");
    REQUIRE(patched);
    CHECK(patched->status == 200);
    CHECK(body(patched)["status"] == "done");
    CHECK(body(patched)["priority"] == "high");

    auto replaced = cli.Put("/tasks/" + std::to_string(id), R"({"title":"Master C++"})", "application/json");
    REQUIRE(replaced);
    CHECK(body(replaced)["title"] == "Master C++");
    CHECK(body(replaced)["priority"] == "medium"); // PUT replaces everything

    auto list = cli.Get("/tasks");
    REQUIRE(list);
    CHECK(body(list)["total"] == 1);

    auto deleted = cli.Delete("/tasks/" + std::to_string(id));
    REQUIRE(deleted);
    CHECK(deleted->status == 204);
    CHECK(cli.Get("/tasks/" + std::to_string(id))->status == 404);
}

TEST_CASE("HTTP: errors are JSON with proper status codes") {
    TestServer srv;
    auto cli = srv.client();

    auto invalid = cli.Post("/tasks", R"({"title":""})", "application/json");
    REQUIRE(invalid);
    CHECK(invalid->status == 422);
    CHECK(body(invalid)["error"]["code"] == "validation_failed");
    CHECK(body(invalid)["error"]["details"].contains("title"));

    auto malformed = cli.Post("/tasks", "{not json", "application/json");
    CHECK(malformed->status == 400);
    CHECK(body(malformed)["error"]["code"] == "malformed_json");

    auto wrong_type = cli.Post("/tasks", "title=x", "application/x-www-form-urlencoded");
    CHECK(wrong_type->status == 422);

    auto missing = cli.Get("/tasks/999");
    CHECK(missing->status == 404);
    CHECK(body(missing)["error"]["code"] == "not_found");

    auto bad_id = cli.Get("/tasks/abc");
    CHECK(bad_id->status == 422);

    auto bad_query = cli.Get("/tasks?status=sleeping&limit=500");
    CHECK(bad_query->status == 422);

    auto no_route = cli.Get("/nope");
    CHECK(no_route->status == 404);
    CHECK(body(no_route)["error"]["code"] == "not_found");
}

TEST_CASE("HTTP: list query parameters") {
    TestServer srv;
    auto cli = srv.client();
    for (const char* t : {R"({"title":"a","priority":"low"})", R"({"title":"b","priority":"high"})",
                          R"({"title":"c","priority":"high","status":"done"})"})
        REQUIRE(cli.Post("/tasks", t, "application/json")->status == 201);

    CHECK(body(cli.Get("/tasks?priority=high"))["total"] == 2);
    CHECK(body(cli.Get("/tasks?priority=high&status=done"))["total"] == 1);
    auto page = body(cli.Get("/tasks?sort=title&order=asc&limit=2&offset=1"));
    CHECK(page["items"].size() == 2);
    CHECK(page["items"][0]["title"] == "b");
    CHECK(page["limit"] == 2);
    CHECK(page["offset"] == 1);
}

TEST_CASE("HTTP: concurrent clients") {
    TestServer srv;
    {
        std::vector<std::jthread> clients;
        for (int i = 0; i < 8; ++i) {
            clients.emplace_back([&srv, i] {
                auto cli = srv.client();
                for (int j = 0; j < 10; ++j)
                    cli.Post("/tasks", json{{"title", "t" + std::to_string(i) + "-" + std::to_string(j)}}.dump(), "application/json");
            });
        }
    }
    CHECK(body(srv.client().Get("/tasks?limit=1"))["total"] == 80);
}
