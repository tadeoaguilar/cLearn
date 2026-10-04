#include <iostream>
#include <string_view>

std::string_view trim(std::string_view s) {
    constexpr std::string_view ws = " \t\n\r";
    auto first = s.find_first_not_of(ws);
    if (first == std::string_view::npos) return {};
    auto last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

int main() {
    std::string_view config = "  host = localhost ;port=5432;  user= admin ";

    while (!config.empty()) {
        auto semi = config.find(';');
        std::string_view pair = config.substr(0, semi); // npos → rest of string
        config = (semi == std::string_view::npos) ? std::string_view{} : config.substr(semi + 1);

        auto eq = pair.find('=');
        if (eq == std::string_view::npos) continue; // skip malformed entries
        auto key = trim(pair.substr(0, eq));
        auto value = trim(pair.substr(eq + 1));
        std::cout << '[' << key << "] = [" << value << "]\n";
    }
}
