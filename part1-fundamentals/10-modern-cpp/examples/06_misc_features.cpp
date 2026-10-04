#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

struct ServerConfig {
    std::string host = "0.0.0.0";
    int port = 8080;
    int threads = 4;
    bool verbose = false;
};

enum class Level { Debug, Info, Warn, Error };

std::string_view name(Level l) {
    switch (l) {
        using enum Level; // C++20: no need to write Level:: in each case
        case Debug: return "DEBUG";
        case Info:  return "INFO";
        case Warn:  return "WARN";
        case Error: return "ERROR";
    }
    return "?";
}

void log(Level lvl, std::string_view msg, std::source_location loc = std::source_location::current()) {
    std::cout << '[' << name(lvl) << "] " << loc.file_name() << ':' << loc.line() << " (" << loc.function_name()
              << "): " << msg << '\n';
}

[[nodiscard]] int compute() { return 42; }

int main() {
    // Designated initializers: name the fields; omitted ones keep their defaults
    ServerConfig cfg{.port = 9000, .verbose = true};
    std::cout << cfg.host << ':' << cfg.port << " threads=" << cfg.threads << " verbose=" << cfg.verbose << '\n';

    log(Level::Info, "server starting");
    log(Level::Warn, "low disk space");

    [[maybe_unused]] int unused_in_release = compute();

    std::cout << "underlying value of Error: " << std::to_underlying(Level::Error) << '\n';

    int x = compute();
    if (x > 0) [[likely]] {
        std::cout << "likely branch\n";
    }
}
