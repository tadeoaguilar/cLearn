# 10 — Exercises: Modern C++

### Ex 1 — Event queue with `std::variant` ⭐⭐
Model the input events `KeyPress{char key}`, `MouseClick{int x, y; int button}`,
`Resize{int w, h}` and `Quit{}` as a `std::variant`. Process a
`std::vector<Event>` with `std::visit` and `overloaded{...}`: print each event,
count each kind, and stop at `Quit`.
→ `solutions/ex01_event_variant.cpp`

### Ex 2 — Expression tree ⭐⭐⭐
Represent arithmetic expressions as
`struct Expr; using ExprPtr = std::unique_ptr<Expr>; struct Expr { std::variant<double, BinOp> node; };`
where `BinOp { char op; ExprPtr lhs, rhs; }`. Write `evaluate(const Expr&)` and
`to_string(const Expr&)` with `std::visit`. Build `(2 + 3) * (10 - 4) / 3`.
→ `solutions/ex02_expression_tree.cpp`

### Ex 3 — Sales report with ranges ⭐⭐
Given `struct Order { std::string customer; double amount; bool paid; int day; };`
use **views and range algorithms only** (no raw loops for the filtering):
1. total revenue from paid orders
2. paid orders in the last 7 days (`day >= today - 7`), sorted by amount desc
3. revenue per customer (accumulate into a `std::map`), then the top 3 customers
→ `solutions/ex03_ranges_report.cpp`

### Ex 4 — Compile-time prime table ⭐⭐
`template <std::size_t N> consteval std::array<int, N> first_primes()`
returns the first N primes. Use `static_assert` to verify the 10th prime is 29.
Print the table at runtime.
→ `solutions/ex04_constexpr_primes.cpp`

### Ex 5 — Generic `memoize` ⭐⭐⭐
Write `memoize(f)`, which takes a function `R(Arg)` and returns a lambda that
caches results in a `std::map<Arg, R>` that it owns (init-capture + `mutable`,
or a `shared_ptr` so copies share the cache). Count how many times the real
function runs.
→ `solutions/ex05_memoize.cpp`
