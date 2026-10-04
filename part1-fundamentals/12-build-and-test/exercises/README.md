# 12 — Exercises: Build & Test

### Ex 1 — CMake a multi-file program ⭐⭐
Create a small `inventory` library (`Inventory` class with `add`, `remove`,
`quantity`, `total_items`) split into `include/inventory/inventory.hpp` and
`src/inventory.cpp`, plus an app in `src/main.cpp`. Write the `CMakeLists.txt`
with a library target and an executable target, using `PUBLIC` include
directories correctly. Build it both standalone and from the repo root.
→ `solutions/ex01_cmake_project/`

### Ex 2 — Test the Fraction class ⭐⭐
Move the `Fraction` from chapter 4 into a header, and write doctest unit tests
covering normalization, arithmetic, comparison, printing (`std::ostringstream`)
and the error cases (`CHECK_THROWS_AS`). Register it with `add_test` and run
`ctest`.
→ `solutions/ex02_fraction_tests/`

### Ex 3 — Sanitizer bug hunt ⭐⭐
The program below has **4 bugs**. Build it with
`-fsanitize=address,undefined -g`, run it, read the reports, and fix them.
```cpp
std::vector<int> make_squares(int n) {
    std::vector<int> v;
    v.reserve(n);
    for (int i = 0; i <= n; ++i) v[i] = i * i;        // bug 1
    return v;
}
const std::string& greeting() { std::string s = "hi"; return s; }   // bug 2
int average(const std::vector<int>& v) {
    int sum = 0; for (int x : v) sum += x;
    return sum / v.size();                             // bug 3 (empty vector? signedness?)
}
int main() {
    auto sq = make_squares(5);
    int* first = &sq[0]; sq.push_back(99); std::cout << *first;  // bug 4
    std::cout << greeting() << average({});
}
```
→ `solutions/ex03_sanitizer_hunt.cpp` (the fixed version, with explanations)

### Ex 4 — Test-driven `slugify` ⭐⭐
Using the mini framework from `examples/02_mini_test_framework.cpp`, **write the
tests first** for `std::string slugify(std::string_view title)`:
`"Hello, World!" → "hello-world"`, `"  C++ is   FUN  " → "c-is-fun"`, `"" → ""`,
`"Ünïcödé?" → "ncd"` (non-ASCII bytes are dropped, ASCII letters kept), `"a--b" → "a-b"`. Then implement it
until all the tests pass.
→ `solutions/ex04_tdd_slugify.cpp`
