// std::expected<T, E> (C++23): return a value or a descriptive error.
#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

enum class ParseError { Empty, NotANumber, OutOfRange };

std::string_view to_string(ParseError e) {
    switch (e) {
        case ParseError::Empty:      return "empty input";
        case ParseError::NotANumber: return "not a number";
        case ParseError::OutOfRange: return "out of range";
    }
    return "?";
}

std::expected<int, ParseError> parse_int(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int value{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec == std::errc::result_out_of_range) return std::unexpected(ParseError::OutOfRange);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return std::unexpected(ParseError::NotANumber);
    return value;
}

// Validation with a richer error type
struct ValidationError {
    std::string field;
    std::string message;
};

std::expected<int, ValidationError> parse_age(std::string_view s) {
    auto n = parse_int(s);
    if (!n) return std::unexpected(ValidationError{"age", std::string(to_string(n.error()))});
    if (*n < 0 || *n > 150) return std::unexpected(ValidationError{"age", "must be between 0 and 150"});
    return *n;
}

int main() {
    for (std::string_view s : {"42", "", "abc", "99999999999", "12x"}) {
        auto r = parse_int(s);
        if (r) std::cout << '"' << s << "\" -> " << *r << '\n';
        else   std::cout << '"' << s << "\" -> error: " << to_string(r.error()) << '\n';
    }

    for (std::string_view s : {"36", "-4", "old"}) {
        auto age = parse_age(s);
        if (age) std::cout << "age ok: " << *age << '\n';
        else     std::cout << "invalid " << age.error().field << ": " << age.error().message << '\n';
    }

    // Chaining: parse, then double, then format — errors short-circuit automatically
    auto result = parse_int("21")
                      .transform([](int x) { return x * 2; })
                      .transform([](int x) { return "answer = " + std::to_string(x); });
    std::cout << result.value_or("failed") << '\n';
}
