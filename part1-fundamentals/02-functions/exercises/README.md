# 02 — Exercises: Functions

### Ex 1 — Swap and min/max ⭐
1. Write `void swap_values(int& a, int& b)` (don't use `std::swap`).
2. Write `MinMax min_max(const std::vector<int>& v)` that returns a struct
   `{int min; int max;}` in a single pass. Decide what to do for an empty vector,
   and document your choice.
→ `solutions/ex01_swap_minmax.cpp`

### Ex 2 — Padding with defaults ⭐
Write `std::string pad(const std::string& s, std::size_t width, char fill = ' ', bool align_left = true)`.
If `s` is already wider than `width`, return it unchanged. Print a small
left/right aligned table with it.
→ `solutions/ex02_pad.cpp`

### Ex 3 — Recursion trio ⭐⭐
Write these recursively:
- `long long power(long long base, unsigned exp)` using **fast exponentiation**:
  `x^n = (x^(n/2))^2`, times `x` if n is odd. That takes O(log n) calls.
- `int digit_sum(int n)`: `digit_sum(1234) == 10`. Handle negatives.
- `std::string to_binary(unsigned n)`: `to_binary(10) == "1010"`, `to_binary(0) == "0"`.
→ `solutions/ex03_recursion.cpp`

### Ex 4 — Your own header ⭐⭐
Create `number_utils.hpp` in namespace `nu` with `gcd`, `lcm`, and
`is_perfect` (a number equal to the sum of its proper divisors: 6, 28, 496…).
Make `gcd` `constexpr` and prove it with `static_assert`. Use the header from
a `main` that prints all perfect numbers below 10 000.
→ `solutions/ex04_number_utils.cpp` + `solutions/number_utils.hpp`

### Ex 5 — Higher-order functions ⭐⭐⭐
Implement:
- `std::vector<int> map_vec(const std::vector<int>&, const std::function<int(int)>&)`
- `std::vector<int> filter_vec(const std::vector<int>&, const std::function<bool(int)>&)`
- `int reduce_vec(const std::vector<int>&, int init, const std::function<int(int,int)>&)`
- `std::function<int(int)> compose(std::function<int(int)> f, std::function<int(int)> g)`, which returns `x ↦ f(g(x))`.

Compute "the sum of the squares of the odd numbers in 1..10" using only these.
→ `solutions/ex05_higher_order.cpp`
