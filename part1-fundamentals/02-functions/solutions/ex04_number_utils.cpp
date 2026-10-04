#include <iostream>

#include "number_utils.hpp"

static_assert(nu::gcd(12, 18) == 6);
static_assert(nu::gcd(-12, 18) == 6);
static_assert(nu::lcm(4, 6) == 12);

int main() {
    std::cout << "gcd(84, 36) = " << nu::gcd(84, 36) << '\n';
    std::cout << "lcm(21, 6)  = " << nu::lcm(21, 6) << '\n';

    std::cout << "Perfect numbers below 10000:";
    for (long long n = 1; n < 10000; ++n)
        if (nu::is_perfect(n)) std::cout << ' ' << n;
    std::cout << '\n';
}
