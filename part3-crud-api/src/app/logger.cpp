#include "app/logger.hpp"

#include <chrono>
#include <iostream>

namespace tasks {

void Logger::log(LogLevel level, std::string_view msg) {
    if (level < min_) return;
    static constexpr std::string_view names[] = {"DEBUG", "INFO ", "WARN ", "ERROR"};
    auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
    // Format outside the lock; only the write is serialized.
    std::string line = std::format("{:%FT%TZ} {} {}\n", now, names[static_cast<int>(level)], msg);
    std::lock_guard lock(mutex_);
    std::cerr << line;
}

LogLevel parse_log_level(std::string_view s) {
    if (s == "debug") return LogLevel::Debug;
    if (s == "warn") return LogLevel::Warn;
    if (s == "error") return LogLevel::Error;
    return LogLevel::Info;
}

} // namespace tasks
