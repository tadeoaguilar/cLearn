#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename F>
long long time_us(F f) {
    auto start = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
}

int main() {
    std::vector<std::string> big(200'000, std::string(64, 'x'));

    std::vector<std::string> copy_target;
    auto copy_us = time_us([&] { copy_target = big; }); // deep copy: allocate + copy 200k strings

    std::vector<std::string> move_target;
    auto move_us = time_us([&] { move_target = std::move(big); }); // swap 3 pointers

    std::cout << "copy: " << copy_us << " us\n";
    std::cout << "move: " << move_us << " us\n";
    std::cout << "big.size() after move: " << big.size() << " (moved-from vector is empty in practice)\n";
}
