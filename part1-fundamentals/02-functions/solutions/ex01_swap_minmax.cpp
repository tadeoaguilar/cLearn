#include <iostream>
#include <stdexcept>
#include <vector>

void swap_values(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

struct MinMax {
    int min;
    int max;
};

// Choice: an empty vector has no min/max, so we throw. (Ch. 8 shows
// std::optional as an alternative that doesn't use exceptions.)
MinMax min_max(const std::vector<int>& v) {
    if (v.empty()) throw std::invalid_argument("min_max of empty vector");
    MinMax r{v[0], v[0]};
    for (int x : v) {
        if (x < r.min) r.min = x;
        if (x > r.max) r.max = x;
    }
    return r;
}

int main() {
    int a = 1, b = 2;
    swap_values(a, b);
    std::cout << "a=" << a << " b=" << b << '\n';

    auto [lo, hi] = min_max({5, -3, 9, 0, 9, 2});
    std::cout << "min=" << lo << " max=" << hi << '\n';

    try {
        min_max({});
    } catch (const std::invalid_argument& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
