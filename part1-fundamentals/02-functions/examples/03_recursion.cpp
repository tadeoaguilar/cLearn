// Recursion: base cases, exponential blow-up, memoization, and Towers of Hanoi.
#include <cstdint>
#include <iostream>
#include <unordered_map>

std::uint64_t factorial(unsigned n) {
    if (n <= 1) return 1;          // base case
    return n * factorial(n - 1);   // recursive step gets closer to the base
}

// Naive fibonacci recomputes the same values over and over: O(2^n)
std::uint64_t fib_naive(unsigned n) { return n < 2 ? n : fib_naive(n - 1) + fib_naive(n - 2); }

// Memoized: cache results → O(n)
std::uint64_t fib_memo(unsigned n, std::unordered_map<unsigned, std::uint64_t>& cache) {
    if (n < 2) return n;
    if (auto it = cache.find(n); it != cache.end()) return it->second;
    auto value = fib_memo(n - 1, cache) + fib_memo(n - 2, cache);
    cache[n] = value;
    return value;
}

// Iterative version: O(n) time, O(1) memory, no stack growth
std::uint64_t fib_iter(unsigned n) {
    std::uint64_t a = 0, b = 1;
    for (unsigned i = 0; i < n; ++i) {
        auto next = a + b;
        a = b;
        b = next;
    }
    return a;
}

void hanoi(int disks, char from, char to, char via, int& moves) {
    if (disks == 0) return;
    hanoi(disks - 1, from, via, to, moves);
    ++moves;
    if (disks <= 3 && moves <= 7) std::cout << "  move disk " << disks << " " << from << " -> " << to << '\n';
    hanoi(disks - 1, via, to, from, moves);
}

int main() {
    for (unsigned n : {0u, 1u, 5u, 10u, 20u}) std::cout << n << "! = " << factorial(n) << '\n';

    std::cout << "fib_naive(30) = " << fib_naive(30) << " (slow-ish: ~1.6 million calls)\n";
    std::unordered_map<unsigned, std::uint64_t> cache;
    std::cout << "fib_memo(90)  = " << fib_memo(90, cache) << '\n';
    std::cout << "fib_iter(90)  = " << fib_iter(90) << '\n';

    int moves = 0;
    std::cout << "Hanoi with 3 disks:\n";
    hanoi(3, 'A', 'C', 'B', moves);
    std::cout << "total moves: " << moves << " (2^n - 1)\n";
}
