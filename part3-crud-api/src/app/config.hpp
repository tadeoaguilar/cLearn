// Runtime configuration from environment variables (12-factor style),
// so the same binary runs locally, in Docker and in production.
#pragma once

#include <string>

namespace tasks {

struct Config {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string storage = "memory";   // "memory" | "postgres"
    std::string database_url;         // used when storage == "postgres"
    int db_pool_size = 8;
    int http_threads = 8;
    std::string log_level = "info";
};

// Reads HOST, PORT, STORAGE, DATABASE_URL, DB_POOL_SIZE, HTTP_THREADS, LOG_LEVEL.
// If STORAGE is unset it defaults to "postgres" when DATABASE_URL is set, else "memory".
// Throws std::invalid_argument for malformed values.
Config load_config_from_env();

} // namespace tasks
