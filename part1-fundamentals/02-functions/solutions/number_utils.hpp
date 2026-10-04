#pragma once

namespace nu {

constexpr long long gcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) { // Euclid's algorithm
        long long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

constexpr long long lcm(long long a, long long b) {
    if (a == 0 || b == 0) return 0;
    return a / gcd(a, b) * b; // divide first to reduce overflow risk
}

inline bool is_perfect(long long n) {
    if (n < 2) return false;
    long long sum = 1; // 1 divides everything
    for (long long d = 2; d * d <= n; ++d) {
        if (n % d == 0) {
            sum += d;
            if (d != n / d) sum += n / d; // the paired divisor
        }
    }
    return sum == n;
}

} // namespace nu
