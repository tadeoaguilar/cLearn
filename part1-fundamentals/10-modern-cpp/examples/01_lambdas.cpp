#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

std::function<int()> make_counter() {
    int count = 0;
    // [&count] here would DANGLE once make_counter returns. Capture by value + mutable:
    return [count]() mutable { return ++count; };
}

int main() {
    // Basic, with explicit return type
    auto divide = [](double a, double b) -> double { return b == 0 ? 0 : a / b; };
    std::cout << "divide: " << divide(10, 4) << '\n';

    // Capture by value vs by reference
    int factor = 2;
    auto by_value = [factor](int x) { return x * factor; };
    auto by_ref = [&factor](int x) { return x * factor; };
    factor = 10;
    std::cout << "by_value(3)=" << by_value(3) << " by_ref(3)=" << by_ref(3) << '\n'; // 6 vs 30

    // mutable state
    auto c1 = make_counter();
    auto c2 = make_counter();
    std::cout << "counters: " << c1() << c1() << c1() << " / " << c2() << '\n';

    // Init-capture: move a unique_ptr into the lambda
    auto ptr = std::make_unique<std::string>("owned by lambda");
    auto owner = [p = std::move(ptr)] { return *p; };
    std::cout << owner() << '\n';

    // Generic lambda (auto params) and C++20 template lambda
    auto print = [](const auto& x) { std::cout << x << ' '; };
    print(1); print(2.5); print("three"); std::cout << '\n';

    auto first_or_default = []<typename T>(const std::vector<T>& v) { return v.empty() ? T{} : v.front(); };
    std::cout << "first_or_default: " << first_or_default(std::vector<int>{}) << ", "
              << first_or_default(std::vector<std::string>{"x"}) << '\n';

    // Lambdas with algorithms: sort by multiple keys
    struct Card { std::string suit; int rank; };
    std::vector<Card> hand{{"hearts", 10}, {"spades", 3}, {"hearts", 2}, {"clubs", 10}};
    std::ranges::sort(hand, [](const Card& a, const Card& b) {
        return a.rank != b.rank ? a.rank > b.rank : a.suit < b.suit;
    });
    for (const auto& [suit, rank] : hand) std::cout << rank << " of " << suit << "; ";
    std::cout << '\n';

    // IIFE: immediately-invoked lambda to initialize a const with complex logic
    const std::string level = [&] {
        if (factor > 5) return "high";
        return "low";
    }();
    std::cout << "level: " << level << '\n';

    // Recursive lambda (C++23 "deducing this")
    auto fact = [](this auto self, int n) -> long long { return n <= 1 ? 1 : n * self(n - 1); };
    std::cout << "10! = " << fact(10) << '\n';
}
