// Fundamental types, their sizes and numeric limits.
#include <cstdint>
#include <iostream>
#include <limits>

int main() {
    std::cout << "sizeof(bool)        = " << sizeof(bool) << '\n';
    std::cout << "sizeof(char)        = " << sizeof(char) << '\n';
    std::cout << "sizeof(int)         = " << sizeof(int) << '\n';
    std::cout << "sizeof(long)        = " << sizeof(long) << '\n';
    std::cout << "sizeof(long long)   = " << sizeof(long long) << '\n';
    std::cout << "sizeof(float)       = " << sizeof(float) << '\n';
    std::cout << "sizeof(double)      = " << sizeof(double) << '\n';
    std::cout << "sizeof(std::size_t) = " << sizeof(std::size_t) << '\n';
    std::cout << "sizeof(int32_t)     = " << sizeof(std::int32_t) << " (guaranteed)\n\n";

    std::cout << "int range:    " << std::numeric_limits<int>::min() << " .. "
              << std::numeric_limits<int>::max() << '\n';
    std::cout << "uint8 range:  0 .. " << +std::numeric_limits<std::uint8_t>::max()
              << "   (+ promotes the char-like type to int for printing)\n";
    std::cout << "double epsilon: " << std::numeric_limits<double>::epsilon() << "\n\n";

    // Integer division truncates toward zero.
    std::cout << "7 / 2   = " << 7 / 2 << '\n';
    std::cout << "-7 / 2  = " << -7 / 2 << '\n';
    std::cout << "7 % 3   = " << 7 % 3 << '\n';
    std::cout << "7 / 2.0 = " << 7 / 2.0 << '\n';

    // Unsigned arithmetic wraps around (well-defined, but often a bug).
    unsigned int u = 0;
    u = u - 1;
    std::cout << "0u - 1  = " << u << '\n';

    // Floating point is not exact.
    double a = 0.1 + 0.2;
    std::cout.precision(17);
    std::cout << "0.1 + 0.2 = " << a << " (== 0.3? " << std::boolalpha << (a == 0.3) << ")\n";
}
