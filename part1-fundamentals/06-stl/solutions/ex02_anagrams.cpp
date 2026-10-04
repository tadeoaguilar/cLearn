#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> words{"listen", "silent", "enlist", "google", "gooegl", "cat", "act", "tac", "dog"};

    std::map<std::string, std::vector<std::string>> groups;
    for (const auto& w : words) {
        std::string key = w;
        std::ranges::sort(key);
        groups[key].push_back(w);
    }

    std::vector<std::vector<std::string>> sorted;
    for (auto& [key, members] : groups) sorted.push_back(std::move(members));
    std::ranges::stable_sort(sorted, std::ranges::greater{}, &std::vector<std::string>::size);

    for (const auto& g : sorted) {
        std::cout << '[';
        for (std::size_t i = 0; i < g.size(); ++i) std::cout << g[i] << (i + 1 < g.size() ? ", " : "");
        std::cout << "]\n";
    }
}
