#include <iostream>
#include <span>
#include <vector>

void reverse(int* begin, int* end) {
    if (begin == end) return;
    --end; // end is one-past-the-last; step back onto the last element
    while (begin < end) {
        int tmp = *begin;
        *begin = *end;
        *end = tmp;
        ++begin;
        --end;
    }
}

void reverse(std::span<int> values) { reverse(values.data(), values.data() + values.size()); }

void print(std::span<const int> values) {
    for (int v : values) std::cout << v << ' ';
    std::cout << '\n';
}

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    reverse(arr, arr + 5);
    print(arr);

    std::vector<int> v{10, 20, 30, 40};
    reverse(v);
    print(v);

    std::vector<int> empty;
    reverse(empty); // must not crash
    print(empty);
}
