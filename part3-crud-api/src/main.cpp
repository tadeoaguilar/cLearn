// Composition root: read config, build the object graph, start the server.
// This is the only file that knows about every concrete type.
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include <httplib.h>

#include "app/config.hpp"
#include "app/logger.hpp"
#include "http/routes.hpp"
#include "repository/memory_task_repository.hpp"
#include "service/task_service.hpp"

#ifdef TASKS_WITH_POSTGRES
#include "infra/connection_pool.hpp"
#include "infra/migrations.hpp"
#include "repository/pg_task_repository.hpp"
#endif

#if !defined(_WIN32)
#include <pthread.h>
#endif

using namespace tasks;

int main() {
    Logger logger;
    Config config;
    try {
        config = load_config_from_env();
    } catch (const std::exception& e) {
        logger.error("configuration error: {}", e.what());
        return EXIT_FAILURE;
    }
    logger.set_level(parse_log_level(config.log_level));

#if !defined(_WIN32)
    // Block SIGINT/SIGTERM in all threads (they inherit this mask); a dedicated
    // thread waits for them with sigwait and stops the server cleanly. That's
    // safer than doing work inside an async signal handler.
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &signals, nullptr);
#endif

    // ---- storage ----
    std::unique_ptr<TaskRepository> repo;
#ifdef TASKS_WITH_POSTGRES
    std::unique_ptr<infra::ConnectionPool> pool;
#endif
    try {
        if (config.storage == "postgres") {
#ifdef TASKS_WITH_POSTGRES
            pool = std::make_unique<infra::ConnectionPool>(config.database_url, static_cast<std::size_t>(config.db_pool_size));
            {
                auto conn = pool->acquire();
                infra::run_migrations(*conn, logger);
            }
            repo = std::make_unique<PgTaskRepository>(*pool);
#else
            logger.error("this binary was built without PostgreSQL support (TASKS_WITH_POSTGRES=OFF)");
            return EXIT_FAILURE;
#endif
        } else {
            repo = std::make_unique<MemoryTaskRepository>();
        }
    } catch (const std::exception& e) {
        logger.error("storage initialization failed: {}", e.what());
        return EXIT_FAILURE;
    }

    TaskService service{*repo};

    // ---- HTTP ----
    httplib::Server server;
    const auto threads = static_cast<std::size_t>(config.http_threads);
    server.new_task_queue = [threads] { return new httplib::ThreadPool(threads); };
    server.set_payload_max_length(1024 * 1024); // 1 MiB request bodies are plenty for tasks
    http::register_routes(server, service, logger);

#if !defined(_WIN32)
    std::jthread signal_waiter([&] {
        int sig = 0;
        sigwait(&signals, &sig);
        logger.info("received signal {}, shutting down", sig);
        server.stop(); // listen() returns; in-flight requests finish first
    });
#endif

    logger.info("tasks_api listening on http://{}:{} (storage={}, http_threads={})", config.host, config.port,
                repo->name(), config.http_threads);
    if (!server.listen(config.host, config.port)) {
        logger.error("could not listen on {}:{}", config.host, config.port);
#if !defined(_WIN32)
        pthread_kill(signal_waiter.native_handle(), SIGTERM); // unblock the waiter so it can be joined
#endif
        return EXIT_FAILURE;
    }
    logger.info("bye");
    return EXIT_SUCCESS;
}
