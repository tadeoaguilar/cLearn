#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    constexpr int N = 100;
    std::vector<bool> is_prime(N + 1, true);
    is_prime[0] = is_prime[1] = false;

    // Cross out multiples of each prime p, starting at p*p
    // (smaller multiples were already crossed out by smaller primes).
    for (int p = 2; p * p <= N; ++p) {
        if (!is_prime[p]) continue;
        for (int m = p * p; m <= N; m += p) is_prime[m] = false;
    }

    int printed = 0;
    for (int i = 2; i <= N; ++i) {
        if (!is_prime[i]) continue;
        std::cout << std::setw(4) << i;
        if (++printed % 10 == 0) std::cout << '\n';
    }
    std::cout << "\nTotal: " << printed << " primes\n";
}
