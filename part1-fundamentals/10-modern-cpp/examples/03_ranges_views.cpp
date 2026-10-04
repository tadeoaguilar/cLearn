#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace views = std::views;

int main() {
    // Lazy infinite sequence, filtered and transformed — only 5 values ever computed
    for (int x : views::iota(1) | views::filter([](int n) { return n % 3 == 0; })
                                | views::transform([](int n) { return n * n; })
                                | views::take(5))
        std::cout << x << ' ';
    std::cout << '\n';

    std::vector<int> data{5, 1, 4, 2, 8, 7, 3};
    for (int x : data | views::drop(2) | views::reverse) std::cout << x << ' ';
    std::cout << '\n';

    for (int x : data | views::take_while([](int n) { return n != 8; })) std::cout << x << ' ';
    std::cout << '\n';

    // keys/values on maps
    std::map<std::string, int> stock{{"apple", 3}, {"pear", 0}, {"kiwi", 12}};
    for (const auto& name : stock | views::filter([](const auto& kv) { return kv.second > 0; }) | views::keys)
        std::cout << name << ' ';
    std::cout << '\n';

    // split a string into words
    std::string_view sentence = "ranges make pipelines readable";
    for (auto word : sentence | views::split(' ')) std::cout << '[' << std::string_view(word) << ']';
    std::cout << '\n';

    // C++23 views. Standard libraries roll these out at different speeds, so we
    // use feature-test macros (from <version>, pulled in by <ranges>) to check.
    std::vector<std::string> names{"ada", "bjarne", "grace"};
#if defined(__cpp_lib_ranges_enumerate)
    for (auto [i, name] : views::enumerate(names)) std::cout << i << ':' << name << ' ';
#else
    for (auto [i, name] : views::zip(views::iota(0), names)) std::cout << i << ':' << name << ' '; // portable fallback
#endif
    std::cout << '\n';

    std::vector<int> ages{36, 73, 85};
    for (auto [name, age] : views::zip(names, ages)) std::cout << name << '=' << age << ' ';
    std::cout << '\n';

#if defined(__cpp_lib_ranges_chunk) && defined(__cpp_lib_ranges_slide)
    for (auto chunk : views::iota(1, 10) | views::chunk(3)) {
        std::cout << '{';
        for (int x : chunk) std::cout << x;
        std::cout << "} ";
    }
    std::cout << '\n';
    for (auto window : data | views::slide(3)) {
        int sum = 0;
        for (int x : window) sum += x;
        std::cout << sum << ' ';
    }
    std::cout << "(sliding window sums)\n";
#else
    std::cout << "(views::chunk / views::slide not available in this standard library yet)\n";
#endif

    auto evens = views::iota(1, 20) | views::filter([](int n) { return n % 2 == 0; }) | std::ranges::to<std::vector>();
    std::cout << "materialized " << evens.size() << " evens into a vector\n";
}
