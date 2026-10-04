#include <array>
#include <cstdint>
#include <iostream>
#include <string_view>

constexpr std::uint64_t fib(int n) {
    std::uint64_t a = 0, b = 1;
    for (int i = 0; i < n; ++i) {
        auto t = a + b;
        a = b;
        b = t;
    }
    return a;
}

// A whole lookup table computed at compile time and baked into the binary
constexpr auto make_square_table() {
    std::array<int, 16> t{};
    for (int i = 0; i < 16; ++i) t[static_cast<std::size_t>(i)] = i * i;
    return t;
}
constexpr auto kSquares = make_square_table();

// consteval: must be evaluated at compile time
consteval std::uint32_t hash(std::string_view s) { // FNV-1a
    std::uint32_t h = 2166136261u;
    for (char c : s) {
        h ^= static_cast<std::uint8_t>(c);
        h *= 16777619u;
    }
    return h;
}

constexpr bool is_prime(int n) {
    if (n < 2) return false;
    for (int d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int main() {
    static_assert(fib(10) == 55);
    static_assert(kSquares[7] == 49);
    static_assert(is_prime(97) && !is_prime(91));

    constexpr auto f50 = fib(50);
    std::cout << "fib(50) = " << f50 << " (computed by the compiler)\n";

    // Compile-time string hashes are handy for switch-like dispatch on strings
    constexpr auto kJump = hash("jump");
    switch (hash("jump")) {
        case kJump: std::cout << "jump command matched via compile-time hash\n"; break;
        case hash("run"): std::cout << "run\n"; break;
        default: break;
    }

    int runtime_n = 20;
    std::cout << "fib(" << runtime_n << ") at runtime = " << fib(runtime_n) << " (constexpr functions work at runtime too)\n";
    // hash(std::string_view{some_runtime_string}); // ERROR: consteval requires a constant
}
