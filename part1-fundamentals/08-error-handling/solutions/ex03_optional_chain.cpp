#include <format>
#include <iostream>
#include <map>
#include <optional>
#include <string>

const std::map<std::string, std::map<std::string, int>> population{
    {"Mexico", {{"Mexico City", 9'209'944}, {"Guadalajara", 1'385'629}}},
    {"Japan", {{"Tokyo", 13'960'000}, {"Osaka", 2'750'000}}},
};

std::optional<int> population_of(const std::string& country, const std::string& city) {
    auto c = population.find(country);
    if (c == population.end()) return std::nullopt;
    auto t = c->second.find(city);
    if (t == c->second.end()) return std::nullopt;
    return t->second;
}

int main() {
    const std::pair<std::string, std::string> queries[] = {
        {"Mexico", "Guadalajara"}, {"Japan", "Tokyo"}, {"Japan", "Kyoto"}, {"Peru", "Lima"}};

    for (const auto& [country, city] : queries) {
        std::cout << city << ", " << country << ": " << population_of(country, city).value_or(-1) << '\n';
        std::string pretty = population_of(country, city)
                                 .transform([](int n) { return std::format("{:.2f}M people", n / 1e6); })
                                 .value_or("unknown");
        std::cout << "   -> " << pretty << '\n';
    }
}
