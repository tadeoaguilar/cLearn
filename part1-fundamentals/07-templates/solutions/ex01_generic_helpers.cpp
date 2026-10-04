#include <array>
#include <concepts>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

template <std::totally_ordered T>
const T& clamp_to(const T& v, const T& lo, const T& hi) {
    if (v < lo) return lo;
    if (hi < v) return hi;
    return v;
}

template <std::ranges::range R, typename T>
    requires std::equality_comparable_with<std::ranges::range_value_t<R>, T>
std::optional<std::size_t> find_index(const R& r, const T& value) {
    std::size_t i = 0;
    for (const auto& x : r) {
        if (x == value) return i;
        ++i;
    }
    return std::nullopt;
}

void print_all(const std::ranges::range auto& r) {
    std::cout << '[';
    bool first = true;
    for (const auto& x : r) {
        std::cout << (first ? "" : ", ") << x;
        first = false;
    }
    std::cout << "]\n";
}

int main() {
    std::cout << clamp_to(15, 0, 10) << ' ' << clamp_to(-2.5, 0.0, 1.0) << ' '
              << clamp_to(std::string{"m"}, std::string{"a"}, std::string{"k"}) << '\n';

    std::vector<std::string> names{"ada", "bjarne", "grace"};
    if (auto i = find_index(names, "grace")) std::cout << "grace at " << *i << '\n';
    if (!find_index(names, "linus")) std::cout << "linus not found\n";

    print_all(std::vector<int>{1, 2, 3});
    print_all(std::array<double, 3>{1.5, 2.5, 3.5});
    print_all(names);
    print_all(std::views::iota(1, 6));
}
