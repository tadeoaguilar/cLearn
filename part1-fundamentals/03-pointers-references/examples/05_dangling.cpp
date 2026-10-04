// Lifetime bugs. The dangerous lines are commented out: uncomment one,
// build with -fsanitize=address -g, and watch AddressSanitizer report it.
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// int* bad_pointer() { int local = 5; return &local; }        // returns address of dead local
// std::string_view bad_view() { std::string s = "temp"; return s; } // view of destroyed string

int main() {
    std::vector<int> v{1, 2, 3};
    v.reserve(3); // capacity may be exactly 3

    int& first = v[0];
    std::cout << "first = " << first << '\n';

    v.push_back(4); // may reallocate: all references/pointers/iterators into v are invalidated
    // std::cout << first << '\n'; // UNDEFINED BEHAVIOR (heap-use-after-free under ASan)

    // Safe alternatives: keep an index, or re-acquire the reference after modifying
    std::size_t first_index = 0;
    std::cout << "via index: " << v[first_index] << '\n';

    // Same issue with iterators:
    // for (auto it = v.begin(); it != v.end(); ++it) if (*it == 2) v.push_back(5); // UB

    std::cout << "No UB executed. Read the comments, then try uncommenting with ASan.\n";
}
