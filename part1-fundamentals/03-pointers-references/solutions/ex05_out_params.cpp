#include <charconv>
#include <iostream>
#include <string_view>

bool parse_int(std::string_view s, int& out) {
    int value{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return false; // error or trailing junk
    out = value;
    return true;
}

bool parse_int(std::string_view s, int* out) {
    int tmp{};
    if (!parse_int(s, tmp)) return false;
    if (out) *out = tmp; // nullptr → caller only wanted validation
    return true;
}

// Why std::optional<int> parse_int(std::string_view) is nicer:
//  * the result and the success flag can't get out of sync,
//  * callers can't forget to check (or read an unset `out`),
//  * it composes: `if (auto n = parse_int(s)) use(*n);`

int main() {
    for (std::string_view s : {"42", "-17", "12abc", "", "99999999999"}) {
        int value = 0;
        if (parse_int(s, value))
            std::cout << '"' << s << "\" -> " << value << '\n';
        else
            std::cout << '"' << s << "\" -> invalid\n";
    }
    std::cout << "validate-only \"123\": " << std::boolalpha << parse_int("123", nullptr) << '\n';
}
