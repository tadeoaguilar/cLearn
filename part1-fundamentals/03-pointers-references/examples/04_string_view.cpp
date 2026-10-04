#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Accepts literals, std::string and substrings without copying.
std::size_t count_char(std::string_view text, char c) {
    std::size_t n = 0;
    for (char ch : text)
        if (ch == c) ++n;
    return n;
}

// Splits into views that point INTO the original string: zero allocations
// for the pieces themselves (the vector still allocates).
std::vector<std::string_view> split(std::string_view text, char delim) {
    std::vector<std::string_view> parts;
    while (true) {
        auto pos = text.find(delim);
        parts.push_back(text.substr(0, pos));
        if (pos == std::string_view::npos) break;
        text.remove_prefix(pos + 1);
    }
    return parts;
}

int main() {
    std::string csv = "name,age,city,country";
    std::cout << "commas: " << count_char(csv, ',') << '\n';
    std::cout << "a's in literal: " << count_char("banana", 'a') << '\n';

    for (auto part : split(csv, ',')) std::cout << '[' << part << "] ";
    std::cout << '\n';

    // string_view::substr is O(1); std::string::substr allocates a copy
    std::string_view sv = csv;
    std::cout << "first field: " << sv.substr(0, sv.find(',')) << '\n';

    // Converting back to an owning string when you need to keep it
    std::string owned{sv.substr(5, 3)};
    std::cout << "owned copy: " << owned << '\n';
}
