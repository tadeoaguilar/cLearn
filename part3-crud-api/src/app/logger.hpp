// A tiny thread-safe logger: one line per message to stderr,
// "2026-10-03T14:05:09.123Z INFO  message". Good enough for containers,
// where stdout/stderr are collected by the runtime.
#pragma once

#include <format>
#include <mutex>
#include <string>
#include <string_view>

namespace tasks {

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
public:
    explicit Logger(LogLevel min = LogLevel::Info) : min_{min} {}

    void log(LogLevel level, std::string_view msg);

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) { log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...)); }
    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) { log(LogLevel::Warn, std::format(fmt, std::forward<Args>(args)...)); }
    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) { log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...)); }
    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) { log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...)); }

    void set_level(LogLevel level) { min_ = level; }

private:
    std::mutex mutex_;
    LogLevel min_;
};

LogLevel parse_log_level(std::string_view s); // "debug" | "info" | "warn" | "error"

} // namespace tasks
