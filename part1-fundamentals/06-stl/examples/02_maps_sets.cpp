#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct GridPos {
    int x, y;
    bool operator==(const GridPos&) const = default;
    auto operator<=>(const GridPos&) const = default; // enables std::map / std::set
};

// Hash for unordered containers: combine the member hashes
struct GridPosHash {
    std::size_t operator()(const GridPos& p) const noexcept {
        std::size_t h1 = std::hash<int>{}(p.x);
        std::size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

int main() {
    std::string text = "the quick brown fox jumps over the lazy dog the end";

    // Word frequency: map keeps keys sorted
    std::map<std::string, int> freq;
    std::istringstream in(text);
    for (std::string w; in >> w;) ++freq[w]; // operator[] default-inserts 0
    for (const auto& [word, n] : freq) std::cout << word << ':' << n << ' ';
    std::cout << '\n';

    // Lookup without inserting
    if (auto it = freq.find("fox"); it != freq.end()) std::cout << "fox appears " << it->second << "x\n";
    std::cout << "contains 'cat'? " << std::boolalpha << freq.contains("cat") << '\n';

    // Range query on an ordered map: words in [l, q)
    std::cout << "words from 'l' to 'q': ";
    for (auto it = freq.lower_bound("l"); it != freq.lower_bound("q"); ++it) std::cout << it->first << ' ';
    std::cout << '\n';

    // unordered_map: fastest for plain lookup
    std::unordered_map<std::string, double> prices{{"apple", 0.5}, {"bread", 2.25}};
    prices.insert_or_assign("apple", 0.6);
    auto [it, inserted] = prices.try_emplace("bread", 99.0); // does nothing: key exists
    std::cout << "bread inserted? " << inserted << ", price " << it->second << '\n';

    // set: unique + sorted
    std::set<char> letters;
    for (char c : text)
        if (std::isalpha(static_cast<unsigned char>(c))) letters.insert(c);
    std::cout << "distinct letters: " << letters.size() << " (pangram? " << (letters.size() == 26) << ")\n";

    // Custom keys
    std::set<GridPos> visited{{0, 0}, {1, 0}, {0, 0}};
    std::cout << "visited cells: " << visited.size() << '\n';
    std::unordered_set<GridPos, GridPosHash> walls{{2, 3}, {4, 5}};
    std::cout << "wall at (2,3)? " << walls.contains({2, 3}) << '\n';
}
