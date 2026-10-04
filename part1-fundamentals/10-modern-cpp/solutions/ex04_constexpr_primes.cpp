#include <array>
#include <iostream>

template <std::size_t N>
consteval std::array<int, N> first_primes() {
    std::array<int, N> out{};
    std::size_t found = 0;
    for (int candidate = 2; found < N; ++candidate) {
        bool prime = true;
        for (std::size_t i = 0; i < found && out[i] * out[i] <= candidate; ++i) {
            if (candidate % out[i] == 0) {
                prime = false;
                break;
            }
        }
        if (prime) out[found++] = candidate;
    }
    return out;
}

constexpr auto kPrimes = first_primes<25>();
static_assert(kPrimes[9] == 29, "the 10th prime is 29");
static_assert(kPrimes[24] == 97);

int main() {
    for (std::size_t i = 0; i < kPrimes.size(); ++i) std::cout << kPrimes[i] << ((i + 1) % 10 == 0 ? '\n' : ' ');
    std::cout << "\n(all computed during compilation)\n";
}
