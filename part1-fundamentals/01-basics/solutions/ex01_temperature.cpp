#include <format>
#include <iostream>

int main() {
    std::cout << std::format("{:>8} | {:>10}\n", "Celsius", "Fahrenheit");
    std::cout << "---------+-----------\n";
    for (int c = -20; c <= 40; c += 10) {
        // 9.0 / 5.0 forces floating-point division (9 / 5 would be 1).
        double f = c * 9.0 / 5.0 + 32.0;
        std::cout << std::format("{:>8} | {:>10.1f}\n", c, f);
    }
}
