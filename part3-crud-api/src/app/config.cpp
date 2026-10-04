#include "app/config.hpp"

#include <charconv>
#include <cstdlib>
#include <stdexcept>
#include <string_view>

namespace tasks {

namespace {

std::string env(const char* name, std::string fallback = {}) {
    const char* v = std::getenv(name);
    return (v && *v) ? std::string(v) : fallback;
}

int env_int(const char* name, int fallback, int min, int max) {
    std::string s = env(name);
    if (s.empty()) return fallback;
    int value{};
    auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || p != s.data() + s.size() || value < min || value > max)
        throw std::invalid_argument(std::string(name) + " must be an integer in [" + std::to_string(min) + ", " +
                                    std::to_string(max) + "], got '" + s + "'");
    return value;
}

} // namespace

Config load_config_from_env() {
    Config c;
    c.host = env("HOST", c.host);
    c.port = env_int("PORT", c.port, 1, 65535);
    c.database_url = env("DATABASE_URL");
    c.storage = env("STORAGE", c.database_url.empty() ? "memory" : "postgres");
    if (c.storage != "memory" && c.storage != "postgres")
        throw std::invalid_argument("STORAGE must be 'memory' or 'postgres', got '" + c.storage + "'");
    if (c.storage == "postgres" && c.database_url.empty())
        throw std::invalid_argument("STORAGE=postgres requires DATABASE_URL");
    c.db_pool_size = env_int("DB_POOL_SIZE", c.db_pool_size, 1, 256);
    c.http_threads = env_int("HTTP_THREADS", c.http_threads, 1, 1024);
    c.log_level = env("LOG_LEVEL", c.log_level);
    return c;
}

} // namespace tasks
