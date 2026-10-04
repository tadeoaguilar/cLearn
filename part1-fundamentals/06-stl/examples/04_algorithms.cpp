#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

void print(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';
}

int main() {
    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1); // 1..10
    print("iota", v);

    std::mt19937 rng{42}; // fixed seed → reproducible
    std::shuffle(v.begin(), v.end(), rng);
    print("shuffled", v);

    std::sort(v.begin(), v.end());
    print("sorted", v);
    std::sort(v.begin(), v.end(), std::greater<>{});
    print("desc", v);

    auto is_even = [](int x) { return x % 2 == 0; };
    std::cout << "evens: " << std::count_if(v.begin(), v.end(), is_even) << '\n';
    std::cout << "any > 9? " << std::boolalpha << std::any_of(v.begin(), v.end(), [](int x) { return x > 9; }) << '\n';

    if (auto it = std::find(v.begin(), v.end(), 7); it != v.end())
        std::cout << "found 7 at index " << std::distance(v.begin(), it) << '\n';

    auto [mn, mx] = std::minmax_element(v.begin(), v.end());
    std::cout << "min " << *mn << " max " << *mx << '\n';

    std::cout << "sum: " << std::accumulate(v.begin(), v.end(), 0) << '\n';
    std::cout << "product: " << std::accumulate(v.begin(), v.end(), 1LL, std::multiplies<>{}) << '\n';

    std::vector<int> squares;
    std::transform(v.begin(), v.end(), std::back_inserter(squares), [](int x) { return x * x; });
    print("squares", squares);

    std::vector<int> evens;
    std::copy_if(v.begin(), v.end(), std::back_inserter(evens), is_even);
    print("evens", evens);

    // Binary search requires a SORTED range
    std::sort(v.begin(), v.end());
    std::cout << "binary_search 4: " << std::binary_search(v.begin(), v.end(), 4) << '\n';
    auto lb = std::lower_bound(v.begin(), v.end(), 6); // first element >= 6
    std::cout << "lower_bound(6) -> " << *lb << '\n';

    // unique only removes ADJACENT duplicates → sort first
    std::vector<int> dups{3, 1, 3, 2, 1, 3};
    std::sort(dups.begin(), dups.end());
    dups.erase(std::unique(dups.begin(), dups.end()), dups.end());
    print("unique", dups);

    // Classic erase-remove (std::erase_if does this in C++20)
    std::vector<int> w{1, 2, 3, 4, 5, 6};
    w.erase(std::remove_if(w.begin(), w.end(), is_even), w.end());
    print("odds only", w);

    // partial_sort: just the top 3
    std::vector<int> scores{55, 90, 72, 88, 61, 99, 70};
    std::partial_sort(scores.begin(), scores.begin() + 3, scores.end(), std::greater<>{});
    std::cout << "top 3: " << scores[0] << ' ' << scores[1] << ' ' << scores[2] << '\n';
}
