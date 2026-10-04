// Fixed version of the exercise program. Each fix explains what the sanitizer reported.
#include <iostream>
#include <optional>
#include <string>
#include <vector>

std::vector<int> make_squares(int n) {
    std::vector<int> v;
    v.reserve(static_cast<std::size_t>(n) + 1);
    // BUG 1 (ASan: container-overflow / heap-buffer-overflow): reserve() changes capacity,
    // not size, so v[i] wrote past the end. Also the loop was <= n. Use push_back.
    for (int i = 0; i <= n; ++i) v.push_back(i * i);
    return v;
}

// BUG 2 (ASan: stack-use-after-return): returned a reference to a local. Return by value.
std::string greeting() { return "hi"; }

// BUG 3 (UBSan: division by zero for empty input; also int / size_t silently converts
// the sum to unsigned, so negative sums produced garbage). Handle empty explicitly.
std::optional<double> average(const std::vector<int>& v) {
    if (v.empty()) return std::nullopt;
    long long sum = 0;
    for (int x : v) sum += x;
    return static_cast<double>(sum) / static_cast<double>(v.size());
}

int main() {
    auto sq = make_squares(5);
    // BUG 4 (ASan: heap-use-after-free): push_back may reallocate, invalidating `first`.
    // Fix: read the value (or an index) instead of keeping a pointer across the mutation.
    int first = sq[0];
    sq.push_back(99);
    std::cout << "first = " << first << '\n';

    std::cout << greeting() << '\n';
    std::cout << "average({}) = " << (average({}) ? std::to_string(*average({})) : "n/a") << '\n';
    std::cout << "average({-3, 4}) = " << *average({-3, 4}) << '\n';
}
