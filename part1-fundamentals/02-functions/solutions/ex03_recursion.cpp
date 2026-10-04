#include <iostream>
#include <string>

long long power(long long base, unsigned exp) {
    if (exp == 0) return 1;
    long long half = power(base, exp / 2); // compute once, reuse
    long long result = half * half;
    return (exp % 2 == 1) ? result * base : result;
}

int digit_sum(int n) {
    if (n < 0) return digit_sum(-n);
    if (n < 10) return n;
    return n % 10 + digit_sum(n / 10);
}

std::string to_binary(unsigned n) {
    if (n < 2) return std::string(1, static_cast<char>('0' + n));
    return to_binary(n / 2) + static_cast<char>('0' + n % 2);
}

int main() {
    std::cout << "2^10 = " << power(2, 10) << '\n';
    std::cout << "3^13 = " << power(3, 13) << '\n';
    std::cout << "7^0  = " << power(7, 0) << '\n';

    std::cout << "digit_sum(1234)  = " << digit_sum(1234) << '\n';
    std::cout << "digit_sum(-9875) = " << digit_sum(-9875) << '\n';

    for (unsigned n : {0u, 1u, 2u, 10u, 255u, 1024u}) std::cout << n << " -> " << to_binary(n) << '\n';
}
