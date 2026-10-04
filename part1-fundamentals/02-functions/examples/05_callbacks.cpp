// Passing behavior into functions: function pointers, std::function, lambdas.
#include <functional>
#include <iostream>
#include <string>
#include <vector>

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }

// 1) Classic function pointer: only accepts plain functions (or captureless lambdas)
int apply_ptr(int (*op)(int, int), int a, int b) { return op(a, b); }

// 2) std::function: accepts ANY callable with a matching signature (small runtime cost)
int apply_fn(const std::function<int(int, int)>& op, int a, int b) { return op(a, b); }

// A practical use: a reusable "for each matching item" routine
void for_each_matching(const std::vector<int>& v,
                       const std::function<bool(int)>& predicate,
                       const std::function<void(int)>& action) {
    for (int x : v)
        if (predicate(x)) action(x);
}

int main() {
    std::cout << apply_ptr(add, 2, 3) << ' ' << apply_ptr(mul, 2, 3) << '\n';

    int offset = 100;
    auto add_offset = [offset](int a, int b) { return a + b + offset; }; // captures offset
    std::cout << apply_fn(add_offset, 2, 3) << '\n';
    // apply_ptr(add_offset, 2, 3); // ERROR: capturing lambda is not a function pointer

    std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum = 0;
    for_each_matching(
        nums, [](int x) { return x % 2 == 0; },
        [&sum](int x) { sum += x; }); // capture by reference to modify sum
    std::cout << "sum of evens: " << sum << '\n';

    // A table of named operations
    std::vector<std::pair<std::string, std::function<int(int, int)>>> ops{
        {"add", add}, {"mul", mul}, {"max", [](int a, int b) { return a > b ? a : b; }}};
    for (const auto& [name, op] : ops) std::cout << name << "(6, 7) = " << op(6, 7) << '\n';
}
