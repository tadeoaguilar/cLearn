#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main() {
    const std::string text =
        "It was the best of times, it was the worst of times, it was the age of wisdom, "
        "it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity.";

    std::unordered_map<std::string, int> counts;
    std::string word;
    auto flush = [&] {
        if (!word.empty()) ++counts[word];
        word.clear();
    };
    for (char ch : text) {
        auto c = static_cast<unsigned char>(ch);
        if (std::isalpha(c)) word += static_cast<char>(std::tolower(c));
        else flush();
    }
    flush();

    std::vector<std::pair<std::string, int>> entries(counts.begin(), counts.end());
    const std::size_t n = std::min<std::size_t>(5, entries.size());
    std::partial_sort(entries.begin(), entries.begin() + static_cast<std::ptrdiff_t>(n), entries.end(),
                      [](const auto& a, const auto& b) {
                          if (a.second != b.second) return a.second > b.second; // count desc
                          return a.first < b.first;                              // then alphabetical
                      });

    for (std::size_t i = 0; i < n; ++i) std::cout << i + 1 << ". " << entries[i].first << " (" << entries[i].second << ")\n";
}
