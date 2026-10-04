#include <charconv>
#include <expected>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

using Config = std::map<std::string, std::string, std::less<>>; // less<> allows string_view lookups

struct ConfigError {
    int line;
    std::string message;
};

std::string_view trim(std::string_view s) {
    auto b = s.find_first_not_of(" \t\r");
    if (b == std::string_view::npos) return {};
    auto e = s.find_last_not_of(" \t\r");
    return s.substr(b, e - b + 1);
}

std::expected<Config, ConfigError> parse_config(const std::string& text) {
    Config cfg;
    std::istringstream in(text);
    std::string raw;
    int line_no = 0;
    while (std::getline(in, raw)) {
        ++line_no;
        auto line = trim(raw);
        if (line.empty() || line.front() == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string_view::npos) return std::unexpected(ConfigError{line_no, "expected key = value"});
        auto key = trim(line.substr(0, eq));
        if (key.empty()) return std::unexpected(ConfigError{line_no, "empty key"});
        cfg.insert_or_assign(std::string(key), std::string(trim(line.substr(eq + 1))));
    }
    return cfg;
}

std::expected<int, ConfigError> get_int(const Config& cfg, std::string_view key) {
    auto it = cfg.find(key);
    if (it == cfg.end()) return std::unexpected(ConfigError{0, "missing key '" + std::string(key) + "'"});
    int value{};
    const auto& s = it->second;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size())
        return std::unexpected(ConfigError{0, "key '" + std::string(key) + "' is not an integer: " + s});
    return value;
}

void report(const ConfigError& e) {
    std::cout << "  error" << (e.line ? " on line " + std::to_string(e.line) : "") << ": " << e.message << '\n';
}

int main() {
    const std::string good = "# server settings\nhost = localhost\nport=8080\n\nworkers = four\n";
    const std::string bad = "host = localhost\nthis line is broken\n";

    std::cout << "good config:\n";
    if (auto cfg = parse_config(good)) {
        for (const auto& [k, v] : *cfg) std::cout << "  " << k << " -> " << v << '\n';
        for (auto key : {"port", "workers", "timeout"}) {
            if (auto n = get_int(*cfg, key)) std::cout << "  " << key << " as int = " << *n << '\n';
            else report(n.error());
        }
    }

    std::cout << "bad config:\n";
    if (auto cfg = parse_config(bad); !cfg) report(cfg.error());
}
