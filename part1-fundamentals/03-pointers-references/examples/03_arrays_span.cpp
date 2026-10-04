#include <array>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

// BAD: the size is lost; `arr` is really an int*
int sum_c_style(const int arr[], std::size_t n) {
    int s = 0;
    for (std::size_t i = 0; i < n; ++i) s += arr[i];
    return s;
}

// GOOD: span carries pointer + size, works with any contiguous container
int sum_span(std::span<const int> values) {
    return std::accumulate(values.begin(), values.end(), 0);
}

void double_all(std::span<int> values) { // non-const span → can modify
    for (int& v : values) v *= 2;
}

int main() {
    int c_arr[5] = {1, 2, 3, 4, 5};
    std::array<int, 5> std_arr{1, 2, 3, 4, 5};
    std::vector<int> vec{1, 2, 3, 4, 5, 6};

    std::cout << "std::size(c_arr) = " << std::size(c_arr) << '\n';
    std::cout << "sum_c_style: " << sum_c_style(c_arr, std::size(c_arr)) << '\n';
    std::cout << "sum_span(c_arr):   " << sum_span(c_arr) << '\n';
    std::cout << "sum_span(std_arr): " << sum_span(std_arr) << '\n';
    std::cout << "sum_span(vec):     " << sum_span(vec) << '\n';
    std::cout << "sum of first 3:    " << sum_span(std::span(vec).first(3)) << '\n';

    double_all(vec);
    std::cout << "vec doubled: ";
    for (int v : vec) std::cout << v << ' ';
    std::cout << '\n';

    // Pointer arithmetic (what [] does under the hood)
    int* p = c_arr;
    std::cout << "*(p + 2) = " << *(p + 2) << " == c_arr[2] = " << c_arr[2] << '\n';
    std::cout << "address step: " << (reinterpret_cast<char*>(p + 1) - reinterpret_cast<char*>(p))
              << " bytes (sizeof(int))\n";

    // std::array behaves like a value: copying copies the elements
    auto copy = std_arr;
    copy[0] = 999;
    std::cout << "std_arr[0] = " << std_arr[0] << ", copy[0] = " << copy[0] << '\n';

    // .at() is bounds-checked
    try {
        std::cout << std_arr.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "at(10) threw out_of_range\n";
    }
}
