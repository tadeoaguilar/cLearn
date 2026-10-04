#include <iostream>
#include <span>
#include <vector>

const int* find_max(std::span<const int> values) {
    if (values.empty()) return nullptr;
    const int* best = &values[0];
    for (const int& v : values)
        if (v > *best) best = &v;
    return best;
}

int main() {
    std::vector<int> v{3, 17, -4, 17, 9};
    if (const int* m = find_max(v)) {
        auto index = m - v.data(); // pointer difference = number of elements between them
        std::cout << "max = " << *m << " at index " << index << '\n';
    }

    std::vector<int> empty;
    if (find_max(empty) == nullptr) std::cout << "empty: no max\n";
}
